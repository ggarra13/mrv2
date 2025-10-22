#!/usr/bin/env python3

import os
import re
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin

MATCH_REGEX=re.compile(r"arm64|aarch64")

# Base SourceForge directory URL
VULKAN_URL = "https://sourceforge.net/projects/mrv2/files/beta/vulkan/"
OPENGL_URL = "https://sourceforge.net/projects/mrv2/files/beta/opengl/"

# Local directory to save downloads
VULKAN_DIR = "vulkan"
OPENGL_DIR = "opengl"

# Create output directory if missing
os.makedirs(VULKAN_DIR, exist_ok=True)
os.makedirs(OPENGL_DIR, exist_ok=True)


def download_url(base_url, dest_dir):

    # Get the HTML page
    print(f"Fetching file list from {base_url}...")
    response = requests.get(base_url)
    response.raise_for_status()

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

    # Download each file
    for filename, file_url in links:
        if filename == 'README.md':
            continue

        if not MATCH_REGEX.search(filename):
            print(f"Skipping non-regex file: {filename}")
            continue
    
    
        output_path = os.path.join(dest_dir, filename)
        if os.path.exists(output_path):
            print(f"Skipping existing file: {filename}")
            continue

        print(f"Downloading {filename}...")
        with requests.get(file_url, stream=True) as r:
            r.raise_for_status()
            with open(output_path, "wb") as f:
                for chunk in r.iter_content(chunk_size=8192):
                    f.write(chunk)
            print(f"Saved {filename} ({os.path.getsize(output_path)} bytes)")

    print("✅ All files downloaded successfully.")


download_url(VULKAN_URL, VULKAN_DIR)
download_url(OPENGL_URL, OPENGL_DIR)
