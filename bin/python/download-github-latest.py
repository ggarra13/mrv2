import requests, os, platform, re, tempfile, subprocess
from fltk14 import *

password = None

def run_as_admin(command):
    cmd = ''
    if platform.system() == 'Windows':
        cmd = r'Powershell -Command Start-Process "' + command + '" -Verb RunAs'
    elif platform.system() == 'Linux':
        user = os.getlogin()
        cmd = f'echo "{password}" | sudo {user} -S "{command}"'
    elif platform.system() == 'Darwin':
        cmd = command
    else:
        print("Unknown platform")
        return
    
    print(cmd)
    os.system(cmd)

def set_password(widget, pwd):
    password = pwd.value()
    print(f'Password: {password}')
    widget.parent().hide()
    
def ask_for_password():
    win = Fl_Window(320, 200)
    pwd = Fl_Secret_Input(20, 40, win.w() - 40, 40, "Password")
    pwd.align(FL_ALIGN_TOP)
    btn = Fl_Button(20, 100, win.w() - 40, 40, "Install")
    btn.callback(set_password, pwd)
    win.end()
    win.set_non_modal()
    win.show()
    return Fl.run()

def install_download(download_file):
    command = ''
    if download_file.endswith('.exe'):
        command = download_file
    elif download_file.endswith('.rpm'):
        ask_for_password()
        command = f'rpm -i {download_file}'
    elif download_file.endswith('.deb'):
        ask_for_password()
        command = f'dpkg -i {download_file}'
    elif download_file.endswith('.dmg'):
        command = f'open {download_file}'
    else:
        print(f'You will need to install file "{download_file}" manually.')
        return
    
    run_as_admin(command)

              
def check_linux_flavor():
    if os.system('which dpkg > /dev/null') == 0:
        return '.deb'
    elif os.system('which rpm > /dev/null') == 0:
        return '.rpm'
    elif os.system('which pacman > /dev/null') == 0:
        return '.tar.gz'
    else:
        return 'Unable to determine Linux flavor'

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
    # print(f"Downloading: {name} ...")
    # download_response = requests.get(download_url)
    # with open(download_file, 'wb') as f:
    #    f.write(download_response.content)
    #    print(f"Download complete: {download_file}")
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
    return Fl.run()
    
def check_latest_release(user, project):
    url = f"https://api.github.com/repos/{user}/{project}/releases/latest"
    response = requests.get(url)
    data = response.json()
    if 'assets' in data:
        
        release = data['name']
        
        current_version = '0.8.2'
        version = match_version(release)

        if current_version == version:
            print(f"Already in the latest version")
            return

        ask_to_upgrade(current_version, version, data)
    else:
        print("No release files found.")

check_latest_release("ggarra13", "mrv2")
