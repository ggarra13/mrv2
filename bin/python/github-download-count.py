#!/usr/bin/env python

import os
import sys
import itertools
import re
from datetime import date
from functions import *

if (len(sys.argv) < 2):
    print ("Usage: " + sys.argv[0] + " github-user [github-project] [tag]")
    exit(1)

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

user=''
repo=''
full_names = []
headers = {}
tag = ''

if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = "token %s" % os.environ["GITHUB_TOKEN"]

if len(sys.argv) == 4:
    full_names.append(sys.argv[1] + "/" + sys.argv[2])
    user = sys.argv[1]
    repo = sys.argv[2]
    tag  = sys.argv[3]
elif len(sys.argv) == 3:
    full_names.append(sys.argv[1] + "/" + sys.argv[2])
    user = sys.argv[1]
    repo = sys.argv[2]
else:
    buf = StringIO()
    r = requests.get('https://api.github.com/users/' + sys.argv[1] + '/repos', headers=headers)
    repos = r.json()
    user = sys.argv[1]
    
    for repo in repos:
        full_names.insert(0, ensure_str(repo['full_name']))

        
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
                    print('Repo: %s\tCount: %d\tDate: %s\tAsset: %s'%(
                        full_name,
                        asset['download_count'],
                        asset_date,
                        asset['name'],
                    ))
    except Exception as e:
        print('Exception while fetching releases for repo %s: %s') % (full_name,
                                                                      e)
        exit(1)

    formatted_total = format_number(total_count, 5)
    print(f'{formatted_total}\tTotal Downloads for repo {full_name}')
    
today = date.today()
formatted_date = today.strftime("%Y-%m-%d")

folder_name = sys.argv[3]

# Base URL for the project downloads page
base_url = f"https://sourceforge.net/projects/{repo}/files/{folder_name}/stats/json?start_date=2014-10-29&end_date={formatted_date}"

# Send request to download page
response = requests.get(base_url)
try:
    r = response.json()
except Exception as e:
    base_url = f"https://sourceforge.net/projects/{repo}/files/archive/{folder_name}/stats/json?start_date=2014-10-29&end_date={formatted_date}"
    try:
        response = requests.get(base_url)
        r = response.json()
    except Exception as e:
        print(f'Could not get info for version {folder_name}')
        exit(1)
    
total = 0
for item in r['oses']:
    num=int(item[1])
    print(f'\tOS: {item[0]}\tCount: {num}')
    total += num

formatted_total = format_number(total, 5)
print(f'{formatted_total}\tTotal Downloads for sourceforge {repo}')
formatted_total = format_number(total + total_count, 5)
print(f'{formatted_total}\tGrand Total')
