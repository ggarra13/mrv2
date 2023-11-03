#!/usr/bin/env python

import importlib, os, platform, re, tempfile, subprocess

#
# mrv2 imports
#
from mrv2 import cmd
from fltk14 import *

#
# Non-standard imports
#
module_name = "requests"

try:
    # Try to import the module
    importlib.import_module(module_name)
    print(f"{module_name} is already installed.")
except ImportError:
    # If the module is not found, install it using pip
    print(f"{module_name} is not installed. Installing it now...")
    if platform.system() == 'Windows':
        python_exec = 'python.exe'
    else:
        python_exec = 'python.sh'
    path = cmd.rootPath()
    path = os.path.join(path, 'bin')
    python_exec = os.path.join(path, python_exec)
    print(python_exec)
    subprocess.call([python_exec, "-m", "pip", "install", module_name])


import requests


def run_as_admin(command, download_file, password = None):
    cmd = None
    if platform.system() == 'Windows':
        cmd = r'Powershell -Command Start-Process "' + command + '" -Verb RunAs'
    elif platform.system() == 'Linux':
        cmd = f'echo "{password}" | sudo -S {command}'
    elif platform.system() == 'Darwin':
        cmd = command
    else:
        print("Unknown platform")
        return
    
    ret = os.system(cmd)
    if ret == 0:
        print("The new version of mrv2 was installed.")
        if os.path.exists(download_file):
            os.remove(download_file)
        print(f"Removed temporary {download_file}.")
    else:
        print("Something failed installing mrv2")

def set_password(widget, args):
    command = args[0]
    download_file = args[1]
    password = widget.value()
    widget.parent().hide()
    run_as_admin(command, download_file, password)
    
def ask_for_password(command, download_file):
    win = Fl_Window(320, 100, "Enter Sudo Password")
    pwd = Fl_Secret_Input(20, 40, win.w() - 40, 40, "Password")
    pwd.textcolor(fl_rgb_color(0, 0, 0 ))
    pwd.align(FL_ALIGN_TOP)
    pwd.when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED)
    pwd.callback(set_password, [command, download_file])
    win.end()
    win.set_non_modal()
    win.show()
    while win.visible():
        Fl.check()

def install_download(download_file):
    if download_file.endswith('.exe'):
        command = download_file
        run_as_admin(command, download_file)
    elif download_file.endswith('.rpm'):
        command = f'rpm -i {download_file}'
        ask_for_password(command, download_file)
    elif download_file.endswith('.deb'):
        command = f'dpkg -i {download_file}'
        ask_for_password(command, download_file)
    elif download_file.endswith('.dmg'):
        command = f'open {download_file}'
        run_as_admin(command, download_file)
    else:
        print(f'You will need to install file "{download_file}" manually.')
        return
    
              
def check_linux_flavor():
    if os.system('which dpkg > /dev/null') == 0:
        return '.deb'
    elif os.system('which rpm > /dev/null') == 0:
        return '.rpm'
    elif os.system('which pacman > /dev/null') == 0:
        return '.tar.gz'
    else:
        print('Unable to determine Linux flavor')
        return '.tar.gz'

def get_download_extension():
    if platform.system() == 'Windows':
        return '.exe'
    elif platform.system() == 'Linux':
        return check_linux_flavor()
    elif platform.system() == 'Darwin':
        return '.dmg'
    else:
        return 'Unknown operating system'

def download_version(name, download_url):
    download_file = os.path.join(tempfile.gettempdir(), name)
    print(f"Downloading: {name} ...")
    download_response = requests.get(download_url)
    with open(download_file, 'wb') as f:
        f.write(download_response.content)
        print(f"Download complete: {download_file}")
    if os.path.exists(download_file):
        install_download(download_file)

def match_version(s):
    match = re.search(r'v(\d+\.\d+\.\d+)', s)
    if match:
        return match.group(1)
    else:
        return "No match found"

def get_latest_release(widget, data):
    extension = get_download_extension()
    found = False
    for asset in data['assets']:
        name = asset['name']
        if name.endswith(extension):
            download_version(name, asset['browser_download_url'])
            found = True
            break
    if not found:
        print(f"No file matching {extension} was found")
    widget.parent().hide()
            
def ask_to_upgrade(current_version, version, data):
    win = Fl_Window(320, 200)
    box = Fl_Box(20, 20, win.w() - 40, 60)
    box.copy_label(f"Current version is v{current_version},\n"
                   f"latest is v{version}")
    btn = Fl_Button(20, 100, win.w() - 40, 40, "Upgrade")
    btn.callback(get_latest_release, data)
    win.end()
    win.set_non_modal()
    win.show()
    while win.visible():
        Fl.check()
    
def check_latest_release(user, project):
    url = f"https://api.github.com/repos/{user}/{project}/releases/latest"
    response = requests.get(url)
    data = response.json()
    if 'assets' in data:
        
        release = data['name']
        
        current_version = cmd.getVersion()
        version = match_version(release)

        if current_version == version:
            print(f"Already in the latest version")
            return

        ask_to_upgrade(current_version, version, data)
    else:
        print("No release files found.")

check_latest_release("ggarra13", "mrv2")
