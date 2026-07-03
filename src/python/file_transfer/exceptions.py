"""Custom exception hierarchy for the file transfer library.

Every backend implementation is responsible for catching its own
SDK-specific errors (paramiko, botocore, etc.) and re-raising one of
these common exceptions, so that calling code never has to import or
handle backend-specific exception types.
"""

from __future__ import annotations


class TransferError(Exception):
    """Base class for all errors raised by this library."""


class BackendConnectionError(TransferError):
    """Connection to the remote backend could not be established, or
    was lost mid-operation."""


class AuthenticationError(TransferError):
    """Credentials are missing, invalid, or rejected by the remote
    service."""


class RemoteFileNotFoundError(TransferError):
    """An operation referenced a remote path that does not exist."""


class RemoteFileExistsError(TransferError):
    """An operation would overwrite an existing remote file and
    overwrite=False was specified."""


class RemotePermissionError(TransferError):
    """The backend rejected an operation due to insufficient
    permissions."""


class TransferInterruptedError(TransferError):
    """An upload or download was interrupted partway through, e.g. due
    to a dropped connection or exhausted retries."""


class TransferCancelledError(TransferError):
    """A transfer was cancelled by the caller via TransferHandle.cancel().

    The remote .part file (for uploads) is left in place so the transfer
    can be resumed later with resume=True.
    """
    def __init__(self, message: str = "Transfer cancelled", offset: int = 0):
        super().__init__(message)
        self.offset = offset
        """Byte offset at which the transfer was cancelled."""


class UnsupportedOperationError(TransferError):
    """The backend does not support a particular operation (e.g. some
    object stores have no real concept of directories)."""


class ConfigurationError(TransferError):
    """A backend was constructed with invalid, incomplete, or unknown
    configuration."""
