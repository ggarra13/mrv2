"""Amazon S3 backend implementation using boto3.

S3 has no real concept of directories; "directories" are simulated
via key prefixes ending in '/'. mkdir() therefore writes a cosmetic
zero-byte marker object (never required for correctness), and
list_dir() uses S3's delimiter-based listing to emulate a
non-recursive directory listing.
"""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterator, Optional

try:
    import boto3
    from botocore.exceptions import ClientError, BotoCoreError
except ImportError as e:  # pragma: no cover
    raise ImportError(
        "The S3 backend requires boto3. Install it with `pip install boto3`."
    ) from e

from ..base import FileTransferBackend, FileSource
from ..exceptions import (
    AuthenticationError,
    BackendConnectionError,
    RemoteFileExistsError,
    RemoteFileNotFoundError,
    TransferInterruptedError,
    UnsupportedOperationError,
)
from ..models import FileMetadata, ProgressCallback, RetryPolicy, TransferResult


@dataclass
class S3Config:
    bucket: str
    region: Optional[str] = None
    access_key_id: Optional[str] = None  # None = use default credential chain
    secret_access_key: Optional[str] = None
    session_token: Optional[str] = None
    endpoint_url: Optional[str] = None  # for S3-compatible stores (MinIO, R2...)
    retry_policy: RetryPolicy = field(default_factory=RetryPolicy)


class _ProgressAdapter:
    """Adapts boto3's callback(bytes_transferred) signature to this
    library's callback(bytes_transferred, total_bytes) signature."""

    def __init__(self, callback: ProgressCallback, total_bytes: Optional[int]):
        self._callback = callback
        self._total = total_bytes
        self._seen = 0

    def __call__(self, chunk_bytes: int) -> None:
        self._seen += chunk_bytes
        self._callback(self._seen, self._total)


class S3Backend(FileTransferBackend):
    def __init__(self, config: S3Config):
        self._config = config
        self._client = None

    # --- lifecycle ---------------------------------------------------

    def connect(self) -> None:
        if self._client is not None:
            return
        cfg = self._config
        try:
            client = boto3.client(
                "s3",
                region_name=cfg.region,
                aws_access_key_id=cfg.access_key_id,
                aws_secret_access_key=cfg.secret_access_key,
                aws_session_token=cfg.session_token,
                endpoint_url=cfg.endpoint_url,
            )
            client.head_bucket(Bucket=cfg.bucket)  # fail fast on bad creds/bucket
            self._client = client
        except ClientError as e:
            code = e.response.get("Error", {}).get("Code", "")
            if code in ("InvalidAccessKeyId", "SignatureDoesNotMatch", "403"):
                raise AuthenticationError(str(e)) from e
            raise BackendConnectionError(str(e)) from e
        except BotoCoreError as e:
            raise BackendConnectionError(str(e)) from e

    def close(self) -> None:
        # boto3 clients don't hold a persistent connection the way SSH
        # does; dropping the reference is enough to mark "disconnected".
        self._client = None

    def _client_or_raise(self):
        if self._client is None:
            raise BackendConnectionError(
                "Not connected. Call connect() or use as a context manager."
            )
        return self._client

    # --- core transfer -----------------------------------------------

    def upload(
        self,
        source: FileSource,
        remote_path: str,
        *,
        overwrite: bool = True,
        resume: bool = False,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        if resume:
            raise UnsupportedOperationError(
                "The S3 backend does not yet support resume=True. "
                "S3 resumable uploads require tracking a multipart upload ID "
                "across sessions, which is not implemented. "
                "Use the SFTP backend, or retry the full upload."
            )
        client = self._client_or_raise()
        bucket = self._config.bucket

        if not overwrite and self.exists(remote_path):
            raise RemoteFileExistsError(remote_path)

        start = time.monotonic()
        is_path = isinstance(source, (str, Path))
        size = Path(source).stat().st_size if is_path else None
        cb = _ProgressAdapter(progress_callback, size) if progress_callback else None

        try:
            if is_path:
                client.upload_file(str(source), bucket, remote_path, Callback=cb)
            else:
                client.upload_fileobj(source, bucket, remote_path, Callback=cb)
        except (ClientError, BotoCoreError) as e:
            raise TransferInterruptedError(str(e)) from e

        if size is None:
            size = self.get_metadata(remote_path).size

        return TransferResult(
            local_path=str(source) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=size,
            duration_seconds=time.monotonic() - start,
        )

    def download(
        self,
        remote_path: str,
        destination: FileSource,
        *,
        resume: bool = False,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferResult:
        if resume:
            raise UnsupportedOperationError(
                "The S3 backend does not yet support resume=True for downloads."
            )
        client = self._client_or_raise()
        bucket = self._config.bucket

        if not self.exists(remote_path):
            raise RemoteFileNotFoundError(remote_path)

        start = time.monotonic()
        size = self.get_metadata(remote_path).size
        cb = _ProgressAdapter(progress_callback, size) if progress_callback else None
        is_path = isinstance(destination, (str, Path))

        try:
            if is_path:
                client.download_file(bucket, remote_path, str(destination), Callback=cb)
            else:
                client.download_fileobj(bucket, remote_path, destination, Callback=cb)
        except (ClientError, BotoCoreError) as e:
            raise TransferInterruptedError(str(e)) from e

        return TransferResult(
            local_path=str(destination) if is_path else None,
            remote_path=remote_path,
            bytes_transferred=size,
            duration_seconds=time.monotonic() - start,
        )

    # --- metadata / browsing -----------------------------------------

    def exists(self, remote_path: str) -> bool:
        client = self._client_or_raise()
        try:
            client.head_object(Bucket=self._config.bucket, Key=remote_path)
            return True
        except ClientError as e:
            if e.response.get("Error", {}).get("Code") in ("404", "NoSuchKey"):
                return False
            raise

    def get_metadata(self, remote_path: str) -> FileMetadata:
        client = self._client_or_raise()
        try:
            resp = client.head_object(Bucket=self._config.bucket, Key=remote_path)
        except ClientError as e:
            if e.response.get("Error", {}).get("Code") in ("404", "NoSuchKey"):
                raise RemoteFileNotFoundError(remote_path) from e
            raise
        return FileMetadata(
            path=remote_path,
            size=resp["ContentLength"],
            last_modified=resp.get("LastModified"),
            is_dir=False,
            etag=resp.get("ETag"),
        )

    def list_dir(self, remote_path: str) -> Iterator[FileMetadata]:
        client = self._client_or_raise()
        prefix = remote_path.rstrip("/") + "/" if remote_path else ""
        paginator = client.get_paginator("list_objects_v2")
        pages = paginator.paginate(
            Bucket=self._config.bucket, Prefix=prefix, Delimiter="/"
        )
        for page in pages:
            for common_prefix in page.get("CommonPrefixes", []):
                yield FileMetadata(path=common_prefix["Prefix"], size=0, is_dir=True)
            for obj in page.get("Contents", []):
                if obj["Key"] == prefix:
                    continue  # skip the directory marker object itself
                yield FileMetadata(
                    path=obj["Key"],
                    size=obj["Size"],
                    last_modified=obj.get("LastModified"),
                    is_dir=False,
                    etag=obj.get("ETag"),
                )

    # --- mutation --------------------------------------------------

    def delete(self, remote_path: str) -> None:
        if not self.exists(remote_path):
            raise RemoteFileNotFoundError(remote_path)
        self._client_or_raise().delete_object(Bucket=self._config.bucket, Key=remote_path)

    def mkdir(self, remote_path: str, *, parents: bool = False) -> None:
        # Cosmetic only — S3 needs no directories to exist before
        # writing a key under that prefix.
        key = remote_path.rstrip("/") + "/"
        self._client_or_raise().put_object(Bucket=self._config.bucket, Key=key, Body=b"")

    def move(self, src_remote_path: str, dst_remote_path: str) -> None:
        client = self._client_or_raise()
        bucket = self._config.bucket
        if not self.exists(src_remote_path):
            raise RemoteFileNotFoundError(src_remote_path)
        client.copy_object(
            Bucket=bucket,
            CopySource={"Bucket": bucket, "Key": src_remote_path},
            Key=dst_remote_path,
        )
        client.delete_object(Bucket=bucket, Key=src_remote_path)
