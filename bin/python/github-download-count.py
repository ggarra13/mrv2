#!/usr/bin/env python

import os
import sys
import itertools
import re
from datetime import datetime
import pytz
from functions import *


try:
    import requests
except ImportError:
    print ("Error: requests is not installed")
    print ("Installing Requests is simple with pip:\n  pip install requests")
    print ("More info: http://docs.python-requests.org/en/latest/")
    exit(1)
import json
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

def dict_to_object(d):
    if '__class__' in d:
        class_name = d.pop('__class__')
        module_name = d.pop('__module__')
        module = __import__(module_name)
        class_ = getattr(module, class_name)
        args = dict((key.encode('ascii'), value) for key, value in d.items())
        inst = class_(**args)
    else:
        inst = d
    return inst


def ensure_str(s):
    try:
        if isinstance(s, unicode):
            s = s.encode('utf-8')
    except:
        pass
    return s

full_names = []
headers = {}

if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = "token %s" % os.environ["GITHUB_TOKEN"]
import argparse

# Initialize the parser
parser = argparse.ArgumentParser(description='Process GitHub download count.')

# Add positional arguments
parser.add_argument('user', type=str, help='GitHub username')
parser.add_argument('repo', type=str, nargs='?', help='Repository name (optional)')
parser.add_argument('tag', type=str, nargs='?', help='Tag name (optional)')
parser.add_argument('start_date', type=str, default='2014-10-29',
                    help='Start Date (optional)')
parser.add_argument('end_date', type=str, nargs='?', help='End Date (optional)')

# Parse the arguments
args = parser.parse_args()

user = args.user
repo = args.repo
tag  = args.tag
start_date = args.start_date
end_date = args.end_date
folder_name = args.tag

utc = pytz.timezone('UTC')

start_date = datetime.fromisoformat(start_date).astimezone(utc)
    
if not end_date:
    today = datetime.utcnow()
    end_date = today.strftime("%Y-%m-%d").astimezone(utc)
else:
    end_date = datetime.fromisoformat(end_date).astimezone(utc)

time_difference = end_date - start_date
days  = time_difference.days
hours, remainder = divmod(time_difference.seconds, 3600)
minutes, seconds = divmod(remainder, 60)

print(" START DATE:", start_date)
print("   END DATE:", end_date)
print(f'DIFFERENCE: {days} days, {hours} hours, {minutes} minutes')

start_date = start_date.strftime("%Y-%m-%d")
end_date = end_date.strftime("%Y-%m-%d")

# Print out the arguments
# print('      User:', user)
# print('Repository:', repo)
# print('       Tag:', tag)
# print('Start Date:', start_date)
# print('  End Date:', end_date)


full_names = []
if user and repo:
    full_names.append(user + "/" + repo)

asset_date='2014-10-29'

for full_name in full_names:
    buf = StringIO()
    total_count = 0
    try:
        page = 1

        # Max count of results returned by the API
        PER_PAGE = 100
  
        for page in itertools.count(1):
            releases = requests.get('https://api.github.com/repos/' + full_name + '/releases?per_page=100&page=' + str(page), headers=headers).json()

            
            # Page is empty, we got all the results
            if not releases:
                break

            for r in releases:
                if "assets" not in r:
                    print(f'Repo: {full_name}\t{r} No assets')
                    continue
                for asset in r['assets']:
                    if tag:
                        regex = '.*' + tag + '.*'
                        if not re.match(regex, asset['name']):
                            continue
                    total_count += asset['download_count']
                    asset_date = asset['updated_at'].split('T')[0]
                    print('%s\tAsset: %s\tDate: %s'%(
                        format_number(asset['download_count'], 5),
                        asset['name'],
                        asset_date,
                    ))
    except Exception as e:
        print('Exception while fetching releases for repo %s: %s') % (full_name,
                                                                      e)
        exit(1)

    formatted_total = format_number(total_count, 5)
    print(f'{formatted_total}\tTotal Downloads for repo {full_name}')
    

def count_sourceforge(repo, folder_name, end_date, start_date = '2021-10-29'):

    print()
    commercial = ''
    if folder_name == 'vulkan':
        commercial = '(commercial limited demo)'
    print(f"\tCount {folder_name} from {start_date} to {end_date} {commercial}")
    
    # Base URL for the project downloads page
    base_url = f"https://sourceforge.net/projects/{repo}/files/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}"

    # Send request to download page
    response = requests.get(base_url)
    try:
        r = response.json()
    except Exception as e:
        base_url = f"https://sourceforge.net/projects/{repo}/files/archive/{folder_name}/stats/json?start_date={start_date}&end_date={end_date}"
        try:
            response = requests.get(base_url)
            r = response.json()
        except Exception as e:
            print(f'Could not get info for version {folder_name}')
            print(e)
            exit(1)
    
    total = 0
    for item in r['oses']:
        num=int(item[1])
        print(f'\tOS: {item[0]}\tCount: {num}')
        total += num

    formatted_total = format_number(total, 5)
    print(f'{formatted_total}\tTotal Downloads for sourceforge {repo}/{folder_name}')

    return total

sourceforge_released_total = count_sourceforge(repo, folder_name,
                                               end_date)

sourceforge_beta_total = count_sourceforge(repo, 'beta/opengl', end_date, start_date)

sourceforge_beta_total = count_sourceforge(repo, 'beta/vulkan', end_date, start_date)

formatted_total = format_number(sourceforge_released_total +
                                sourceforge_beta_total +
                                total_count, 5)
print('-----------------------------------------------------')
print(f'{formatted_total}\tGrand Total')
