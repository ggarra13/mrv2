"""Example usage of the file_transfer library.

The key point: `upload_report()` below never branches on backend
type. Swapping SFTP for S3 (or adding a third backend later) is a
one-line change at the call site — only the config object differs.
"""

from file_transfer import create_backend
from file_transfer.backends.sftp import SFTPConfig

import argparse
from pathlib import Path

#
# Script version
#
VERSION = 1.0

#
# Script description
#
description=f"""
upload_sftp v{VERSION}

A script to upload a file to a server.
"""

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('local_filename', type=str,
                    help='Local filename to upload.')
parser.add_argument('remote_filename', type=str,
                    help='Name for remote filename once uploaded.')
parser.add_argument('--host', type=str, help='Host to upload file to.')
parser.add_argument('--port', type=int, nargs='?', default=22,
                    help='Port to upload file to')
parser.add_argument('-u', '--username', type=str, nargs='?', default=None,
                    help='Username to login as')
parser.add_argument('-p', '--password', type=str, nargs='?', default=None,
                    help='Optional password to login with')
parser.add_argument('-c', '--credentials', type=str, nargs='?', default=None,
                    help='Credentials to login with')

args = parser.parse_args()

#
# Parsed arguments
#
local_filename = args.local_filename
remote_filename = args.remote_filename
host = args.host
port = args.port
username = args.username
password = args.password
credentials = args.credentials

def print_progress(sent: int, total) -> None:
    pct = f"{sent / total:.0%}" if total else f"{sent} bytes"
    print(f"  progress: {pct}")


def upload_sftp(backend_name: str, config,
                filename, remote_filename) -> None:
    backend = create_backend(backend_name, config)
    with backend:
        result = backend.upload(
            filename,
            remote_filename,
            progress_callback=print_progress,
        )
        print(
            f"Uploaded {result.bytes_transferred} bytes "
            f"in {result.duration_seconds:.2f}s"
        )

if __name__ == "__main__":
    # upload_report(
    #     "sftp",
    #     SFTPConfig(
    #         host="72.60.240.137",
    #         port=22,
    #         username="licenseuser",
    #         password="mrv21973!",
    #         # private_key_path="/secrets/id_rsa",
    #     ),
    # )
    
    upload_sftp(
        "sftp",
        SFTPConfig(
            host=host,
            port=port,
            username=username,
            password=password,
            private_key_path=credentials
        ),
        local_filename,
        remote_filename
    )
        
