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
import os, platform, re, subprocess, sys, tempfile, threading, time

#
# mrv2 imports
#
import mrv2
from mrv2 import cmd, plugin, settings

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



def _get_password_cb(widget, args):
    """FLTK callback to get the secret password, hide the parent window and
    run the command to install the downloaded file with the sudo password.
    
    Args:
        widget (Fl_Widget): FLTK's secret input widget.
        args (list): [self, command, download_file]
    """
    from fltk14 import Fl
    this    = args[0]
    command = args[1]
    download_file = args[2]
    password = widget.value()
    widget.parent().hide()
    Fl.check()
    this.run_as_admin(command, download_file, password)


def _ignore_cb(widget, args):
    """FLTK callback to ignore the upgrade and continue with the
    current version.

    Args:
        widget (Fl_Widget): FLTK widget that triggered the callback
        args (list): None

    Returns:
        None
    """
    from fltk14 import Fl
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
        print(f_("Invalid ISO 8601 format provided: {release_date_iso}"))
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
    from fltk14 import Fl
    this = args[0]
    release_info = args[1]
    extension = this.get_download_extension()
    widget.parent().hide()
    Fl.check()
    found = False
    name = release_info['name']
    if name.endswith(extension):
        this.download_version(name, release_info['download_url'])
    else:
        print(_(f'No file matching {extension} was found'))


class UpdatePlugin(plugin.Plugin):
    """
    This plugin checks GitHub for the latest release version of mrv2 and
    allows the user to download and install it.  It works on all platforms.
    """

    startup = True
    
    def __init__(self):
        super().__init__()
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
                print(_(f'Install Location: {exe}'))
            except WindowsError as e:
                print(_(f'Error retrieving value: {e}'))
            finally:
                # Always close the opened key
                winreg.CloseKey(key)

            #
            # Look for default install locations
            #
            if not os.path.exists(exe):
                exe = f'C:/Program Files/mrv2-v{version}/bin/mrv2.exe'
            if not os.path.exists(exe):
                exe = f'C:/Program Files/mrv2 v{version}/bin/mrv2.exe'
            if not os.path.exists(exe):
                exe = f'C:/Program Files/mrv2 {version}/bin/mrv2.exe'
        elif kernel == 'Darwin':
            exe = f'/Applications/mrv2.app/Contents/MacOS/mrv2'
        else:
            print(_(f'Unknown platform {kernel}'))
        return exe
        
    def run_command(self, cmd):
        """
        Run a command trapping stderr and stdout.

        Returns:
          int: Exit code of the process.
        """
        # Don't print out the command as that will print out the sudo password
        print('Running:\n',cmd)
        
        # Run the command and capture stdout, stderr, and the exit code
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        
        # Wait for the command to complete
        stdout, stderr = process.communicate()

        # Get the exit code
        exit_code = process.returncode

        # Print or handle the captured stdout and stderr
        print(_("Standard Output:"))
        print(stdout.decode('utf-8'))

        print(_("Standard Error:"))
        print(stderr.decode('utf-8'))

        return exit_code

    def start_new_mrv2(self, download_file, kernel):
        """Given a download_file and a platform, create the path to the new
        executable.
        """
        version = self.match_version(download_file)
        exe = self.get_installed_executable(download_file)
        if kernel == 'Windows':
            quoted_exe = f'"{exe}"'
        else:
            quoted_exe = exe

        try:
            print(_(f'Starting {exe}...'))
            os.execv(exe, [quoted_exe])
        except Exception as e:
            print(_('An unexpected error occurred:'),e)
            return

    def run_as_admin(self, command, download_file, password = None):
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
        cmd = None
        print(_(f'Trying to install {download_file}...'))
        Fl.check()
        kernel = platform.system()
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

        ret = self.run_command(cmd)
        if ret == 0:
            kernel = platform.system()

            # On Windows, the installer runs as a background process, but it
            # locks the installer file.  We keep trying to remove it until we
            # can, which means the installer finished
            if kernel == 'Windows':
                time.sleep(10) # Wait for the installer to start
                while os.path.exists(download_file):
                    try:
                        os.remove(download_file)
                        print(_(f'Removed temporary "{download_file}.'))
                        Fl.check()
                    except:
                        time.sleep(2) # Wait 2 seconds before trying again. 
                        pass

            exe = self.get_installed_executable(download_file)
            if os.path.exists(exe):
                print(_('The new version of mrv2 was installed.'))
                Fl.check()
                if os.path.exists(download_file):
                    os.remove(download_file)
                    print(_(f'Removed temporary "{download_file}.'))
                    print(_('Removed temporary'),download_file,'.')
                    Fl.check()
                self.start_new_mrv2(download_file, kernel)
            else:
                if kernel == 'Windows':
                    print(_(f'Could not locate mrv2 in:\n{exe}.\n') +
                          _(f'Maybe you installed it in a non-default location.'))
                    return
                print(_('Something failed installing mrv2 - It is not in "') +
                      exe + '"')
        else:
            print(_('Something failed installing mrv2 - Return error was: ') +
                    ret)
            


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
            self.run_as_admin(command, download_file)
        elif download_file.endswith('.rpm'):
            command = f'rpm -i {download_file}'
            self.ask_for_password(command, download_file)
        elif download_file.endswith('.deb'):
            command = f'dpkg -i {download_file}'
            self.ask_for_password(command, download_file)
        elif download_file.endswith('.tar.gz'):
            command = f'tar -xzvf {download_file} -C ~/'
            self.run_as_admin(command, download_file)
        elif download_file.endswith('.dmg'):
            root_dir=cmd.rootPath()
            command = f'{root_dir}/bin/install_dmg.sh {download_file}'
            self.run_as_admin(command, download_file)
        else:
            print(f'You will need to install file "{download_file}" manually.')
            return
    

    def check_linux_flavor(self):
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
            print(_('Unable to determine Linux flavor'))
            return '.tar.gz'


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

    def download_version(self, name, download_url):
        """Downloads a file from a download URL from GitHub in a separate thread.
        If successful, tries to install it. Updates UI during download progress.
            
        Args:
            name (str): Name of the file to download.
            download_url (str): URL for downloading the file.
        """

        def download_file_in_thread(download_url, download_file,
                                    progress_callback):
            """Downloads the file in a separate thread and updates progress."""
            from fltk14 import Fl
            import requests
            print(_(f'Downloading: {name} from {download_url}...'))
            Fl.check()  # Ensure UI responsiveness
            response = requests.get(download_url, stream=True)
            response.raise_for_status()  # Raise exception for non-200 status codes
                
            print("Get total size")
            Fl.check()  # Ensure UI responsiveness
            total_size = int(response.headers.get('content-length', 0))
            downloaded = 0

            with open(download_file, 'wb') as f:
                print('#',)
                Fl.check()  # Ensure UI responsiveness
                for chunk in response.iter_content(chunk_size=1024):
                    print('*',)
                    Fl.check()  # Ensure UI responsiveness
                    if chunk:  # filter out keep-alive new chunks
                        downloaded += len(chunk)
                        f.write(chunk)
                        progress_callback(downloaded, total_size)  # Update UI with progress
                        f.flush()

            Fl.check()  # Ensure UI responsiveness
            print(_(f'Download complete: {download_file}'))


        # Define a function to update UI progress
        def update_progress(downloaded, total_size):
            from fltk14 import Fl
            if total_size > 0:
                progress_percentage = int(downloaded * 100 / total_size)
                # Update your UI progress bar or indicator here with
                # `progress_percentage`
                print(f"Download progress: {progress_percentage}%")
                Fl.check()

        # Create a temporary download file path
        download_file = os.path.join(tempfile.gettempdir(), name)
        
        # Start the download in a separate thread
        download_thread = threading.Thread(target=download_file_in_thread,
                                           args=(download_url, download_file,
                                                 update_progress))
        download_thread.start()

        # Wait for the download thread to finish
        download_thread.join()

        if os.path.exists(download_file):
            self.install_download(download_file)
        else:
            print('Download seems to have failed!')

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
        from fltk14 import Fl, Fl_Window, Fl_Box, Fl_Button
        date = release_info['published_at']
        win = Fl_Window(320, 200)
        box = Fl_Box(20, 20, win.w() - 40, 60)
        box.copy_label(_(f'Current version is v{current_version}\n') + 
                       _(f'\nLatest version at Github is v{latest_version}.') +
                       _(f'\nReleased on {date}'))
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
            print(f"Error fetching latest release information: {e}")
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
                    print(_(f"There's a new release but it has not been more "
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
                print(_(f'No download url was found for {release}'))
            
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
        
