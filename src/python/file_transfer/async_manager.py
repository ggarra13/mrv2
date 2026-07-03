"""Async transfer manager: run multiple uploads/downloads concurrently
without blocking the UI thread.

Design rationale
----------------
Network I/O (SFTP, S3) releases Python's GIL, so multiple transfers in a
ThreadPoolExecutor run in *true* parallel — no async/await needed and no
C++ required. Each submitted job gets its own backend connection (a
paramiko transport is not safe to share across threads) created from a
caller-supplied factory function.

Cancellation is cooperative: each transfer checks a threading.Event every
chunk (32 KiB by default) and raises TransferCancelledError promptly.
Cancelled uploads leave their .part file in place so they can be resumed
later with resume=True.

UI integration
--------------
Progress callbacks fire from worker threads. Each UI toolkit has its own
safe way to hand work back to the main thread:

  PyQt / PySide  →  emit a Qt signal (see PyQtProgressBridge below)
  tkinter        →  queue.Queue + widget.after() polling (see TkProgressBridge)

Both bridge classes wrap any ProgressCallback and can be passed directly
to submit_upload / submit_download.
"""

from __future__ import annotations

import queue
import threading
from concurrent.futures import Future, ThreadPoolExecutor
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Callable, List, Optional

from .base import FileTransferBackend
from .exceptions import TransferCancelledError
from .models import ProgressCallback, TransferResult


# ---------------------------------------------------------------------------
# TransferHandle
# ---------------------------------------------------------------------------

class TransferState(Enum):
    PENDING    = auto()   # queued, not yet started
    RUNNING    = auto()   # actively transferring
    DONE       = auto()   # completed successfully
    CANCELLING = auto()   # cancel requested, worker hasn't finished yet
    CANCELLED  = auto()   # worker acknowledged the cancel
    FAILED     = auto()   # raised an exception other than cancellation


@dataclass
class TransferHandle:
    """Returned by submit_upload / submit_download.

    Thread-safe: all properties and methods can be called from any thread,
    including the UI thread.
    """
    _future: Future = field(repr=False)
    _cancel_event: threading.Event = field(repr=False)
    remote_path: str = ""
    local_path: Optional[str] = None

    def cancel(self) -> None:
        """Request cancellation. Returns immediately; the worker stops
        at the next chunk boundary (within one chunk_size of I/O).

        After cancel() the state transitions through CANCELLING → CANCELLED.
        Cancelled uploads leave a .part file so they can be resumed later.
        """
        self._cancel_event.set()
        # Also try to cancel if the job hasn't started yet
        self._future.cancel()

    @property
    def state(self) -> TransferState:
        if self._future.cancelled():
            return TransferState.CANCELLED
        if self._future.running():
            if self._cancel_event.is_set():
                return TransferState.CANCELLING
            return TransferState.RUNNING
        if self._future.done():
            exc = self._future.exception()
            if exc is None:
                return TransferState.DONE
            if isinstance(exc, TransferCancelledError):
                return TransferState.CANCELLED
            return TransferState.FAILED
        # not yet started
        if self._cancel_event.is_set():
            return TransferState.CANCELLING
        return TransferState.PENDING

    def done(self) -> bool:
        """True when the transfer has finished, failed, or been cancelled."""
        return self.state in (
            TransferState.DONE, TransferState.CANCELLED, TransferState.FAILED
        )

    def result(self, timeout: Optional[float] = None) -> TransferResult:
        """Block until done and return the TransferResult, or re-raise
        the exception if the transfer failed.

        Raises TransferCancelledError if the transfer was cancelled.
        Raises concurrent.futures.TimeoutError if timeout elapses.
        """
        return self._future.result(timeout=timeout)

    def exception(self) -> Optional[BaseException]:
        """Return the exception raised by the transfer, or None."""
        if self._future.done():
            return self._future.exception()
        return None

    def add_done_callback(self, fn: Callable[["TransferHandle"], None]) -> None:
        """Register a callback invoked when the transfer finishes, fails,
        or is cancelled. Called from the worker thread — use a bridge if
        you need to update the UI.
        """
        self._future.add_done_callback(lambda _: fn(self))


# ---------------------------------------------------------------------------
# AsyncTransferManager
# ---------------------------------------------------------------------------

BackendFactory = Callable[[], FileTransferBackend]


class AsyncTransferManager:
    """Runs file transfers concurrently on a thread pool.

    Each submitted job creates its own backend connection via the supplied
    factory, so concurrent SFTP transfers don't share a transport.

    Usage::

        factory = lambda: SFTPBackend(SFTPConfig(host=..., username=..., ...))
        manager = AsyncTransferManager(factory, max_workers=4)

        with manager:
            handle = manager.submit_download(
                "remote/big_file.iso",
                "/local/big_file.iso",
                resume=True,
                progress_callback=my_callback,
            )
            # ... UI keeps running ...
            handle.cancel()          # if the user clicks Cancel
            result = handle.result() # blocks; raises if failed/cancelled
    """

    def __init__(
        self,
        backend_factory: BackendFactory,
        *,
        max_workers: int = 4,
    ) -> None:
        self._factory = backend_factory
        self._executor = ThreadPoolExecutor(max_workers=max_workers)
        self._handles: List[TransferHandle] = []
        self._lock = threading.Lock()

    # --- context manager ---------------------------------------------

    def __enter__(self) -> "AsyncTransferManager":
        return self

    def __exit__(self, *_) -> None:
        self.shutdown(wait=True)

    def shutdown(self, *, wait: bool = True, cancel_pending: bool = False) -> None:
        """Shut down the thread pool.

        cancel_pending=True cancels all transfers that haven't started
        yet; running transfers are allowed to finish (or respond to
        their own cancel_event if you called handle.cancel() separately).
        """
        if cancel_pending:
            with self._lock:
                for handle in self._handles:
                    if handle.state == TransferState.PENDING:
                        handle.cancel()
        self._executor.shutdown(wait=wait)

    # --- submit ------------------------------------------------------

    def submit_upload(
        self,
        source,
        remote_path: str,
        *,
        overwrite: bool = True,
        resume: bool = False,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferHandle:
        """Schedule an upload and return a TransferHandle immediately."""
        cancel_event = threading.Event()
        future = self._executor.submit(
            self._run_upload,
            source, remote_path,
            overwrite, resume, cancel_event, progress_callback,
        )
        handle = TransferHandle(
            _future=future,
            _cancel_event=cancel_event,
            remote_path=remote_path,
            local_path=str(source) if not hasattr(source, "read") else None,
        )
        with self._lock:
            self._handles.append(handle)
        return handle

    def submit_download(
        self,
        remote_path: str,
        destination,
        *,
        resume: bool = False,
        progress_callback: Optional[ProgressCallback] = None,
    ) -> TransferHandle:
        """Schedule a download and return a TransferHandle immediately."""
        cancel_event = threading.Event()
        future = self._executor.submit(
            self._run_download,
            remote_path, destination,
            resume, cancel_event, progress_callback,
        )
        handle = TransferHandle(
            _future=future,
            _cancel_event=cancel_event,
            remote_path=remote_path,
            local_path=str(destination) if not hasattr(destination, "write") else None,
        )
        with self._lock:
            self._handles.append(handle)
        return handle

    # --- internal workers --------------------------------------------

    def _run_upload(self, source, remote_path, overwrite, resume,
                    cancel_event, progress_callback):
        backend = self._factory()
        with backend:
            return backend.upload(
                source, remote_path,
                overwrite=overwrite,
                resume=resume,
                cancel_event=cancel_event,
                progress_callback=progress_callback,
            )

    def _run_download(self, remote_path, destination, resume,
                      cancel_event, progress_callback):
        backend = self._factory()
        with backend:
            return backend.download(
                remote_path, destination,
                resume=resume,
                cancel_event=cancel_event,
                progress_callback=progress_callback,
            )

    # --- introspection -----------------------------------------------

    def active_handles(self) -> List[TransferHandle]:
        """Return handles for all transfers that are not yet done."""
        with self._lock:
            return [h for h in self._handles if not h.done()]

    def all_handles(self) -> List[TransferHandle]:
        with self._lock:
            return list(self._handles)


