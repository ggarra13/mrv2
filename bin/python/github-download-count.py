import os
import sys
import itertools
import re
from datetime import datetime, timezone, timedelta
import argparse
import csv
from io import StringIO

# --- Third-Party Imports ---
try:
    import requests
except ImportError:
    print("Error: requests is not installed")
    print("Installing Requests is simple with pip:\n  pip install requests")
    print("More info: http://docs.python-requests.org/en/latest/")
    sys.exit(1)

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
headers = {
    "User-Agent": (
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/120.0 Safari/537.36"
    ),
    "Accept": "application/json",
}

if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = "token %s" % os.environ["GITHUB_TOKEN"]



# --- GLOBAL TRACKERS FOR GRAND TOTALS ---
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
    
    # NEW ARGUMENT: For specifying the SourceForge local CSV file path
    parser.add_argument('--sf_csv_path', type=str, default=None, 
                        help='Path to the manually downloaded SourceForge CSV log file.')

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

    return start_date_str, end_date_str, args.user, args.repo, args.tag, args.sf_csv_path

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
            response = requests.get(url, headers=headers, timeout=15)
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
    
    return mrv2_total_count, vmrv2_total_count


# --- Main Execution ---
if __name__ == "__main__":
    args = get_date_arguments()

    # The function now handles the date flipping and parses the new sf_csv_path argument
    start_date_str, end_date_str, user, repo, tag, sf_csv_path = parse_and_process_dates(args)

    # 1. Get GitHub Totals
    mrv2_github_total, vmrv2_github_total = get_github_downloads(user, repo, tag)
    mrv2_grand_total += mrv2_github_total
    vmrv2_grand_total += vmrv2_github_total
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
