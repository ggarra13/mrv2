"""Example usage of the file_transfer library.

The key point: `upload_report()` below never branches on backend
type. Swapping SFTP for S3 (or adding a third backend later) is a
one-line change at the call site — only the config object differs.
"""

from file_transfer import create_backend
from file_transfer.backends.s3 import S3Config
from file_transfer.backends.sftp import SFTPConfig


def print_progress(sent: int, total) -> None:
    pct = f"{sent / total:.0%}" if total else f"{sent} bytes"
    print(f"  progress: {pct}")


def upload_report(backend_name: str, config) -> None:
    backend = create_backend(backend_name, config)
    with backend:
        backend.mkdir("reports/2026", parents=True)
        result = backend.upload(
            "/home/gga/Movies/4K/Top_Gun.mp4",
            "reports/2026/Top_Gun.mp4",
            progress_callback=print_progress,
        )
        print(
            f"Uploaded {result.bytes_transferred} bytes "
            f"in {result.duration_seconds:.2f}s"
        )

        for entry in backend.list_dir("reports/2026"):
            print(entry.path, entry.size, entry.is_dir)

        result = backend.download(
            "reports/2026/q2_report.csv",
            "downloaded_report.csv",
            progress_callback=print_progress,
        )


if __name__ == "__main__":
    # Same calling code, two different backends:
    upload_report(
        "sftp",
        SFTPConfig(
            host="72.60.240.137",
            port=22,
            username="licenseuser",
            password="mrv21973!",
            # private_key_path="/secrets/id_rsa",
        ),
    )

    upload_report(
        "s3",
        S3Config(
            bucket="my-reports-bucket",
            region="us-east-1",
        ),
    )
