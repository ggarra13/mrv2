"""Data structures shared across all backends."""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime
from typing import Callable, Optional

# Called as progress_callback(bytes_transferred, total_bytes).
# total_bytes may be None when the size can't be known in advance
# (e.g. uploading from an arbitrary stream).
ProgressCallback = Callable[[int, Optional[int]], None]


@dataclass(frozen=True)
class FileMetadata:
    """Metadata for a single remote file or directory entry."""

    path: str
    size: int
    last_modified: Optional[datetime] = None
    is_dir: bool = False
    etag: Optional[str] = None  # checksum/version marker, backend-specific


@dataclass(frozen=True)
class TransferResult:
    """Returned by upload()/download() to describe what happened."""

    local_path: Optional[str]
    remote_path: str
    bytes_transferred: int
    duration_seconds: float
    resumed_from_offset: int = 0
    """Byte offset at which this transfer picked up. 0 means it
    started from scratch. Non-zero means a previous interrupted
    transfer was detected and resumed from that position."""


@dataclass
class RetryPolicy:
    """Shared retry configuration, honored by all backends for
    transient/network failures."""

    max_attempts: int = 3
    backoff_seconds: float = 1.0
    backoff_multiplier: float = 2.0
