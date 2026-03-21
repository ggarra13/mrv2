import argparse
import os
import re
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin
import subprocess
import json

VERSION=1.3
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
    """
    Download files from SourceForge with resume support.
    
    Important: SourceForge uses redirects to mirror servers. To ensure resume
    works correctly, we resolve the redirect chain ONCE to get the final mirror
    URL, then use that same URL for both size checking and downloading. This
    prevents issues where different requests might redirect to different mirrors.
    """

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
            # First, resolve the redirect chain to get the final download URL
            # This ensures we use the same mirror for both HEAD and GET requests
            print(f"Resolving download URL for {filename}...")
            redirect_response = requests.head(file_url, headers=headers, allow_redirects=True)
            final_url = redirect_response.url
            
            # Check if file exists and get current size for resume
            resume_pos = 0
            mode = 'wb'
            
            if os.path.exists(output_path):
                resume_pos = os.path.getsize(output_path)
                
                # Check the remote file size using the resolved final URL
                head_response = requests.head(final_url, headers=headers, allow_redirects=False)
                if head_response.status_code == 200:
                    remote_size = int(head_response.headers.get('Content-Length', 0))
                    
                    if remote_size > 0 and resume_pos == remote_size:
                        print(f'✓ Already complete: {filename} ({remote_size} bytes)')
                        continue
                    elif resume_pos > 0 and resume_pos < remote_size:
                        print(f'Resuming {filename} from byte {resume_pos}...')
                        mode = 'ab'
                    elif resume_pos > 0:
                        print(f'Partial file found but size mismatch, restarting: {filename}')
                        resume_pos = 0
                else:
                    print(f'Could not verify remote file size, restarting: {filename}')
                    resume_pos = 0
            
            # Set up headers for resume
            download_headers = headers.copy()
            if resume_pos > 0:
                download_headers['Range'] = f'bytes={resume_pos}-'
            
            # Fetch the file from the final URL (no more redirects)
            response = requests.get(final_url, headers=download_headers,
                                    allow_redirects=False, stream=True)
            
            # Accept both 200 (full content) and 206 (partial content)
            if response.status_code not in [200, 206]:
                response.raise_for_status()
            
            # If server doesn't support resume (returns 200 instead of 206), start over
            if resume_pos > 0 and response.status_code == 200:
                print(f'Server does not support resume, restarting: {filename}')
                resume_pos = 0
                mode = 'wb'
            
            if resume_pos == 0:
                print(f"Downloading {filename}...")
            
            # Save the binary content
            with open(output_path, mode) as f:
                for chunk in response.iter_content(chunk_size=4*1024*1024):
                    if chunk:
                        print('*',end='', flush=True)
                        f.write(chunk)

            print(f"\n✓ Download complete: {os.path.abspath(output_path)} (Size: {os.path.getsize(output_path)} bytes)")

        except requests.exceptions.RequestException as e:
            print(f"\n✗ Error downloading {filename}: {e}")
            if 'final_url' in locals():
                print(f"  Mirror URL: {final_url}")
            print(f"  Partial file kept at: {output_path}")
            print(f"  Run script again to resume download")
            # Continue to next file instead of exiting
            continue

    print("✅ All download attempts completed.")

download_url(VULKAN_URL, VULKAN_DIR, 'vmrv2')
download_url(OPENGL_URL, OPENGL_DIR, 'mrv2')
