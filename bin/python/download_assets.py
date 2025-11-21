#!/usr/bin/env python3

import argparse
import os
import re
import requests # Still needed for initial directory listing and parsing
from bs4 import BeautifulSoup
from urllib.parse import urljoin
import subprocess # New import for running curl

VERSION=1.0
MATCH_REGEX=re.compile(r"arm64|aarch64")

description=f"""
po_merge v{VERSION}

A program to download all betas or version assets from sourceforge.
"""
            
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('version', type=str,
                    help='Version to download')

args = parser.parse_args()
branch=args.version

# Base SourceForge directory URL

if branch == 'beta':
    VULKAN_URL = "https://sourceforge.net/projects/mrv2/files/beta/vulkan/"
    OPENGL_URL = "https://sourceforge.net/projects/mrv2/files/beta/opengl/"
else:
    VULKAN_URL = f"https://sourceforge.net/projects/mrv2/files/{branch}"
    OPENGL_URL = f"https://sourceforge.net/projects/mrv2/files/{branch}"

# Local directory to save downloads
VULKAN_DIR = "releases/vulkan"
OPENGL_DIR = "releases/opengl"

# Create output directory if missing
os.makedirs(VULKAN_DIR, exist_ok=True)
os.makedirs(OPENGL_DIR, exist_ok=True)


def download_url(base_url, dest_dir, mrv2_prefix):

    # Get the HTML page (This part still uses requests/BeautifulSoup)
    print(f"Fetching file list from {base_url}...")
    try:
        response = requests.get(base_url)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching URL: {e}")
        return

    # Parse HTML
    soup = BeautifulSoup(response.text, "html.parser")

    # Find all download links (they end with "/download")
    links = []
    for a in soup.find_all("a", href=True):
        href = a["href"]
        if href.endswith("/download"):
            file_url = urljoin(base_url, href)
            filename = href.strip("/").split("/")[-2]  # Extract filename before '/download'
            links.append((filename, file_url))

    print(f"Found {len(links)} files to download.")

    # Download each file using curl
    for filename, file_url in links:
        if filename == 'README.md':
            continue

        PREFIX_REGEX=re.compile(rf"^{mrv2_prefix}-")
        if not PREFIX_REGEX.search(filename):
            print(f"Skipping non-prefix {mrv2_prefix} file: {filename}")
            continue

        if not MATCH_REGEX.search(filename):
            print(f"Skipping non-regex file: {filename}")
            continue
        
        output_path = os.path.join(dest_dir, filename)
        
        # Check for existing file size to handle resume/skip logic
        file_exists = os.path.exists(output_path)
        
        if file_exists:
            # Check if the file is already fully downloaded
            # SourceForge download links usually redirect, so getting the exact remote size is tricky
            # The safest approach is to either SKIP if it exists or use 'curl -C -' to resume.
            print(f"File {filename} exists. Attempting to **resume** download...")
        else:
            print(f"Downloading {filename}...")

        # Construct the curl command:
        # -L: Follow redirects (necessary for SourceForge download links)
        # -C -: Resume a broken/partial transfer
        # -o <file>: Write output to a local file
        curl_command = [
            "curl",
            "-L",
            "-C", "-",
            "-A", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 Chrome/142.0.0.0 Safari/537.36",
            "-o", output_path,
            file_url
        ]
        
        try:
            # Execute the curl command
            # Note: This will block until curl finishes (or fails)
            result = subprocess.run(
                curl_command, 
                check=True, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE,
                text=True
            )
            # The check=True argument raises a CalledProcessError if curl exits with a non-zero status
            
            # Print success message. curl outputs progress to stderr by default
            # We capture stdout/stderr but rely on the exit code.
            if result.returncode == 0:
                print(f"Successfully downloaded/resumed {filename}")
            
        except subprocess.CalledProcessError as e:
            print(f"❌ Error downloading {filename} with curl (Exit code {e.returncode}):")
            print(f"   {e.stderr.strip()}")
        except FileNotFoundError:
            print("❌ Error: 'curl' command not found. Please ensure curl is installed and in your PATH.")
            return

    print("✅ All download attempts completed.")

download_url(VULKAN_URL, VULKAN_DIR, 'vmrv2')
download_url(OPENGL_URL, OPENGL_DIR, 'mrv2')
