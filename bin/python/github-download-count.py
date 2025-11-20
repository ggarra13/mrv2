import os
import sys
import itertools
import re
from datetime import datetime, timezone, timedelta
import argparse

# --- Third-Party Imports ---
try:
    import requests
except ImportError:
    print("Error: requests is not installed")
    print("Installing Requests is simple with pip:\n  pip install requests")
    print("More info: http://docs.python-requests.org/en/latest/")
    sys.exit(1)

# --- Standard Library Imports (Modernized) ---
import json
from io import StringIO

# Assuming 'functions' is a file you have that contains 'format_number'
try:
    from functions import format_number
except ImportError:
    # Placeholder for format_number if the external file is missing
    def format_number(n, width):
        """Placeholder for a function to format a number for display."""
        return str(n).rjust(width)

# --- Global Configuration and Authentication ---
full_names = []
headers = {}

if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = "token %s" % os.environ["GITHUB_TOKEN"]

# --- GLOBAL TRACKERS FOR GRAND TOTALS ---
# These will accumulate downloads for each product across all sources (GitHub + SourceForge)
mrv2_grand_total = 0
vmrv2_grand_total = 0
# ----------------------------------------

def get_date_arguments():
    """Initializes and parses command-line arguments."""
    parser = argparse.ArgumentParser(
        description='Process GitHub and SourceForge download counts over a date range.'
    )

    parser.add_argument('user', type=str, help='GitHub username')
    parser.add_argument('repo', type=str, nargs='?', default=None, help='Repository name (optional)')
    parser.add_argument('tag', type=str, nargs='?', default=None, help='Tag name (optional)')
    parser.add_argument('start_date', type=str, default='2014-10-29',
                        help='Start Date (YYYY-MM-DD format). Defaults to 2014-10-29.')
    parser.add_argument('end_date', type=str, nargs='?', default=None, help='End Date (YYYY-MM-DD format, optional)')

    return parser.parse_args()

def parse_and_process_dates(args):
    """
    Parses and formats start/end dates, ensures they are UTC-aware,
    and flips them if end_dt is before start_dt.
    """
    # Define a helper function to parse date strings with flexibility
    def parse_date_string(date_str, is_end_date=False):
        # 1. Try to parse the complex format (e.g., '2025-10-05 10:43:37 -0300')
        try:
            dt = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S %z')
            dt = dt.astimezone(timezone.utc)
            return dt
        except ValueError:
            pass # Fall through to the next attempt

        try:
            # Try parsing a datetime without offset, assuming it's UTC if no offset is present
            dt = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S')
            dt = dt.replace(tzinfo=timezone.utc)
            return dt
        except ValueError:
            pass # Fall through to the next attempt
            
        # 2. Try to parse the simple YYYY-MM-DD format (as required by the original script)
        try:
            dt_naive = datetime.strptime(date_str, '%Y-%m-%d')
            # For a simple date string, set it to the start/end of the day in UTC
            if is_end_date:
                dt = dt_naive.replace(hour=23, minute=59, second=59, microsecond=999999, tzinfo=timezone.utc)
            else:
                dt = dt_naive.replace(hour=0, minute=0, second=0, microsecond=0, tzinfo=timezone.utc)
            return dt
            
        except ValueError:
            # If neither format works, raise the final error
            raise ValueError(f"Invalid date format '{date_str}'. Please use either 'YYYY-MM-DD' or 'YYYY-MM-DD HH:MM:SS +/-ZZZZ'.")

    # 1. Parse start_date
    try:
        start_dt = parse_date_string(args.start_date, is_end_date=False)
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)

    # 2. Parse or set end_date
    if not args.end_date:
        end_dt = datetime.now(timezone.utc)
    else:
        try:
            end_dt = parse_date_string(args.end_date, is_end_date=True)
        except ValueError as e:
            print(f"Error: {e}")
            sys.exit(1)

    # 3. Compare and flip dates if necessary
    if end_dt < start_dt:
        print(f"\n⚠️ **Warning:** End date ({end_dt.strftime('%Y-%m-%d')}) is before start date ({start_dt.strftime('%Y-%m-%d')}).")
        print("  **Dates have been flipped** to ensure a valid range.")
        start_dt, end_dt = end_dt, start_dt
        
        # When dates are swapped, ensure the new start_dt is at the beginning of its day
        # and the new end_dt is at the end of its day for clean API calls.
        start_dt = start_dt.replace(hour=0, minute=0, second=0, microsecond=0)
        end_dt = end_dt.replace(hour=23, minute=59, second=59, microsecond=999999)

    # 4. Correctly Calculate Difference (must be done AFTER any potential swap)
    time_difference = end_dt - start_dt
    days = time_difference.days
    hours, remainder = divmod(time_difference.seconds, 3600)
    minutes, seconds = divmod(remainder, 60)

    # Display dates and difference (using the newly swapped/ordered dates)
    print(" START DATE:", start_dt.strftime("%Y-%m-%d %H:%M:%S %Z"))
    print("    END DATE:", end_dt.strftime("%Y-%m-%d %H:%M:%S %Z"))
    print(f' DIFFERENCE: {days} days, {hours} hours, {minutes} minutes')
    print("-----------------------------------------------------------------------")

    # Format dates to 'YYYY-MM-DD' strings for use in API calls
    start_date_str = start_dt.strftime("%Y-%m-%d")
    end_date_str = end_dt.strftime("%Y-%m-%d")

    return start_date_str, end_date_str, args.user, args.repo, args.tag

def get_github_downloads(user, repo, tag):
    """
    Fetches and sums GitHub release asset download counts.
    Returns: (mrv2_total_count, vmrv2_total_count)
    """
    if not user or not repo:
        print("Skipping GitHub downloads: Both user and repository must be provided.")
        return 0, 0

    full_name = f"{user}/{repo}"
    print(f"--- GitHub Downloads for {full_name} ---")

    mrv2_total_count = 0
    vmrv2_total_count = 0
    try:
        PER_PAGE = 100
        for page in itertools.count(1):
            url = f'https://api.github.com/repos/{full_name}/releases?per_page={PER_PAGE}&page={page}'
            response = requests.get(url, headers=headers)
            response.raise_for_status()
            releases = response.json()

            if not releases:
                break

            for r in releases:
                if "assets" not in r:
                    print(f'Repo: {full_name}\t{r.get("tag_name", "N/A")} No assets')
                    continue

                if tag and r.get('tag_name') and not re.search(re.escape(tag), r['tag_name']):
                    continue

                for asset in r['assets']:
                    if tag:
                        regex = f'.*{re.escape(tag)}.*'
                        if not re.match(regex, asset['name']):
                            continue

                    downloads = asset['download_count']
                    if asset['name'].startswith('mrv2'):
                        mrv2_total_count += downloads
                    elif asset['name'].startswith('vmrv2'):
                        vmrv2_total_count += downloads
                        
                    asset_date = asset['updated_at'].split('T')[0]
                    print('{:>5} Asset: {:<40} Date: {}'.format(
                        format_number(asset['download_count'], 5),
                        asset['name'],
                        asset_date,
                    ))

            if len(releases) < PER_PAGE:
                break

    except requests.exceptions.HTTPError as e:
        print(f'HTTP Error fetching releases for repo {full_name}: {e}')
        sys.exit(1)
    except Exception as e:
        print(f'General Exception while fetching releases for repo {full_name}: {e}')
        sys.exit(1)

    print("-----------------------------------------------------------------------")
    
    formatted_mrv2_total = format_number(mrv2_total_count, 5)
    print(f'{formatted_mrv2_total} Total mrv2 Downloads (GitHub)')
    
    formatted_vmrv2_total = format_number(vmrv2_total_count, 5)
    print(f'{formatted_vmrv2_total} Total vmrv2 Downloads (GitHub)')

    total_count = mrv2_total_count + vmrv2_total_count
    formatted_total = format_number(total_count, 5)
    print(f'{formatted_total} Total Downloads for GitHub repo {full_name}')
    
    # Return separate counts
    return mrv2_total_count, vmrv2_total_count

def get_file_stats(repo, file_path, start_date, end_date):
    """Fetches download stats for a specific SourceForge file path."""
    urls_to_try = [
        f"https://sourceforge.net/projects/{repo}/files/{file_path}/stats/json?start_date={start_date}&end_date={end_date}",
        f"https://sourceforge.net/projects/{repo}/files/archive/{file_path}/stats/json?start_date={start_date}&end_date={end_date}"
    ]

    for url in urls_to_try:
        try:
            response = requests.get(url)
            response.raise_for_status()
            r = response.json()
            if 'oses' in r:
                return r['oses']
        except requests.exceptions.HTTPError:
            continue
        except Exception:
            continue
    return None

def count_sourceforge(repo, folder_name, end_date, start_date):
    """
    Fetches and sums SourceForge download counts for a folder.
    Returns: (mrv2_total_count, vmrv2_total_count)
    """
    print()
    print(f"\tCount {folder_name} from {start_date} to {end_date}")

    # Heuristic: Only attempt detailed, file-level parsing for non-beta/non-archive folders
    is_detailed_stats_folder = not any(key in folder_name.lower() for key in ['beta/', 'archive'])
    
    mrv2_downloads = 0
    vmrv2_downloads = 0

    if is_detailed_stats_folder:
        # --- 1. DETAILED FILE-LEVEL COUNTING (For main tag releases) ---
        
        detailed_stats = {} # Structure: { (package_prefix, os_spec): count }
        version = folder_name.split('/')[-1] if '/' in folder_name else folder_name
        
        packages = ['mrv2', 'vmrv2']
        # Define known file patterns (OS-ARCH.EXT)
        file_patterns = [
            f"-Windows-amd64.exe", f"-Windows-amd64.zip",
            f"-Windows-aarch64.exe", f"-Windows-aarch64.zip",
            f"-Darwin-amd64.dmg", f"-Darwin-arm64.dmg",
            f"-Linux-amd64.deb", f"-Linux-amd64.rpm", f"-Linux-amd64.tar.gz",
            f"-Linux-aarch64.deb", f"-Linux-aarch64.rpm", f"-Linux-aarch64.tar.gz",
        ]
        
        for package in packages:
            for pattern in file_patterns:
                file_name = f"{package}-{version}{pattern}"
                file_path = f"{folder_name}/{file_name}"
                
                oses_stats = get_file_stats(repo, file_path, start_date, end_date)
                
                if oses_stats:
                    for _, count_str in oses_stats:
                        try:
                            count = int(count_str)
                            
                            if package == 'mrv2':
                                mrv2_downloads += count
                            elif package == 'vmrv2':
                                vmrv2_downloads += count
                                
                            # Use regex to extract OS and architecture for grouping
                            match = re.search(r'-(Windows|Darwin|Linux)-([a-zA-Z0-9]+)', pattern)
                            
                            if match:
                                os_type = match.group(1)
                                arch = match.group(2)
                                
                                if os_type == 'Darwin':
                                    arch_spec = 'Intel' if arch == 'amd64' else 'M1+' if arch == 'arm64' else arch
                                    os_spec = f"Darwin {arch} ({arch_spec})" 
                                else:
                                    os_spec = f"{os_type} {arch}"
                                    
                                key = (package, os_spec)
                                detailed_stats[key] = detailed_stats.get(key, 0) + count
                            
                        except ValueError:
                            continue
        
        # Print detailed stats if any were found
        if detailed_stats:
            sorted_keys = sorted(detailed_stats.keys(), key=lambda x: (x[1], x[0])) # Sort by OS/Arch, then package
            os_group_totals = {}
            for package, os_spec in sorted_keys:
                 count = detailed_stats[(package, os_spec)]
                 os_group_totals[os_spec] = os_group_totals.get(os_spec, 0) + count

            for os_spec, os_total in os_group_totals.items():
                print('{:>5}  OS: {}'.format(os_total, os_spec))
                for package in packages:
                    key = (package, os_spec)
                    count = detailed_stats.get(key, 0)
                    if count > 0:
                        print('        - File: {:<5} {:>5}'.format(package, count))

            total_downloads = mrv2_downloads + vmrv2_downloads
            formatted_total = format_number(total_downloads, 5)
            print('{:>5} Total Downloads for SourceForge {}'.
                  format(formatted_total, f'{repo}/{folder_name} (File Breakdown)'))
            
            # Return separate counts
            return mrv2_downloads, vmrv2_downloads

    # --- 2. AGGREGATED FOLDER STATS (For beta/archive folders) ---
    
    base_url = f"https://sourceforge.net/projects/{repo}/files/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}"
    archive_url = f"https://sourceforge.net/projects/{repo}/files/archive/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}"
    
    r = None
    try:
        response = requests.get(base_url)
        response.raise_for_status()
        r = response.json()
    except requests.exceptions.HTTPError:
        try:
            response = requests.get(archive_url)
            response.raise_for_status()
            r = response.json()
        except requests.exceptions.HTTPError as e:
            print(f'Could not get aggregated info for sfolder {folder_name} for either path. Error: {e}')
            return 0, 0 # Return 0, 0 on failure
        except Exception as e:
            print(f'General error getting aggregated info for folder {folder_name}: {e}')
            return 0, 0 # Return 0, 0 on failure

    if r and 'oses' in r:
        prefix = ''
        if 'opengl' in folder_name.lower():
            prefix = 'mrv2'
        elif 'vulkan' in folder_name.lower():
            prefix = 'vmrv2'
            
        for item in r['oses']:
            try:
                num = int(item[1])
                
                # Assign the aggregated count based on the folder name heuristic
                if prefix == 'mrv2':
                    mrv2_downloads += num
                elif prefix == 'vmrv2':
                    vmrv2_downloads += num
                
                # The output format for betas (generic OS name)
                print('{:>5}  {:<5} OS: {:<40}'.format(num, prefix, item[0]))
            except (ValueError, IndexError):
                print(f"Warning: Could not parse SourceForge data item: {item}")
                continue

    total_downloads = mrv2_downloads + vmrv2_downloads
    formatted_total = format_number(total_downloads, 5)
    print('{:>5} Total Downloads for SourceForge {}'.
          format(formatted_total, f'{repo}/{folder_name} (Aggregated)'))

    # Return separate counts
    return mrv2_downloads, vmrv2_downloads

# --- Main Execution ---
if __name__ == "__main__":
    args = get_date_arguments()

    # The function now handles the date flipping
    start_date_str, end_date_str, user, repo, tag = parse_and_process_dates(args)

    # 1. Get GitHub Totals
    mrv2_github_total, vmrv2_github_total = get_github_downloads(user, repo, tag)
    mrv2_grand_total += mrv2_github_total
    vmrv2_grand_total += vmrv2_github_total

    # 2. Get GitHub totals
    mrv2_github_betas_total, vmrv2_github_betas_total = get_github_downloads(user, repo, "betas")
    print("mrv2_github_betas_total=", mrv2_github_betas_total)
    print("vmrv2_github_betas_total=", vmrv2_github_betas_total)
    mrv2_grand_total += mrv2_github_betas_total
    vmrv2_grand_total += vmrv2_github_betas_total

    if not repo or not args.tag:
        print("\nSkipping SourceForge downloads: Both repository (SourceForge project name) and tag (folder name) must be provided.")
        sys.exit(0)

    print(f"\n--- SourceForge Downloads for Project: {repo} ---")

    # 3. Get SourceForge Released Totals
    mrv2_sf_released_total, vmrv2_sf_released_total = count_sourceforge(repo, args.tag, end_date_str, start_date_str)
    mrv2_grand_total += mrv2_sf_released_total
    vmrv2_grand_total += vmrv2_sf_released_total

    # 4. Get SourceForge Beta OpenGL Totals
    mrv2_sf_beta_opengl_total, vmrv2_sf_beta_opengl_total = count_sourceforge(repo, 'beta/opengl', end_date_str, start_date_str)
    mrv2_grand_total += mrv2_sf_beta_opengl_total
    vmrv2_grand_total += vmrv2_sf_beta_opengl_total

    # 5. Get SourceForge Beta Vulkan Totals
    mrv2_sf_beta_vulkan_total, vmrv2_sf_beta_vulkan_total = count_sourceforge(repo, 'beta/vulkan', end_date_str, start_date_str)
    mrv2_grand_total += mrv2_sf_beta_vulkan_total
    vmrv2_grand_total += vmrv2_sf_beta_vulkan_total

    final_grand_total = mrv2_grand_total + vmrv2_grand_total
    
    print("\n=======================================================================")
    
    # Print separate totals
    formatted_mrv2_grand_total = format_number(mrv2_grand_total, 5)
    formatted_vmrv2_grand_total = format_number(vmrv2_grand_total, 5)
    formatted_final_grand_total = format_number(final_grand_total, 5)

    print(f'{formatted_mrv2_grand_total} Grand Total mrv2 Downloads (GitHub + SourceForge)')
    print(f'{formatted_vmrv2_grand_total} Grand Total vmrv2 Downloads (GitHub + SourceForge)')
    
    print("-----------------------------------------------------------------------")
    print(f'{formatted_final_grand_total} Grand Total (GitHub + SourceForge)')
    
