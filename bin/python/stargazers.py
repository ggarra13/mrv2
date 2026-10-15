import requests
import time
from datetime import datetime
import os

# --- Configuration ---
OWNER = "ggarra13"
REPO = "mrv2"
GITHUB_API_URL = f"https://api.github.com/repos/{OWNER}/{REPO}/stargazers"
# Maximum number of results per page (GitHub max is usually 100)
PER_PAGE = 100
# Output file name
OUTPUT_FILE = "stargazers_mrv2.txt"
# Delimiter to separate fields in the text file
DELIMITER = ", " 

# --- IMPORTANT: Authentication ---
# Get token from environment variable or set to None for unauthenticated access (limited)
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN") 

HEADERS = {
    "Accept": "application/vnd.github.v3.star+json",
    "X-GitHub-Api-Version": "2022-11-28"   # or "2026-03-10"
}

if GITHUB_TOKEN:
    HEADERS["Authorization"] = f"Bearer {GITHUB_TOKEN}"
    print("✅ Running with authenticated access (using GITHUB_TOKEN).")
else:
    print("⚠️  No GITHUB_TOKEN found → unauthenticated (very limited rate).")

# --- Function to Fetch All Stargazers ---
def fetch_all_stargazers():
    """Fetches all stargazers for the repository using pagination."""
    stargazers = []
    page = 1
    total_fetched = 0
    
    print(f"Starting to fetch stargazers for {OWNER}/{REPO}...")

    while True:
        params = {
            "page": page,
            "per_page": PER_PAGE
        }
        
        try:
            response = requests.get(GITHUB_API_URL, headers=HEADERS, params=params)
            
            # --- Error and Rate Limit Handling (same as before) ---
            if response.status_code == 403:
                if 'X-RateLimit-Remaining' in response.headers and int(response.headers['X-RateLimit-Remaining']) == 0:
                    reset_timestamp = int(response.headers['X-RateLimit-Reset'])
                    reset_time = datetime.fromtimestamp(reset_timestamp)
                    wait_time = reset_timestamp - time.time()
                    print(f"\nRate limit exceeded. Waiting for {int(wait_time)} seconds until {reset_time.strftime('%H:%M:%S')}")
                    time.sleep(wait_time + 5)
                    continue
                else:
                    print(f"\nError 403: Forbidden (Check your token and permissions). Response: {response.json()}")
                    break
            elif response.status_code != 200:
                print(f"\nError: Received status code {response.status_code}.")
                print(f"Response: {response.json()}")
                if response.status_code == 404:
                    print("→ This usually means the token doesn't have access to this repo.")
                    print("→ Try generating a new fine-grained token with Metadata read permission.")
                break
            
            data = response.json()
            
            if not data:
                print("--- All pages fetched ---")
                break
            
            stargazers.extend(data)
            total_fetched += len(data)

            print(f"Page {page} fetched. Total stargazers collected: {total_fetched}", end='\r', flush=True)

            page += 1
            time.sleep(1) # Be respectful of the API rate limit

        except requests.exceptions.RequestException as e:
            print(f"\nAn error occurred during the request: {e}")
            break
            
    return stargazers

# --- Function to Save Data to TXT ---
def save_to_txt(data):
    """Saves the list of stargazer data to a plain text file."""
    if not data:
        print("\nNo stargazer data to save.")
        return

    # Define the header line
    header = f"login{DELIMITER}id{DELIMITER}html_url{DELIMITER}starred_at\n"
    
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        # Write the header
        f.write(header)
        
        for stargazer_entry in data:
            user = stargazer_entry['user']
            
            # Construct the line: login, id, html_url, starred_at
            line = DELIMITER.join([
                user['login'], 
                str(user['id']), # Ensure ID is converted to string
                user['html_url'], 
                stargazer_entry['starred_at']
            ]) + '\n'
            
            f.write(line)

    print(f"\nSuccessfully saved {len(data)} stargazers to {OUTPUT_FILE}")

# --- Main Execution ---
if __name__ == "__main__":
    stargazer_data = fetch_all_stargazers()
    save_to_txt(stargazer_data)
    
