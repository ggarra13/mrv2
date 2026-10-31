import argparse
import os
import re
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin
import subprocess
import time
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

global num_downloads
num_downloads = 0

KNOWN_SF_MIRRORS = [
    "ufpr", "altushost-swe", "netix", "nav", "kent", "nchc", "jaist",
    "versaweb", "cytranet", "iweb", "tenet",
]

def resolve_mirror_url(file_url, mirror_name):
    """Build a direct mirror URL using SF's ?use_mirror param."""
    return f"{file_url}?use_mirror={mirror_name}"

def benchmark_mirrors(file_url, mirrors, timeout=3):
    """
    Time a HEAD request to each mirror and return them sorted fastest-first.
    """
    results = []
    for mirror in mirrors:
        url = resolve_mirror_url(file_url, mirror)
        try:
            start = time.monotonic()
            r = requests.head(url, headers=headers, allow_redirects=True, timeout=timeout)
            elapsed = time.monotonic() - start
            if r.status_code in (200, 206):
                results.append((elapsed, mirror, r.url))
                #print(f"  {mirror}: {elapsed:.2f}s → {r.url}")
        except requests.exceptions.RequestException:
            #print(f"  {mirror}: unreachable")
            pass
    results.sort()
    return results  # [(latency, mirror_name, resolved_url), ...]

def download_with_best_mirror(file_url, output_path, filename):
    print(f"Benchmarking mirrors for {filename}...")
    ranked = benchmark_mirrors(file_url, KNOWN_SF_MIRRORS)

    global num_downloads
    downloaded = False
    temp_output_path = output_path + ".part"

    for latency, mirror, final_url in ranked:
        print(f"Trying mirror '{mirror}' ({latency:.2f}s)...")
        try:
            # Check if final file already exists and is complete
            # (Optional: you can also check remote size against output_path directly)
            
            # Check temp file size for resume
            resume_pos = (
                os.path.getsize(temp_output_path)
                if os.path.exists(temp_output_path)
                else 0
            )

            # Get the complete remote file size
            head_response = requests.head(
                final_url,
                headers=headers,
                allow_redirects=True,
                timeout=15,
            )
            head_response.raise_for_status()

            remote_size_header = head_response.headers.get("Content-Length")
            remote_size = (
                int(remote_size_header)
                if remote_size_header is not None
                else 0
            )

            if remote_size > 0 and os.path.exists(output_path):
                if os.path.getsize(output_path) == remote_size:
                    print(f"✓ Already complete: {filename} ({remote_size} bytes)")
                    downloaded = True
                    break

            if remote_size > 0 and resume_pos == remote_size:
                print(f"✓ Temp file already complete, finalizing: {filename}")
                os.replace(temp_output_path, output_path)
                downloaded = True
                break

            if remote_size <= 0 or resume_pos > remote_size:
                print(f"Restarting download: {filename}")
                resume_pos = 0
                if os.path.exists(temp_output_path):
                    os.remove(temp_output_path)

            download_headers = headers.copy()
            if resume_pos > 0:
                download_headers["Range"] = f"bytes={resume_pos}-"
                print(f"Resuming {filename} from byte {resume_pos}...")

            with requests.get(
                final_url,
                headers=download_headers,
                allow_redirects=True,
                stream=True,
                timeout=(15, 120),
            ) as response:

                if response.status_code == 206:
                    content_range = response.headers.get("Content-Range", "")
                    expected_prefix = f"bytes {resume_pos}-"

                    if resume_pos == 0 or not content_range.startswith(expected_prefix):
                        raise requests.exceptions.RequestException(
                            f"Unexpected Content-Range: {content_range}"
                        )
                    mode = "ab"

                elif response.status_code == 200:
                    mode = "wb"
                    resume_pos = 0

                else:
                    response.raise_for_status()
                    raise requests.exceptions.RequestException(
                        f"Unexpected HTTP status: {response.status_code}"
                    )

                with open(temp_output_path, mode) as f:
                    for chunk in response.iter_content(chunk_size=4 * 1024 * 1024):
                        if chunk:
                            print("*", end="", flush=True)
                            f.write(chunk)

            actual_size = os.path.getsize(temp_output_path)

            if remote_size > 0 and actual_size != remote_size:
                raise requests.exceptions.RequestException(
                    f"Size mismatch: got {actual_size}, expected {remote_size}"
                )

            # Move temp file to final destination only after successful verification
            os.replace(temp_output_path, output_path)

            print(
                f"\n✓ Download complete: {os.path.abspath(output_path)} "
                f"(Size: {actual_size} bytes)"
            )
            downloaded = True
            break
            
        except requests.exceptions.RequestException as e:
            print(f"Mirror '{mirror}' failed: {e}, trying next...")

    if not downloaded:
        print(f"✗ ALL MIRRORS FAILED FOR: {filename}")
    else:
        num_downloads += 1

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
            #print(f"Skipping non-prefix {mrv2_prefix} file: {filename}")
            continue

        if not MATCH_REGEX.search(filename):
            #print(f"Skipping non-regex file: {filename}")
            continue

        output_path = os.path.join(dest_dir, filename)
        download_with_best_mirror(file_url, output_path, filename)
        
    print("✅ All download attempts completed.")

download_url(VULKAN_URL, VULKAN_DIR, 'vmrv2')
download_url(OPENGL_URL, OPENGL_DIR, 'mrv2')

print("Number of downloads are:",num_downloads)
