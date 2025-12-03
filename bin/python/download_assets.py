import argparse
import os
import re
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin
import subprocess
import json

VERSION=1.1
MATCH_REGEX=re.compile(r"arm64|aarch64")

description=f"""
download_assets v{VERSION}

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
VULKAN_DIR = f"releases/{branch}/vulkan"
OPENGL_DIR = f"releases/{branch}/opengl"

# Create output directory if missing
os.makedirs(VULKAN_DIR, exist_ok=True)
os.makedirs(OPENGL_DIR, exist_ok=True)


# Optional: Set a custom User-Agent to ensure direct redirect (default python-requests works, but this mimics wget)
headers = {
    'User-Agent': 'Wget/1.21.4'
}


def parse_sourceforge_page(html_content, base_url):
    """
    Parse SourceForge page and extract file download links.
    Handles both traditional HTML links and JavaScript-embedded data.
    """
    soup = BeautifulSoup(html_content, "html.parser")
    links = []
    
    for a in soup.find_all("a", href=True):
        href = a["href"]
        if href.endswith("/download"):
            file_url = urljoin(base_url, href)
            filename = href.strip("/").split("/")[-2]
            links.append((filename, file_url))
    
    return links

def download_url(base_url, dest_dir, mrv2_prefix):

    # Get the HTML page
    print(f"Fetching file list from {base_url}...")
    try:
        # Use headers to mimic a browser request
        response = requests.get(base_url, headers=headers)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching URL: {e}")
        return

    # Parse the page for file links
    links = parse_sourceforge_page(response.text, base_url)

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

        try:
            # Fetch the file, following redirects
            response = requests.get(file_url, headers=headers,
                                    allow_redirects=True, stream=True)
            response.raise_for_status()  # Raise an error for bad status codes

            print(f"Downloading {filename}...")
            
            try:
                os.remove(output_path)
            except FileNotFoundError:
                pass   # Ignore if it doesn't exist


            # Save the binary content
            with open(output_path, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    if chunk:
                        print('*',end='', flush=True)
                        f.write(chunk)

            print(f"\nDownload complete: {os.path.abspath(filename)} (Size: {os.path.getsize(filename)} bytes)")

        except requests.exceptions.RequestException as e:
            print(f"Error downloading: {e}")
            exit(1)

    print("✅ All download attempts completed.")

download_url(VULKAN_URL, VULKAN_DIR, 'vmrv2')
download_url(OPENGL_URL, OPENGL_DIR, 'mrv2')
