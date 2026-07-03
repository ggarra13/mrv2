"""Abstract interface that every file transfer backend must implement.

The goal: calling code is written once against `FileTransferBackend`
and can swap SFTP, S3, or any future backend (Azure Blob, GCS, FTP...)
via configuration alone, with no branching on backend type.
"""

from __future__ import annotations

import threading
from abc import ABC, abstractmethod
from pathlib import Path
from typing import BinaryIO, Iterator, Optional, Union

from .models import FileMetadata, ProgressCallback, TransferResult

PathLike = Union[str, Path]
FileSource = Union[PathLike, BinaryIO]


class FileTransferBackend(ABC):
    """Common interface for all file transfer backends.

    Implementations should:
      - Translate SDK-specific exceptions into the exceptions defined
        in `exceptions.py`, so callers never see paramiko/botocore
        error types.
      - Be usable as a context manager (`with backend: ...`), opening
        the connection on __enter__ and closing it on __exit__.
      - Honor the `retry_policy` passed in their config for transient
        network failures.
      - Accept either a local file path or an open binary file object
        for upload/download, so callers can stream without touching
        disk if they want to.
    """

    # --- lifecycle ---------------------------------------------------

    @abstractmethod
    def connect(self) -> None:
        """Open the underlying connection. Safe to call when already
        connected (no-op)."""

    @abstractmethod
    def close(self) -> None:
        """Close the underlying connection and release resources."""

    def __enter__(self) -> "FileTransferBackend":
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.close()

    # --- core transfer -------------------------------------------------

    @abstractmethod
    def upload(
        self,
        source: FileSource,
        remote_path: str,
        *,
        overwrite: bool = True,
        resume: bool = False,
        cancel_event: Optional[threading.Event] = None,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        """Upload a local file (path or open binary file object) to
        `remote_path`.

        If `resume=True` the backend detects any previous partial
        transfer and continues from where it left off. Uploads stage
        through a `<remote_path>.part` file so the destination only
        appears atomically on completion — meaning a cancelled upload
        leaves the .part file in place and can be resumed later.

        If `cancel_event` is set (by AsyncTransferManager), the backend
        checks it each chunk and raises `TransferCancelledError` promptly.
        For the non-chunked fast path, the check happens before the
        transfer starts; use `resume=True` for granular mid-transfer
        cancellation on large files.

        Raises `RemoteFileExistsError` if `overwrite=False` and the
        destination already exists.
        Raises `UnsupportedOperationError` if the backend does not
        support resumption.
        Raises `TransferCancelledError` if cancelled via cancel_event.
        """

    @abstractmethod
    def download(
        self,
        remote_path: str,
        destination: FileSource,
        *,
        resume: bool = False,
        cancel_event: Optional[threading.Event] = None,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        """Download `remote_path` to a local path or writable binary
        file object.

        If `resume=True` and the destination already contains a partial
        file, the backend skips the bytes already received and appends
        from that offset.

        If `cancel_event` is set, the backend checks it each chunk and
        raises `TransferCancelledError` promptly.

        Raises `RemoteFileNotFoundError` if `remote_path` does not exist.
        Raises `UnsupportedOperationError` if the backend does not
        support resumption.
        Raises `TransferCancelledError` if cancelled via cancel_event.
        """

    # --- metadata / browsing -------------------------------------------

    @abstractmethod
    def exists(self, remote_path: str) -> bool:
        """Return True if remote_path exists (file or directory)."""

    @abstractmethod
    def get_metadata(self, remote_path: str) -> FileMetadata:
        """Return metadata for a single remote path.

        Raises RemoteFileNotFoundError if it does not exist.
        """

    @abstractmethod
    def list_dir(self, remote_path: str) -> Iterator[FileMetadata]:
        """Yield metadata for each entry directly under remote_path
        (non-recursive)."""

    # --- mutation --------------------------------------------------

    @abstractmethod
    def delete(self, remote_path: str) -> None:
        """Delete a remote file. Raises RemoteFileNotFoundError if it
        does not exist."""

    @abstractmethod
    def mkdir(self, remote_path: str, *, parents: bool = False) -> None:
        """Create a remote directory. For object stores without real
        directories, this is a cosmetic no-op-equivalent (keys imply
        their own prefix structure)."""

    @abstractmethod
    def move(self, src_remote_path: str, dst_remote_path: str) -> None:
        """Move/rename a remote file."""
