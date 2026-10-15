# file_transfer

A small Python library that gives you one interface for file transfers,
with pluggable backends (SFTP, S3 included; more can be added without
touching calling code).

## Design

```
file_transfer/
  base.py          FileTransferBackend (abstract interface)
  exceptions.py     common exception hierarchy
  models.py          FileMetadata, TransferResult, RetryPolicy
  factory.py          create_backend(name, config) registry
  backends/
    sftp.py        SFTPBackend + SFTPConfig (paramiko)
    s3.py            S3Backend + S3Config (boto3)
```

**One interface, many backends.** `FileTransferBackend` is an ABC with
`upload`, `download`, `exists`, `get_metadata`, `list_dir`, `delete`,
`mkdir`, `move`, plus context-manager support for connect/close. Code
written against this interface works unchanged regardless of which
backend is plugged in.

**Backends own their config, not the caller.** Each backend has its
own dataclass (`SFTPConfig`, `S3Config`) holding exactly what it needs
(host/key vs. bucket/region). The interface itself stays free of
backend-specific fields.

**Errors are normalized.** Every backend catches its SDK's native
exceptions (`paramiko.SSHException`, `botocore.ClientError`, ...) and
re-raises one of the shared exceptions in `exceptions.py`
(`RemoteFileNotFoundError`, `AuthenticationError`, etc.). Calling code
never needs to import or branch on backend-specific error types — this
is the main thing that makes backends actually swappable in practice.

**Pluggable via registry, not a big if/elif.** `factory.py` maps a
string name (`"sftp"`, `"s3"`) to a constructor. New backends register
themselves with `register_backend(name, BackendClass)` — adding Azure
Blob or GCS later means adding a new file under `backends/`, not
editing existing code (open/closed principle). A backend whose SDK
isn't installed (e.g. boto3 missing) is simply absent from the
registry rather than breaking the import of the whole package.

**Streaming-friendly.** `upload`/`download` accept either a local path
*or* an open binary file object, so large files can be piped through
without hitting disk, and progress can be tracked via an optional
`progress_callback(bytes_transferred, total_bytes)`.

**Retries are backend-internal, policy is shared.** `RetryPolicy`
(max attempts, backoff) is a shared dataclass; each backend applies it
to its own transient-failure conditions (SSH/network errors for SFTP,
botocore errors for S3) without leaking the retry mechanics to callers.

## Usage

```python
from file_transfer import create_backend
from file_transfer.backends.s3 import S3Config

backend = create_backend("s3", S3Config(bucket="my-bucket", region="us-east-1"))

with backend:
    backend.upload("report.csv", "reports/2026/q2.csv")
    for entry in backend.list_dir("reports/2026"):
        print(entry.path, entry.size)
```

Swapping to SFTP is purely a config change — `upload_report()` in
`example_usage.py` is identical for both backends:

```python
from file_transfer.backends.sftp import SFTPConfig

backend = create_backend("sftp", SFTPConfig(
    host="sftp.example.com", username="svc", private_key_path="/secrets/id_rsa",
))
```

## Install

```
pip install paramiko   # for the SFTP backend
pip install boto3       # for the S3 backend
```
Only the dependency for the backend(s) you actually use is required.

## Extending with a new backend

1. Create `backends/<name>.py` with a `<Name>Config` dataclass and a
   `<Name>Backend(FileTransferBackend)` implementing all abstract
   methods, translating its SDK's exceptions into the shared
   exception types.
2. Register it in `factory.py`'s `_register_builtin_backends()`
   (wrapped in a `try/except ImportError`, same pattern as the
   existing two), or call `register_backend()` yourself if you're
   shipping it as a separate package.

## Notes on semantics

- S3 has no real directories: `mkdir()` writes a cosmetic zero-byte
  marker object (never required), and `list_dir()` emulates a
  non-recursive listing via key-prefix delimiters.
- `list_dir()` is intentionally non-recursive on both backends, to
  keep behavior consistent and let callers opt into recursion
  explicitly if they need it.
- This library is currently synchronous/blocking by design — wrap
  calls in a thread pool if you need concurrency, since paramiko and
  boto3's underlying I/O isn't natively async.
