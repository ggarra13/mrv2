#!/usr/bin/env python

#
# Standard libs
#
import importlib, os, platform, re, tempfile, subprocess, sys, requests

#
# mrv2 imports
#
from mrv2 import cmd
from fltk14 import *


def match_version(s):
    """Match a version in a string like v0.8.3.

    Args:
        s (str): String with a v0.0.0 literal.
    
    Returns:
        str: The numbers like 0.8.3 without the 'v'.
    """
    match = re.search(r'v(\d+\.\d+\.\d+)', s)
    if match:
        return match.group(1)
    else:
        return "No match found"

    
def start_new_mrv2(download_file, os):
    """Given a download_file and a platform, create the path to the new
       executable.
    """
    version = match_version(download_file)
    exe = ''
    if os == 'Linux':
        exe = f'/usr/local/mrv2-v{version}-Linux-64/bin/mrv2.sh'
    elif os == 'Windows':
        exe = f'C:\Program Files\mrv2 {version}\bin\mrv2.exe'
    elif os == 'Darwin':
        exe = f'/Applications/mrv2.app/Contents/mrv2'
    else:
        raise 'Unknown platform "{os}"'

    try:
        print(f'Starting "{exe}"...')
        # os.execv(exe, [exe])
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return

    
def run_as_admin(command, download_file, password = None):
    """Given a download file, use the extension name to try to install it.
    If required, pop a requester to ask for a password.
    Once the file is successfully installed, it removes the temporary
    downloaded file.

    Args:
        command (str): Command to run to install the downloaded file
        download_file (str): Full path to the downloaded file.
        password (str): Optional password for those platforms that require it.

    Returns:
        None
    """
    cmd = None
    os = platform.system()
    if os == 'Windows':
        cmd = r'Powershell -Command Start-Process "' + command + '" -Verb RunAs'
    elif os == 'Linux':
        if password != None:
            cmd = f'echo "{password}" | sudo -S {command}'
        else:
            cmd = command
    elif os == 'Darwin':
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
        start_new_mrv2(download_file, os)
    else:
        print("Something failed installing mrv2")

def get_password_cb(widget, args):
    """FLTK callback to get the secret password, hide the parent window and
       run the command to install the downloaded file with the sudo password.
    
    Args:
        widget (Fl_Widget): FLTK's secret input widget.
        args (list): [command, download_file]

    """
    command = args[0]
    download_file = args[1]
    password = widget.value()
    widget.parent().hide()
    run_as_admin(command, download_file, password)
    
def ask_for_password(command, download_file):
    """Open a FLTK window and secret input to enter a password for sudo access.

    Args:
        command (str): command to run to install the downloaded file.
        download_file (str): Full path to the file downloaded.

    Returns:
        None
    """
    win = Fl_Window(320, 100, "Enter Sudo Password")
    pwd = Fl_Secret_Input(20, 40, win.w() - 40, 40, "Password")
    pwd.textcolor(fl_rgb_color(0, 0, 0 ))
    pwd.align(FL_ALIGN_TOP)
    pwd.when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED)
    pwd.callback(get_password_cb, [command, download_file])
    win.end()
    win.set_non_modal()
    win.show()
    while win.visible():
        Fl.check()

        
def install_download(download_file):
    """Given a download file, use the extension name to try to install it.
    If required, pop a requester to ask for a password.

    Args:
        download_file (str): Full path to the file downloaded.

    Returns:
        None
    """
    if download_file.endswith('.exe'):
        command = download_file
        run_as_admin(command, download_file)
    elif download_file.endswith('.rpm'):
        command = f'rpm -i {download_file}'
        ask_for_password(command, download_file)
    elif download_file.endswith('.deb'):
        command = f'dpkg -i {download_file}'
        ask_for_password(command, download_file)
    elif download_file.endswith('.tar.gz'):
        command = f'tar -xzvf {download_file} -C ~/'
        run_as_admin(command, download_file)
    elif download_file.endswith('.dmg'):
        command = f'open {download_file}'
        run_as_admin(command, download_file)
    else:
        print(f'You will need to install file "{download_file}" manually.')
        return
    
              
def check_linux_flavor():
    """Given a Linux distribution, try to determine what package manager to
       use and return the package manager's extension to it.

    Returns:
        str: The extension for the Linux flavour.
    """
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
    """Based on platform, return the extension for the file
       to download.

    Returns:
        str: The extension of the platform or 'Unknown operating system'
    """
    os = platform.system()
    if os == 'Windows':
        return '.exe'
    elif os == 'Linux':
        return check_linux_flavor()
    elif os == 'Darwin':
        return '.dmg'
    else:
        return 'Unknown operating system'

    
def download_version(name, download_url):
    """Download a filename from a download url from github.
       If successful, tries to install it.

    Args:
        name (str): name of the file to download.
        download_url (str): URL for downloading the file.
    
    Returns:
        None
    """
    download_file = os.path.join(tempfile.gettempdir(), name)
    print(f"Downloading: {name} ...")
    download_response = requests.get(download_url)
    with open(download_file, 'wb') as f:
        f.write(download_response.content)
        print(f"Download complete: {download_file}")
    if os.path.exists(download_file):
        install_download(download_file)

        
def get_latest_release_cb(widget, data):
    """FLTK callback to start the download of the latest release for
    the current platform.  Hides the widget's parent window when done.

    Args:
        widget (Fl_Widget): FLTK widget that triggered the callback
        data (map): github data for the latest release in a map.

    Returns:
        None
    """
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

    
def ask_to_upgrade(current_version, latest_version, data):
    """Open an FLTK window to allow the user to upgrade.

    Args:
        current_version (str): The current version.
        latest_version (str): The latest version to upgrade.
        data (map): github data for the latest release in a map.

    Returns:
        None
    """
    win = Fl_Window(320, 200)
    box = Fl_Box(20, 20, win.w() - 40, 60)
    box.copy_label(f"Current version is v{current_version},\n"
                   f"latest is v{latest_version}")
    btn = Fl_Button(20, 100, win.w() - 40, 40, "Upgrade")
    btn.callback(get_latest_release_cb, data)
    win.end()
    win.set_non_modal()
    win.show()
    while win.visible():
        Fl.check()


def compare_versions(version1, version2):
    """Compares two versions like version.revision.patch

    Args:
        version1 (str): The first version to compare.
        version2 (str): The second version to compare.

    Returns:
        int:  0 if they are equal.
             -1 if version1 is older. 
              1 if version1 is newer.
    """
    v1_components = list(map(int, version1.split('.')) if version1 else [])
    v2_components = list(map(int, version2.split('.')) if version2 else [])

    for v1, v2 in zip(v1_components, v2_components):
        if v1 < v2:
            return -1  # version1 is older
        elif v1 > v2:
            return 1  # version1 is newer

    if len(v1_components) < len(v2_components):
        return -1  # version1 is shorter, hence older
    elif len(v1_components) > len(v2_components):
        return 1  # version1 is longer, hence newer

    return 0  # versions are identical


def check_latest_release(user, project):
    """Checks for the latest github release for a user and project.

    Args:
        user (str): The github user.
        project (str): The github project.

    Returns:
        None
    """
    url = f"https://api.github.com/repos/{user}/{project}/releases/latest"
    response = requests.get(url)
    data = response.json()
    if 'assets' in data:
        
        release = data['name']
        
        current_version = cmd.getVersion()
        version = match_version(release)

        result = compare_versions(current_version, version)
        if result == 0:
            print(f"Already in the latest version of mrv2 (v{version})")
            return
        elif result == 1:
            print(f"Version v{current_version} is newer than v{version}")
            return
        else:
            print(f"Version v{current_version} is older than v{version}")       
            ask_to_upgrade(current_version, version, data)
            
    else:
        print("No release files found.")


check_latest_release("ggarra13", "mrv2")
