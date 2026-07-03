"""SFTP backend implementation using paramiko."""

from __future__ import annotations

import os
import stat
import threading
import time
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Iterator, Optional

try:
    import paramiko
except ImportError as e:  # pragma: no cover
    raise ImportError(
        "The SFTP backend requires paramiko. Install it with "
        "`pip install paramiko`."
    ) from e

from ..base import FileTransferBackend, FileSource
from ..exceptions import (
    AuthenticationError,
    BackendConnectionError,
    RemoteFileExistsError,
    RemoteFileNotFoundError,
    RemotePermissionError,
    TransferCancelledError,
    TransferInterruptedError,
    UnsupportedOperationError,
)
from ..models import FileMetadata, ProgressCallback, RetryPolicy, TransferResult


@dataclass
class SFTPConfig:
    host: str
    port: int = 22
    username: str = ""
    password: Optional[str] = None
    private_key_path: Optional[str] = None
    private_key_passphrase: Optional[str] = None
    timeout_seconds: float = 30.0
    retry_policy: RetryPolicy = field(default_factory=RetryPolicy)
    chunk_size: int = 32 * 1024
    """Bytes per read/write call used by the resumable transfer path.
    The default (32 KiB) matches paramiko's internal buffer size.
    Increase for high-latency, high-bandwidth links."""


class SFTPBackend(FileTransferBackend):
    def __init__(self, config: SFTPConfig):
        self._config = config
        self._transport: Optional["paramiko.Transport"] = None
        self._sftp: Optional["paramiko.SFTPClient"] = None

    # --- lifecycle ---------------------------------------------------

    def connect(self) -> None:
        if self._sftp is not None:
            return
        cfg = self._config
        try:
            self._transport = paramiko.Transport((cfg.host, cfg.port))
            if cfg.private_key_path:
                pkey = paramiko.RSAKey.from_private_key_file(
                    cfg.private_key_path, password=cfg.private_key_passphrase
                )
                self._transport.connect(username=cfg.username, pkey=pkey)
            else:
                self._transport.connect(
                    username=cfg.username, password=cfg.password
                )
            self._sftp = paramiko.SFTPClient.from_transport(self._transport)
            self._sftp.get_channel().settimeout(cfg.timeout_seconds)
        except paramiko.AuthenticationException as e:
            self.close()
            raise AuthenticationError(str(e)) from e
        except (paramiko.SSHException, OSError) as e:
            self.close()
            raise BackendConnectionError(str(e)) from e

    def close(self) -> None:
        if self._sftp is not None:
            self._sftp.close()
            self._sftp = None
        if self._transport is not None:
            self._transport.close()
            self._transport = None

    def _client(self) -> "paramiko.SFTPClient":
        if self._sftp is None:
            raise BackendConnectionError(
                "Not connected. Call connect() or use as a context manager."
            )
        return self._sftp

    def _with_retry(
        self,
        func,
        *args,
        reconnect: bool = False,
        cancel_event: Optional[threading.Event] = None,
        **kwargs,
    ):
        policy = self._config.retry_policy
        delay = policy.backoff_seconds
        last_err: Optional[Exception] = None
        for attempt in range(1, policy.max_attempts + 1):
            if cancel_event and cancel_event.is_set():
                raise TransferCancelledError()
            try:
                return func(*args, **kwargs)
            except TransferCancelledError:
                raise  # never swallow a cancellation
            except (paramiko.SSHException, OSError) as e:
                last_err = e
                if attempt == policy.max_attempts:
                    break
                if reconnect:
                    try:
                        self.close()
                        self.connect()
                    except Exception:
                        pass  # connect() failure will surface on next func() call
                time.sleep(delay)
                delay *= policy.backoff_multiplier
        raise TransferInterruptedError(
            f"Operation failed after {policy.max_attempts} attempts: {last_err}"
        ) from last_err

    # --- helpers for resumable I/O -----------------------------------

    def _source_size(self, source: FileSource) -> Optional[int]:
        """Return the total byte length of source, or None if unknown."""
        if isinstance(source, (str, Path)):
            return os.path.getsize(source)
        try:
            pos = source.tell()
            source.seek(0, 2)
            size = source.tell()
            source.seek(pos)
            return size
        except (AttributeError, OSError):
            return None

    def _require_seekable(self, source, label: str) -> None:
        """Raise UnsupportedOperationError if source is not seekable."""
        if isinstance(source, (str, Path)):
            return  # paths are always seekable via open()
        seekable = getattr(source, "seekable", None)
        if seekable is None or not seekable():
            raise UnsupportedOperationError(
                f"resume=True requires a seekable {label}; "
                f"the supplied file object does not support seek()."
            )

    # --- core transfer -------------------------------------------------

    def upload(
        self,
        source: FileSource,
        remote_path: str,
        *,
        overwrite: bool = True,
        resume: bool = True,
        cancel_event: Optional[threading.Event] = None,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        if cancel_event and cancel_event.is_set():
            raise TransferCancelledError()
        if resume:
            return self._upload_resumable(
                source, remote_path, overwrite=overwrite,
                cancel_event=cancel_event,
                progress_callback=progress_callback,
            )
        # --- non-resumable fast path ---------------------------------
        # Cancellation is checked before the transfer starts; for
        # granular mid-transfer cancellation on large files use resume=True.
        client = self._client()
        if not overwrite and self.exists(remote_path):
            raise RemoteFileExistsError(remote_path)

        start = time.monotonic()
        is_path = isinstance(source, (str, Path))
        cb = (lambda sent, total: progress_callback(sent, total)) if progress_callback else None

        def _do_upload():
            if is_path:
                client.put(str(source), remote_path, callback=cb)
                return os.path.getsize(source)
            client.putfo(source, remote_path, callback=cb)
            return source.tell()

        try:
            size = self._with_retry(_do_upload, cancel_event=cancel_event)
        except IOError as e:
            raise RemotePermissionError(str(e)) from e

        return TransferResult(
            local_path=str(source) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=size,
            duration_seconds=time.monotonic() - start,
        )

    def _upload_resumable(
        self,
        source: FileSource,
        remote_path: str,
        *,
        overwrite: bool,
        cancel_event: Optional[threading.Event],
        progress_callback: Optional[ProgressCallback],
    ) -> TransferResult:
        """Upload via a .part staging file.

        The target path only appears when the transfer finishes, so a
        partial file is never visible at the intended destination.
        Re-entrant: calling again (or after resume=True) picks up from
        the end of the existing .part file.
        Cancellation is checked every chunk so the UI stays responsive.
        """
        self._require_seekable(source, "upload source")
        is_path = isinstance(source, (str, Path))
        local_size = self._source_size(source)  # None → unknown (stream)
        part_path = remote_path + ".part"
        start = time.monotonic()

        def _attempt():
            client = self._client()

            # --- determine how far we got last time -------------------
            offset = 0
            try:
                attr = client.stat(part_path)
                offset = attr.st_size or 0
            except FileNotFoundError:
                pass

            # Sanity: if .part is larger than the source, the source has
            # changed or is corrupted state — start over.
            if local_size is not None and offset > local_size:
                client.remove(part_path)
                offset = 0

            # --- open source at offset --------------------------------
            if is_path:
                fh = open(source, "rb")
                fh.seek(offset)
            else:
                source.seek(offset)
                fh = source

            chunk = self._config.chunk_size
            bytes_done = offset

            try:
                # 'ab' on SFTP == append; set_pipelined() batches
                # multiple small SSH_FXP_WRITE packets for throughput.
                with client.open(part_path, "ab") as remote_fh:
                    remote_fh.set_pipelined(True)
                    while True:
                        if cancel_event and cancel_event.is_set():
                            raise TransferCancelledError(
                                f"Upload cancelled at offset {bytes_done}",
                                offset=bytes_done,
                            )
                        data = fh.read(chunk)
                        if not data:
                            break
                        remote_fh.write(data)
                        bytes_done += len(data)
                        if progress_callback:
                            progress_callback(bytes_done, local_size)
            finally:
                if is_path:
                    fh.close()

            return offset, bytes_done

        try:
            offset, bytes_done = self._with_retry(
                _attempt, reconnect=True, cancel_event=cancel_event
            )
        except IOError as e:
            raise RemotePermissionError(str(e)) from e

        # --- atomic rename to final path ------------------------------
        client = self._client()
        if not overwrite and self.exists(remote_path):
            client.remove(part_path)  # clean up
            raise RemoteFileExistsError(remote_path)
        try:
            client.rename(part_path, remote_path)
        except OSError:
            # Some servers reject rename when target exists
            client.remove(remote_path)
            client.rename(part_path, remote_path)

        return TransferResult(
            local_path=str(source) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=bytes_done - offset,
            duration_seconds=time.monotonic() - start,
            resumed_from_offset=offset,
        )

    def download(
        self,
        remote_path: str,
        destination: FileSource,
        *,
        resume: bool = True,
        cancel_event: Optional[threading.Event] = None,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        if cancel_event and cancel_event.is_set():
            raise TransferCancelledError()
        client = self._client()
        if not self.exists(remote_path):
            raise RemoteFileNotFoundError(remote_path)

        if resume:
            return self._download_resumable(
                remote_path, destination,
                cancel_event=cancel_event,
                progress_callback=progress_callback,
            )
        # --- non-resumable fast path ---------------------------------
        start = time.monotonic()
        is_path = isinstance(destination, (str, Path))
        cb = (lambda sent, total: progress_callback(sent, total)) if progress_callback else None

        def _do_download():
            if is_path:
                client.get(remote_path, str(destination), callback=cb)
            else:
                client.getfo(remote_path, destination, callback=cb)

        self._with_retry(_do_download, cancel_event=cancel_event)
        size = client.stat(remote_path).st_size

        return TransferResult(
            local_path=str(destination) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=size,
            duration_seconds=time.monotonic() - start,
        )

    def _download_resumable(
        self,
        remote_path: str,
        destination: FileSource,
        *,
        cancel_event: Optional[threading.Event],
        progress_callback: Optional[ProgressCallback],
    ) -> TransferResult:
        """Download by seeking the remote stream to the end of any
        already-received local data and appending the remainder.
        Cancellation is checked every chunk.
        """
        self._require_seekable(destination, "download destination")
        is_path = isinstance(destination, (str, Path))
        remote_size = self._client().stat(remote_path).st_size
        start = time.monotonic()

        def _local_offset() -> int:
            if is_path:
                p = Path(destination)
                return p.stat().st_size if p.exists() else 0
            try:
                destination.seek(0, 2)
                return destination.tell()
            except (AttributeError, OSError):
                return 0

        def _attempt():
            client = self._client()
            offset = _local_offset()

            # If local is somehow larger than remote, the local copy is
            # from a different (or truncated) remote file — start over.
            if offset > remote_size:
                if is_path:
                    open(destination, "wb").close()
                else:
                    destination.seek(0)
                    destination.truncate(0)
                offset = 0

            chunk = self._config.chunk_size
            bytes_done = offset

            with client.open(remote_path, "rb") as remote_fh:
                remote_fh.prefetch(remote_size)   # pipelined read-ahead
                remote_fh.seek(offset)

                if is_path:
                    mode = "ab" if offset > 0 else "wb"
                    local_fh = open(destination, mode)
                else:
                    if offset > 0:
                        destination.seek(offset)
                    local_fh = destination

                try:
                    while True:
                        if cancel_event and cancel_event.is_set():
                            raise TransferCancelledError(
                                f"Download cancelled at offset {bytes_done}",
                                offset=bytes_done,
                            )
                        data = remote_fh.read(chunk)
                        if not data:
                            break
                        local_fh.write(data)
                        bytes_done += len(data)
                        if progress_callback:
                            progress_callback(bytes_done, remote_size)
                finally:
                    if is_path:
                        local_fh.close()

            return offset, bytes_done

        offset, bytes_done = self._with_retry(
            _attempt, reconnect=True, cancel_event=cancel_event
        )

        return TransferResult(
            local_path=str(destination) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=bytes_done - offset,
            duration_seconds=time.monotonic() - start,
            resumed_from_offset=offset,
        )

    # --- metadata / browsing -----------------------------------------

    def exists(self, remote_path: str) -> bool:
        try:
            self._client().stat(remote_path)
            return True
        except FileNotFoundError:
            return False

    def get_metadata(self, remote_path: str) -> FileMetadata:
        try:
            attr = self._client().stat(remote_path)
        except FileNotFoundError as e:
            raise RemoteFileNotFoundError(remote_path) from e
        return FileMetadata(
            path=remote_path,
            size=attr.st_size or 0,
            last_modified=(
                datetime.fromtimestamp(attr.st_mtime) if attr.st_mtime else None
            ),
            is_dir=bool(attr.st_mode and stat.S_ISDIR(attr.st_mode)),
        )

    def list_dir(self, remote_path: str) -> Iterator[FileMetadata]:
        client = self._client()
        try:
            entries = client.listdir_attr(remote_path)
        except FileNotFoundError as e:
            raise RemoteFileNotFoundError(remote_path) from e
        for attr in entries:
            yield FileMetadata(
                path=f"{remote_path.rstrip('/')}/{attr.filename}",
                size=attr.st_size or 0,
                last_modified=(
                    datetime.fromtimestamp(attr.st_mtime) if attr.st_mtime else None
                ),
                is_dir=bool(attr.st_mode and stat.S_ISDIR(attr.st_mode)),
            )

    # --- mutation --------------------------------------------------

    def delete(self, remote_path: str) -> None:
        try:
            self._client().remove(remote_path)
        except FileNotFoundError as e:
            raise RemoteFileNotFoundError(remote_path) from e
        except IOError as e:
            raise RemotePermissionError(str(e)) from e

    def mkdir(self, remote_path: str, *, parents: bool = False) -> None:
        client = self._client()
        if not parents:
            client.mkdir(remote_path)
            return
        current = "."
        for part in remote_path.strip("/").split("/"):
            current = f"{current}/{part}" if current else f"./{part}"
            if not self.exists(current):
                client.mkdir(current)

    def move(self, src_remote_path: str, dst_remote_path: str) -> None:
        try:
            self._client().rename(src_remote_path, dst_remote_path)
        except FileNotFoundError as e:
            raise RemoteFileNotFoundError(src_remote_path) from e
