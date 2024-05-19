# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# Current github constants
#
GITHUB_ASSET_NAME = 'name'
GITHUB_ASSET_TAG  = 'tag_name'
GITHUB_ASSET_DATE = 'published_at'
GITHUB_ASSET_URL = 'browser_download_url'

GITHUB_ASSET_RELEASE_DAYS = 5

    
#
# Standard libs
#
import os, platform, re, inspect, subprocess, sys, tempfile, threading, time

#
# mrv2 imports
#
import mrv2
from mrv2 import cmd, plugin, session, settings

try:
    from fltk14 import *
except Exception as e:
    pass

try:
    import gettext
    
    locales = cmd.rootPath() + '/python/plug-ins/locales'

    language = cmd.getLanguage()

    # Set the domain (name for your translations) and directory
    translator = gettext.translation('update-mrv2', localedir=locales,
                                     languages=[language])

    # Mark strings for translation using the _() function
    _ = translator.gettext
except Exception as e:
    print(e)
    def _gettext(text):
        return text
    _ = _gettext

# Global variable to store the result of the subprocess
subprocess_result = None
install_progress  = 0

def run_subprocess(command):
    """
    Run a command trapping stderr and stdout.

    Returns:
        int: Exit code of the process.
    """
    global subprocess_result
    try:
        result = subprocess.run(command, shell=True,
                                check=True, capture_output=True, text=True)
        subprocess_result = result.stdout
    except subprocess.CalledProcessError as e:
        subprocess_result = f"An error occurred: {e.stderr}"

        
def fltk_check_callback(data):
    """FLTK Function to periodically call Fl.check() and check subprocess status

    Args:
        data (list): [self, download_file]

    Returns:
        None

    """

    global subprocess_result, install_progress
    this = data[0]
    download_file = data[1]

    #
    # Linux and Darwin have automatic installers
    #
    kernel = platform.system()
    if kernel != 'Windows':
        print('.', end='', flush=True)
        install_progress += 1
        if install_progress > 40:
            print()
            install_progress = 0
            Fl.check()
    if subprocess_result is None:
        Fl.repeat_timeout(1.0, fltk_check_callback, data)
    else:
        print(f"\n\nInstall completed with output:\n{subprocess_result}")
        
        # On Windows, the installer runs as a background process, but it
        # locks the installer file.  We keep trying to remove it until we
        # can, which means the installer finished
        if kernel == 'Windows':
            time.sleep(10) # Wait for the installer to start
            while os.path.exists(download_file):
                try:
                    os.remove(download_file)
                    print(_('Removed temporary "') + download_file + '".')
                    Fl.check()
                except:
                    time.sleep(2) # Wait 2 seconds before trying again. 
                    pass

        exe = this.get_installed_executable(download_file)
        if os.path.exists(exe):
            print(_('The new version of mrv2 was installed.'))
            Fl.check()
            if os.path.exists(download_file):
                os.remove(download_file)
                print(_('Removed temporary "') + download_file + '".')
                Fl.check()
            this.start_new_mrv2(download_file, kernel)
        else:
            if kernel == 'Windows':
                print(_('Could not locate mrv2 in:\n') + exe + '.\n' +
                      _('Maybe you installed it in a non-default location.'))
                return
            print(_('Something failed installing mrv2 - It is not in "'),
                  exe,'"')

        subprocess_result = None  # Reset for next run
        
def _get_password_cb(widget, args):
    """FLTK callback to get the secret password, hide the parent window and
    run the command to install the downloaded file with the sudo password.
    
    Args:
        widget (Fl_Widget): FLTK's secret input widget.
        args (list): [self, command, download_file]
    """
    this    = args[0]
    command = args[1]
    download_file = args[2]
    password = widget.value()
    widget.parent().hide()
    Fl.check()
    this.install_as_admin(command, download_file, password)


def _ignore_cb(widget, args):
    """FLTK callback to ignore the upgrade and continue with the
    current version.

    Args:
        widget (Fl_Widget): FLTK widget that triggered the callback
        args (list): None

    Returns:
        None
    """
    widget.parent().hide()
    Fl.check()
    

def _more_than_5_days_elapsed(release_date_iso):   
    """Compares the release date with the current date and returns True if
    more than 5 days have elapsed.  This is used for automatic updates to
    leave a buffer in case some critical bugs in a release are found.
        
    Args:
        release_date_iso (str): The release date in ISO 8601 format 
                                (YYYY-MM-DD).

    Returns:
        bool: True if more than 5 days have passed, False otherwise.
    """
    try:
        from datetime import datetime, timedelta
        # Parse the release date string into a datetime object
        release_date = datetime.fromisoformat(release_date_iso)

        # Get today's date
        today = datetime.utcnow().date()

        # Calculate the difference in days between release date and today
        time_elapsed = today - release_date.date()

        # Check if more than 5 days have passed
        return time_elapsed.days > GITHUB_ASSET_RELEASE_DAYS
    except ValueError:
        print(_("Invalid ISO 8601 format provided: "),release_date_iso)
        return False

def _get_latest_release_cb(widget, args):
    """FLTK callback to start the download of the latest release for
    the current platform.  Hides the widget's parent window when done.

    Args:
        widget (Fl_Widget): FLTK widget that triggered the callback
        args (list): [self, release_info for the latest release in a map.]

    Returns:
        None
    """
    this = args[0]
    release_info = args[1]
    extension = this.get_download_extension()
    widget.parent().hide()
    Fl.check()
    found = False
    name = release_info['name']
    if name.endswith(extension):
        this.download_file(name, release_info['download_url'])
    else:
        print(_('No file matching'),extension,_('was found'))


class UpdatePlugin(plugin.Plugin):
    """
    This plugin checks GitHub for the latest release version of mrv2 and
    allows the user to download and install it.  It works on all platforms.
    """

    startup = True
    
    def __init__(self):
        super().__init__()
        self.tempdir = tempfile.gettempdir()
        if settings.checkForUpdates():
            self.check_latest_release("ggarra13", "mrv2")
        UpdatePlugin.startup = False
        
        
    def match_version(self, s):
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
            return _("No match found")
        
    def get_installed_executable(self, download_file):
        """
        Given a download file, return the default location for the
        installed executable.

        Args:
            download_file (str): Download file.
        
        Returns:
            str: Installed executable.
        """
        version = self.match_version(download_file)
        kernel = platform.system()
        exe = None
        if kernel == 'Linux':
            exe = f'/usr/local/mrv2-v{version}-Linux-64/bin/mrv2.sh'
        elif kernel == 'Windows':
            import winreg
            exe = 'C:/Program Files/mrv2/bin/mrv2.exe'
            try:
                key_path = r"Applications\mrv2.exe\shell\Open\command"
                key = winreg.OpenKey(winreg.HKEY_CLASSES_ROOT,
                                     key_path)
                value, reg_type = winreg.QueryValueEx(key, '')
                exe = f'{value[:-5]}'
                exe = exe.replace('\\', '/')
                print(_('Install Location: ') + exe)
            except WindowsError as e:
                print(_('Error retrieving value:\n'),e)
            finally:
                # Always close the opened key
                winreg.CloseKey(key)

            #
            # Look for default install locations
            #
            if not os.path.exists(exe):
                exe = f'"C:/Program Files/mrv2-v{version}/bin/mrv2.exe"'
            if not os.path.exists(exe):
                exe = f'"C:/Program Files/mrv2 v{version}/bin/mrv2.exe"'
            if not os.path.exists(exe):
                exe = f'"C:/Program Files/mrv2 {version}/bin/mrv2.exe"'
        elif kernel == 'Darwin':
            exe = f'/Applications/mrv2.app/Contents/MacOS/mrv2'
        else:
            print(_('Unknown platform'),kernel)
        return exe
    

    def start_new_mrv2(self, download_file, kernel):
        """Given a download_file and a platform, create the path to the new
        executable.
        """
        version = self.match_version(download_file)
        exe = self.get_installed_executable(download_file)
        
        try:
            tmp = os.path.join(self.tempdir, "installed.mrv2s")
            print(_('Saving session:',tmp)
            session.save(tmp)
            cmd.run(exe, tmp)
        except Exception as e:
            print(_('An unexpected error occurred:'),e)
            return

    # Function to start the subprocess in a thread
    def start_subprocess_in_thread(self, command, download_file):
        # Start the timeout callback
        data = [ self, download_file ]
        Fl.add_timeout(4.0, fltk_check_callback, data)
        threading.Thread(target=run_subprocess, args=(command,)).start()

                         
    def install_as_admin(self, command, download_file, password = None):
        """Given a command, a download file, and an optional password,
        install the download_file by running the command using the provided
        password if needed.
        Once the file is successfully installed, it removes the temporary
        downloaded file and starts the new version.
        
        Args:
            command (str): Command to run to install the downloaded file
            download_file (str): Full path to the downloaded file.
            password (str): Optional password for those platforms that require 
                            it.

        Returns:
            None
        """
        global subprocess_result
        cmd = None
        print(_('Trying to install'),download_file + '.')
        kernel = platform.system()
        if kernel != 'Windows':
            print(_('Please wait'), end='', flush=True)
            Fl.check()
        if kernel == 'Windows':
            cmd = r'Powershell -Command Start-Process "' + command + '" -Verb RunAs'
        elif kernel == 'Linux':
            if password != None:
                cmd = f'echo "{password}" | sudo -S {command}'
            else:
                cmd = command
        elif kernel == 'Darwin':
            cmd = command
        else:
            print(_('Unknown platform'))
            return

        self.start_subprocess_in_thread(cmd, download_file)

    def ask_for_password(self, command, download_file):
        """Open a FLTK window and secret input to enter a password for sudo
        access.

        Args:
            command (str): command to run to install the downloaded file.
            download_file (str): Full path to the file downloaded.

        Returns:
           None
        """
        win = Fl_Window(320, 100, _('Enter Sudo Password'))
        pwd = Fl_Secret_Input(20, 40, win.w() - 40, 40, _('Password'))
        pwd.textcolor(fl_rgb_color(0, 0, 0 ))
        pwd.align(FL_ALIGN_TOP)
        pwd.when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED)
        pwd.callback(_get_password_cb, [self, command, download_file])
        win.end()
        win.set_non_modal()
        win.show()
        while win.visible():
            Fl.check()


    def install_download(self, download_file):
        """Given a download file, use the extension name to try to install it.
        If required, pop a requester to ask for a password.

        Args:
            download_file (str): Full path to the file downloaded.

        Returns:
            None
        """
        if download_file.endswith('.exe'):
            command = download_file
            self.install_as_admin(command, download_file)
        elif download_file.endswith('.rpm'):
            command = f'rpm -i --force {download_file}'
            self.ask_for_password(command, download_file)
        elif download_file.endswith('.deb'):
            command = f'dpkg -i {download_file}'
            self.ask_for_password(command, download_file)
        elif download_file.endswith('.tar.gz'):
            command = f'tar -xzvf {download_file} -C ~/'
            self.install_as_admin(command, download_file)
        elif download_file.endswith('.dmg'):
            root_dir=cmd.rootPath()
            command = f'{root_dir}/bin/install_dmg.sh {download_file}'
            self.install_as_admin(command, download_file)
        else:
            print(f'You will need to install file "{download_file}" manually.')
            return
        

    def check_linux_flavor(self):
        """Given a Linux distribution, try to determine what package manager to
        use and return the package manager's extension to it.

        Returns:
           str: The extension for the Linux flavour, or None if undetermined.
        """

        # Prioritize checking for common package managers using subprocess.run
        # to capture return codes and avoid potential shell injection
        # vulnerabilities.

        package_managers = [
            ("rpm", ".rpm"),
            ("dpkg", ".deb"),
            ("pacman", ".pkg.tar.xz")  # Updated extension for Arch Linux
        ]

        for manager, extension in package_managers:
            try:
                result = subprocess.run(["which", manager],
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.DEVNULL)
                if result.returncode == 0:
                    return extension
            except OSError:
                # Handle potential errors during execution
                # (e.g., missing 'which' command)
                pass

        # If common package managers not found, return None for undetermined
        return ".tar.gz"


    def get_download_extension(self):
        """Based on platform, return the extension for the file
        to download.

        Returns:
            str: The extension of the platform or 'Unknown operating system'
        """
        os = platform.system()
        if os == 'Windows':
            return '.exe'
        elif os == 'Linux':
            return self.check_linux_flavor()
        elif os == 'Darwin':
            return 'amd64.dmg'
        else:
            return 'Unknown operating system'

    def download_file(self, name, download_url):
        import requests
        response = requests.get(download_url, stream=True)
        response.raise_for_status()  # Raise exception for non-200 status codes
        total_size = int(response.headers.get('content-length', 0))

        #
        # Create Progress window with Fl_Progress in it
        #
        window = Fl_Window(400, 120)
        progress = Fl_Progress(10, 50, 380, 60, _("Downloading..."))
        progress.minimum(0)
        progress.maximum(total_size)
        progress.align(FL_ALIGN_TOP)
        window.show()
        Fl.check()  # Ensure UI responsiveness

        # Create a temporary download file path
        download_file = os.path.join(self.tempdir, name)
        
        downloaded = 0
        with open(download_file, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                Fl.check()  # Ensure UI responsiveness
                if chunk:  # filter out keep-alive new chunks
                    downloaded += len(chunk)
                    f.write(chunk)
                    # Update UI with progress
                    progress.value(downloaded)
                    progress.redraw()
                    Fl.flush()
                    f.flush()
                    if not window.visible():
                        break
                    
        window.hide()

        if downloaded != total_size:
            print(_('Download seems to have failed!'))
            return False
        self.install_download(download_file)
        
    def fltk_ask_to_update(self, current_version, latest_version, title,
                           release_info):
        """Open an FLTK window to allow the user to update mrv2.

        Args:
            current_version (str): The current version that is running.
            latest_version (str): The latest version for upgrade.
            release_info (map): release_info for the latest release in a map.

        Returns:
            None
        """
        date = release_info['published_at']
        win = Fl_Window(320, 200)
        box = Fl_Box(20, 20, win.w() - 40, 60)
        label  = _('Current version is v') + current_version + '\n\n'
        label += _('Latest version at Github is v') + latest_version + '.\n'
        label += _('Released on ') + date
        box.copy_label(label)
        update = Fl_Button(20, 120, 130, 40, title)
        update.callback(_get_latest_release_cb, [self, release_info])
        ignore = Fl_Button(update.w() + 40, 120, 130, 40, _("Ignore"))
        ignore.callback(_ignore_cb, None)
        win.end()
        win.set_non_modal()
        win.show()
        while win.visible():
            Fl.check()


    def compare_versions(self, version1, version2):
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
    
    def get_latest_release_info(self, user, project):
        """Fetches details of the latest release from a GitHub repository.

        Args:
            user (str): The username or organization that owns the repository.
            project (str): The name of the repository.

        Returns:
            dict: A dictionary containing information about the latest release,
                  including the release name, tag name, and published at date
                  (if available).
                  Returns None if no releases are found or an error occurs.
        """
        import requests
        url = f"https://api.github.com/repos/{user}/{project}/releases/latest"
        try:
            response = requests.get(url)
            response.raise_for_status()  # Raise an exception for non-200 status codes
            data = response.json()
        except requests.exceptions.RequestException as e:
            print(_("Error fetching latest release information:\n"),e)
            return None
        
        extension = self.get_download_extension()
        for asset in data['assets']:
            name = asset['name']
            if name.endswith(extension):
                return {
                    "name": asset.get(GITHUB_ASSET_NAME, None),
                    "download_url": asset.get(GITHUB_ASSET_URL, None),
                    "tag_name": data.get(GITHUB_ASSET_TAG, None),
                    "published_at": data.get(GITHUB_ASSET_DATE, None)
                }
        return None
        
    def ask_to_update(self, release_info):
        release         = release_info['name']
        release_version = release_info['tag_name']
        release_date    = release_info['published_at']
        download_url    = release_info['download_url']
        if release_date:
            # Extract date from 'published_at' in ISO 8601 format
            # (e.g., 2024-05-17T06:50:00Z)
            release_date = release_date.split("T")[0]
            if UpdatePlugin.startup:
                if not _more_than_5_days_elapsed(release_date):
                    print(_("There's a new release but it has not been more "
                            "than 5 days"))
                    return
        else:
            release_date = _('unknown date.')

            
        release_info['published_at'] = release_date
        
        release_version = self.match_version(release_version)
        current_version = cmd.getVersion()
        
        result = self.compare_versions(current_version, release_version)
        if result == 0:
            if not UpdatePlugin.startup:
                self.fltk_ask_to_update(current_version, release_version,
                                        _('Update anyway'), release_info)
                return
        elif result == 1:
            if not UpdatePlugin.startup:
                self.fltk_ask_to_update(current_version, release_version,
                                        _('Downgrade'),
                                        release_info)
        else:
            self.fltk_ask_to_update(current_version, release_version,
                                    _('Upgrade'), relese_info)

    def check_latest_release(self, user, project):
        """Checks for the latest github release for a user and project.

        Args:
            user (str): The github user.
            project (str): The github project.

        Returns:
            None
        """

        release_info = self.get_latest_release_info(user, project)
        if release_info:
            download_url = release_info['download_url']
            if download_url:
                self.ask_to_update(release_info)
            else:
                print(_('No download url was found for'),release)
            
        else:
            print(_('No releases found for the specified repository.'))


    def run(self):
        self.check_latest_release("ggarra13", "mrv2")

    def menus(self):
        menu_entry = _("Help/Update mrv2")
        menus = {
            # Call a method and place a divider line after the menu
            menu_entry : (self.run, '__divider__')
        }
        return menus
        
