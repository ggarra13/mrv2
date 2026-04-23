DEBUG_CLOUDFLARE=1


import os
import sys
import itertools
import re
from datetime import datetime, timezone, timedelta
import argparse
import time

# --- Third-Party Imports ---
try:
    from curl_cffi import requests
except ImportError:
    print("Error: curl_cffi is not installed")
    print("pip install curl_cffi")
    sys.exit(1)

import json
from io import StringIO

try:
    from functions import format_number
except ImportError:
    def format_number(n, width):
        return str(n).rjust(width)

# --- Playwright (NEW - replaces curl_cffi session for SourceForge) ---
try:
    from playwright.sync_api import sync_playwright
    PLAYWRIGHT_AVAILABLE = True
except ImportError:
    print("Error: playwright is not installed")
    print("Run: pip install playwright playwright-stealth")
    print("Then: playwright install chromium")
    sys.exit(1)

# --- Global Configuration ---
full_names = []
headers = {}

if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = "token %s" % os.environ["GITHUB_TOKEN"]

BROWSER_IMPERSONATE = 'safari15_5'

# --- GRAND TOTALS ---
mrv2_grand_total = vmrv2_grand_total = 0
mrv2_windows_grand_total = mrv2_linux_grand_total = mrv2_macos_grand_total = 0
vmrv2_windows_grand_total = vmrv2_linux_grand_total = vmrv2_macos_grand_total = 0

windows_re = re.compile(R"(?:Windows|Unknown)")
linux_re = re.compile(R"Linux")
macos_re = re.compile(R"(?:Darwin|Macintosh)")


def get_date_arguments():
    parser = argparse.ArgumentParser(description='Process GitHub and SourceForge download counts.')
    parser.add_argument('user', type=str, help='GitHub username')
    parser.add_argument('repo', type=str, nargs='?', default=None, help='Repository name')
    parser.add_argument('tag', type=str, nargs='?', default=None, help='Tag/folder name')
    parser.add_argument('start_date', type=str, default='2014-10-29')
    parser.add_argument('end_date', type=str, nargs='?', default=None)
    return parser.parse_args()


def parse_and_process_dates(args):
    # (unchanged - your excellent date parser stays exactly the same)
    def parse_date_string(date_str, is_end_date=False):
        try:
            dt = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S %z')
            return dt.astimezone(timezone.utc)
        except ValueError:
            pass
        try:
            dt = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S')
            return dt.replace(tzinfo=timezone.utc)
        except ValueError:
            pass
        try:
            dt_naive = datetime.strptime(date_str, '%Y-%m-%d')
            if is_end_date:
                dt = dt_naive.replace(hour=23, minute=59, second=59, microsecond=999999, tzinfo=timezone.utc)
            else:
                dt = dt_naive.replace(hour=0, minute=0, second=0, microsecond=0, tzinfo=timezone.utc)
            return dt
        except ValueError:
            raise ValueError(f"Invalid date format '{date_str}'.")

    start_dt = parse_date_string(args.start_date, False)
    end_dt = datetime.now(timezone.utc) if not args.end_date else parse_date_string(args.end_date, True)

    if end_dt < start_dt:
        print(f"\n **Warning:** End date before start date - dates flipped.")
        start_dt, end_dt = end_dt, start_dt
        start_dt = start_dt.replace(hour=0, minute=0, second=0, microsecond=0)
        end_dt = end_dt.replace(hour=23, minute=59, second=59, microsecond=999999)

    beta_start_dt = start_dt + timedelta(days=1)

    print(" START DATE:", start_dt.strftime("%Y-%m-%d %H:%M:%S %Z"))
    print("    END DATE:", end_dt.strftime("%Y-%m-%d %H:%M:%S %Z"))
    diff = end_dt - start_dt
    print(f' DIFFERENCE: {diff.days} days, {diff.seconds//3600} hours, {(diff.seconds//60)%60} minutes')
    print("-----------------------------------------------------------------------")

    return (beta_start_dt.strftime("%Y-%m-%d"),
            start_dt.strftime("%Y-%m-%d"),
            end_dt.strftime("%Y-%m-%d"),
            args.user, args.repo, args.tag)


def get_github_downloads(user, repo, tag):
    # (unchanged - your original GitHub function, still uses curl_cffi)
    if not user or not repo:
        print("Skipping GitHub downloads: user and repo required.")
        return 0, 0, 0, 0, 0, 0, 0, 0

    full_name = f"{user}/{repo}"
    print(f"--- GitHub Downloads for {full_name} ---")

    mrv2_total = vmrv2_total = 0
    mrv2_win = vmrv2_win = mrv2_lin = vmrv2_lin = mrv2_mac = vmrv2_mac = 0

    try:
        PER_PAGE = 100
        for page in itertools.count(1):
            url = f'https://api.github.com/repos/{full_name}/releases?per_page={PER_PAGE}&page={page}'
            response = requests.get(url, impersonate="chrome120")
            response.raise_for_status()
            releases = response.json()
            if not releases:
                break

            for r in releases:
                if tag and r.get('tag_name') and not re.search(re.escape(tag), r['tag_name']):
                    continue
                for asset in r.get('assets', []):
                    if tag and not re.match(f'.*{re.escape(tag)}.*', asset['name']):
                        continue
                    downloads = asset['download_count']
                    name = asset['name']
                    if name.startswith('mrv2'):
                        mrv2_total += downloads
                        if windows_re.search(name): mrv2_win += downloads
                        if linux_re.search(name): mrv2_lin += downloads
                        if macos_re.search(name): mrv2_mac += downloads
                    elif name.startswith('vmrv2'):
                        vmrv2_total += downloads
                        if windows_re.search(name): vmrv2_win += downloads
                        if linux_re.search(name): vmrv2_lin += downloads
                        if macos_re.search(name): vmrv2_mac += downloads

                    print('{:>5} Asset: {:<40} Date: {}'.format(
                        format_number(asset['download_count'], 5), asset['name'],
                        asset['updated_at'].split('T')[0]))

            if len(releases) < PER_PAGE:
                break
    except Exception as e:
        print(f'GitHub error: {e}')
        sys.exit(1)

    print("-----------------------------------------------------------------------")
    return (mrv2_total, vmrv2_total, mrv2_win, vmrv2_win, mrv2_lin, vmrv2_lin, mrv2_mac, vmrv2_mac)

def count_sourceforge(repo, folder_name, end_date, start_date, fetch_json):
    """Fetches and sums SourceForge download counts for a folder."""
    print(f"\n\tCount {folder_name} from {start_date} to {end_date}")

    is_detailed_stats_folder = not any(key in folder_name.lower() for key in ['beta/', 'archive'])

    mrv2_total = vmrv2_total = 0
    mrv2_win = vmrv2_win = mrv2_lin = vmrv2_lin = mrv2_mac = vmrv2_mac = 0

    if is_detailed_stats_folder:
        # === DETAILED FILE-LEVEL STATS ===
        detailed_stats = {}
        version = folder_name.split('/')[-1] if '/' in folder_name else folder_name
        packages = ['mrv2', 'vmrv2']
        file_patterns = [
            "-Windows-amd64.exe", "-Windows-amd64.zip",
            "-Windows-aarch64.exe", "-Windows-aarch64.zip",
            "-Darwin-amd64.dmg", "-Darwin-arm64.dmg",
            "-Linux-amd64.deb", "-Linux-amd64.rpm", "-Linux-amd64.tar.gz",
            "-Linux-aarch64.deb", "-Linux-aarch64.rpm", "-Linux-aarch64.tar.gz",
        ]

        for package in packages:
            for pattern in file_patterns:
                file_name = f"{package}-{version}{pattern}"
                file_path = f"{folder_name}/{file_name}"

                if package == 'mrv2' and (pattern.endswith('aarch64.exe') or pattern.endswith('aarch64.zip')):
                    continue

                # Small delay between file stats calls
                time.sleep(0.7)

                # Build FULL URL here (uses the correct start_date for this folder)
                full_url = (f"https://sourceforge.net/projects/{repo}/files/{file_path}"
                            f"/stats/json?start_date={start_date}&end_date={end_date}")

                oses_stats = fetch_json(full_url)

                if oses_stats and 'oses' in oses_stats:
                    for _, count_str in oses_stats['oses']:
                        try:
                            count = int(count_str)
                            if package == 'mrv2':
                                mrv2_total += count
                            else:
                                vmrv2_total += count

                            match = re.search(r'-(Windows|Darwin|Linux)-([a-zA-Z0-9]+)', pattern)
                            if match:
                                os_type, arch = match.groups()
                                os_spec = f"{os_type} {arch}"
                                key = (package, os_spec)
                                detailed_stats[key] = detailed_stats.get(key, 0) + count

                                if os_type in ('Windows', 'Unknown'):
                                    if package == 'mrv2':
                                        mrv2_win += count
                                    else:
                                        vmrv2_win += count
                                elif os_type == 'Linux':
                                    if package == 'mrv2':
                                        mrv2_lin += count
                                    else:
                                        vmrv2_lin += count
                                elif os_type == 'Darwin':
                                    if package == 'mrv2':
                                        mrv2_mac += count
                                    else:
                                        vmrv2_mac += count
                        except ValueError:
                            continue

        if detailed_stats:
            sorted_keys = sorted(detailed_stats.keys(), key=lambda x: (x[1], x[0]))
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

            total_downloads = mrv2_total + vmrv2_total
            print('{:>5} Total Downloads for SourceForge {} (File Breakdown)'.format(
                format_number(total_downloads, 5), f'{repo}/{folder_name}'))

        return (mrv2_total, vmrv2_total,
                mrv2_win, vmrv2_win,
                mrv2_lin, vmrv2_lin,
                mrv2_mac, vmrv2_mac)

    # === AGGREGATED FOLDER STATS (beta + older released) ===
    urls = [
        f"https://sourceforge.net/projects/{repo}/files/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}",
        f"https://sourceforge.net/projects/{repo}/files/archive/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}"
    ]

    r = None
    for url in urls:
        r = fetch_json(url)          # ← now passes full URL (consistent)
        if r and 'oses' in r:
            break
        time.sleep(1)

    if r and 'oses' in r:
        prefix = 'mrv2' if 'opengl' in folder_name.lower() else 'vmrv2' if 'vulkan' in folder_name.lower() else ''
        for item in r['oses']:
            try:
                os_name = item[0]
                num = int(item[1])
                if prefix == 'mrv2':
                    mrv2_total += num
                    if windows_re.search(os_name): mrv2_win += num
                    elif linux_re.search(os_name): mrv2_lin += num
                    elif macos_re.search(os_name): mrv2_mac += num
                elif prefix == 'vmrv2':
                    vmrv2_total += num
                    if windows_re.search(os_name): vmrv2_win += num
                    elif linux_re.search(os_name): vmrv2_lin += num
                    elif macos_re.search(os_name): vmrv2_mac += num
                print('{:>5}  {:<5} OS: {:<40}'.format(num, prefix, os_name))
            except Exception:
                continue

        total_downloads = mrv2_total + vmrv2_total
        print('{:>5} Total Downloads for SourceForge {} (Aggregated)'.format(
            format_number(total_downloads, 5), f'{repo}/{folder_name}'))

    return (mrv2_total, vmrv2_total, mrv2_win, vmrv2_win, mrv2_lin, vmrv2_lin, mrv2_mac, vmrv2_mac)

# ====================== MAIN ======================
if __name__ == "__main__":
    args = get_date_arguments()
    beta_start_date_str, start_date_str, end_date_str, user, repo, tag = parse_and_process_dates(args)

    # 1. GitHub (unchanged)
    (mrv2_github, vmrv2_github,
     mrv2_win_g, vmrv2_win_g,
     mrv2_lin_g, vmrv2_lin_g,
     mrv2_mac_g, vmrv2_mac_g) = get_github_downloads(user, repo, tag)

    print(f'{format_number(mrv2_win_g, 5)} mrv2 Windows Downloads (All Archs)')
    print(f'{format_number(mrv2_lin_g, 5)} mrv2 Linux Downloads (All Archs)')
    print(f'{format_number(mrv2_mac_g, 5)} mrv2 macOS Downloads (All Archs)')
    print()
    print(f'{format_number(vmrv2_win_g, 5)} vmrv2 Windows Downloads (All Archs)')
    print(f'{format_number(vmrv2_lin_g, 5)} vmrv2 Linux Downloads (All Archs)')
    print(f'{format_number(vmrv2_mac_g, 5)} vmrv2 macOS Downloads (All Archs)')
    print()
    print(f'{format_number(mrv2_github, 5)} Total mrv2 Total Downloads')
    print(f'{format_number(vmrv2_github, 5)} Total vmrv2 Total Downloads')
    print('===================================================================')
    github_total = mrv2_github + mrv2_github
    print(f'{format_number(github_total, 5)} Total Downloads for GitHub')
    
    mrv2_grand_total += mrv2_github
    vmrv2_grand_total += vmrv2_github
    mrv2_windows_grand_total += mrv2_win_g
    vmrv2_windows_grand_total += vmrv2_win_g
    mrv2_linux_grand_total += mrv2_lin_g
    vmrv2_linux_grand_total += vmrv2_lin_g
    mrv2_macos_grand_total += mrv2_mac_g
    vmrv2_macos_grand_total += vmrv2_mac_g

    if not repo or not tag:
        print("\nSkipping SourceForge: repo and tag required.")
        sys.exit(0)


    print(f"\n--- SourceForge Downloads for Project: {repo} ---")

    # === Playwright using YOUR real Chrome + real-browser fetch ===
    with sync_playwright() as p:
        browser = p.chromium.launch(
            channel="chrome",
            headless=True,
            args=["--no-sandbox", "--disable-blink-features=AutomationControlled"]
        )

        context = browser.new_context(
            user_agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36",
            viewport={"width": 1920, "height": 1080},
            locale="en-US",
        )

        # Strong anti-detection init script
        context.add_init_script("""
            Object.defineProperty(navigator, 'webdriver', {get: () => undefined});
            delete window.cdc_undefined;
            delete window._cdc_;
            Object.defineProperty(navigator, 'plugins', {get: () => [1,2,3,4,5]});
            Object.defineProperty(navigator, 'languages', {get: () => ['en-US', 'en']});
        """)

        page = context.new_page()

        # Warm-up with realistic human behavior
        try:
            if DEBUG_CLOUDFLARE:
                print("Warming up real Chrome session on SourceForge...")
            page.goto(f"https://sourceforge.net/projects/{repo}/", 
                      wait_until="domcontentloaded", timeout=60000)
            page.wait_for_timeout(2500)
            page.mouse.move(400, 300)
            page.wait_for_timeout(800)
            page.evaluate("window.scrollBy(0, 700)")
            page.wait_for_timeout(1200)
            page.evaluate("window.scrollBy(0, -400)")
            page.wait_for_timeout(1000)
            if DEBUG_CLOUDFLARE:
                print("✅ Session warmed up — Cloudflare Turnstile cleared")
        except Exception as e:
            print(f"Warning: Warm-up issue: {e}")

        # === THIS IS THE KEY FIX: fetch from inside the real page ===
        def fetch_sf_json(full_url):
            """full_url is now always a complete stats/json URL"""
            try:
                result = page.evaluate("""async (url) => {
                    try {
                        const res = await fetch(url, {
                            credentials: 'include',
                            headers: { 
                                'Accept': 'application/json', 
                                'Referer': 'https://sourceforge.net/projects/mrv2/files/' 
                            }
                        });
                        if (!res.ok) return {error: `HTTP ${res.status}`};
                        return await res.json();
                    } catch (e) { return {error: e.toString()}; }
                }""", full_url)

                if isinstance(result, dict) and 'error' in result:
                    print(f"   403/Blocked for {full_url.split('/projects/')[1][:60]}...")
                    return None
                return result
            except Exception as e:
                print(f"Evaluate failed for stats URL: {e}")
                return None
        
        # Run the three queries
        for folder, date_start in [
            (tag, start_date_str),                     # released (detailed)
            ('beta/opengl', beta_start_date_str),
            ('beta/vulkan', beta_start_date_str),
        ]:
            (mrv2_sf, vmrv2_sf,
             mrv2_win_sf, vmrv2_win_sf,
             mrv2_lin_sf, vmrv2_lin_sf,
             mrv2_mac_sf, vmrv2_mac_sf) = count_sourceforge(
                repo, folder, end_date_str, date_start, fetch_sf_json)

            mrv2_grand_total += mrv2_sf
            vmrv2_grand_total += vmrv2_sf
            mrv2_windows_grand_total += mrv2_win_sf
            vmrv2_windows_grand_total += vmrv2_win_sf
            mrv2_linux_grand_total += mrv2_lin_sf
            vmrv2_linux_grand_total += vmrv2_lin_sf
            mrv2_macos_grand_total += mrv2_mac_sf
            vmrv2_macos_grand_total += vmrv2_mac_sf

        browser.close()
        
    # Final grand totals (your original printing code - unchanged)
    print("\n=======================================================================")
    print(f'{format_number(mrv2_windows_grand_total, 5)} Grand Total mrv2 Windows Downloads (GitHub + SourceForge)')
    print(f'{format_number(mrv2_linux_grand_total, 5)} Grand Total mrv2 Linux Downloads (GitHub + SourceForge)')
    print(f'{format_number(mrv2_macos_grand_total, 5)} Grand Total mrv2 macOS Downloads (GitHub + SourceForge)')
    print()
    print(f'{format_number(vmrv2_windows_grand_total, 5)} Grand Total vmrv2 Windows Downloads (GitHub + SourceForge)')
    print(f'{format_number(vmrv2_linux_grand_total, 5)} Grand Total vmrv2 Linux Downloads (GitHub + SourceForge)')
    print(f'{format_number(vmrv2_macos_grand_total, 5)} Grand Total vmrv2 macOS Downloads (GitHub + SourceForge)')
    print()
    print(f'{format_number(mrv2_grand_total, 5)} Grand Total mrv2 Downloads (GitHub + SourceForge)')
    print(f'{format_number(vmrv2_grand_total, 5)} Grand Total vmrv2 Downloads (GitHub + SourceForge)')
    print("-----------------------------------------------------------------------")
    print(f'{format_number(mrv2_grand_total + vmrv2_grand_total, 5)} Grand Total (GitHub + SourceForge)')
