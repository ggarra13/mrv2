"""Factory/registry so callers can construct a backend from
configuration without importing backend-specific classes directly,
and so new backends (Azure Blob, GCS, FTP...) can be added without
modifying this file.
"""

from __future__ import annotations

from typing import Any, Callable, Dict

from .base import FileTransferBackend
from .exceptions import ConfigurationError

_REGISTRY: Dict[str, Callable[..., FileTransferBackend]] = {}


def register_backend(name: str, factory: Callable[..., FileTransferBackend]) -> None:
    """Register a backend constructor under a string name, e.g.
    register_backend("sftp", SFTPBackend).

    `factory` is typically the backend class itself (called with its
    config object as the sole argument), but can be any callable
    returning a FileTransferBackend — useful for custom wiring or
    swapping in a test double.
    """
    _REGISTRY[name] = factory


def create_backend(name: str, config: Any) -> FileTransferBackend:
    """Construct a backend by name. Raises ConfigurationError if the
    name is not registered (e.g. its dependency, like boto3, isn't
    installed)."""
    try:
        factory = _REGISTRY[name]
    except KeyError as e:
        available = ", ".join(sorted(_REGISTRY)) or "(none registered)"
        raise ConfigurationError(
            f"Unknown backend '{name}'. Available backends: {available}"
        ) from e
    return factory(config)


def _register_builtin_backends() -> None:
    # Imports are local so that, e.g., boto3 is only required if the
    # caller actually wants S3 — importing this package shouldn't force
    # every backend's dependency to be installed.
    try:
        from .backends.sftp import SFTPBackend
        register_backend("sftp", SFTPBackend)
    except ImportError:
        pass

    try:
        from .backends.s3 import S3Backend
        register_backend("s3", S3Backend)
    except ImportError:
        pass


_register_builtin_backends()
