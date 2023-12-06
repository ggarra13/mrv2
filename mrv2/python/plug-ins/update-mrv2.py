# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.




#
# Standard libs
#
import importlib, os, platform, re, tempfile, subprocess, sys, requests, time

#
# mrv2 imports
#
import mrv2
from mrv2 import cmd, plugin, settings
from fltk14 import *


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
            return "No match found"
        
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
            exe = f'C:/Program Files/mrv2 {version}/bin/mrv2.exe'
        elif kernel == 'Darwin':
            exe = f'/Applications/mrv2.app/Contents/MacOS/mrv2'
        else:
            print(f'Unknown platform "{kernel}"')
        return exe
        
    def run_command(self, cmd):
        """
        Run a command trapping stderr and stdout.

        Returns:
          int: Exit code of the process.
        """

        print(f"Running:\n{cmd}")
        
        # Run the command and capture stdout, stderr, and the exit code
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        
        # Wait for the command to complete
        stdout, stderr = process.communicate()

        # Get the exit code
        exit_code = process.returncode

        # Print or handle the captured stdout and stderr
        print("Standard Output:")
        print(stdout.decode('utf-8'))

        print("Standard Error:")
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
            print(f'Starting {exe}...')
            os.execv(exe, [quoted_exe])
        except Exception as e:
            print(f"An unexpected error occurred: {e}")
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
        print(f'Trying to install "{download_file}"...')
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
            print("Unknown platform")
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
                        print(f"Removed temporary {download_file}.")
                        Fl.check()
                    except:
                        time.sleep(2) # Wait 2 seconds before trying again. 
                        pass

            exe = self.get_installed_executable(download_file)
            if os.path.exists(exe):
                print("The new version of mrv2 was installed.")
                Fl.check()
                if os.path.exists(download_file):
                    os.remove(download_file)
                    print(f"Removed temporary {download_file}.")
                    Fl.check()
                self.start_new_mrv2(download_file, kernel)
            else:
                if kernel == 'Windows':
                    print(f'Could not locate mrv2 in "{exe}".  '
                          f'Maybe you installed it in a non-default location.')
                    return
                print(f'Something failed installing mrv2 - It is not in "{exe}"')
        else:
            print(f"Something failed installing mrv2 - Return error was {ret}")
            

    @staticmethod
    def get_password_cb(widget, args):
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
        this.run_as_admin(command, download_file, password)


    def ask_for_password(self, command, download_file):
        """Open a FLTK window and secret input to enter a password for sudo
        access.

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
        pwd.callback(UpdatePlugin.get_password_cb, [self, command, download_file])
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
            print('Unable to determine Linux flavor')
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
            return '.dmg'
        else:
            return 'Unknown operating system'


    def download_version(self, name, download_url):
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
        Fl.check()
        download_response = requests.get(download_url)
        with open(download_file, 'wb') as f:
            f.write(download_response.content)
        print(f"Download complete: {download_file}")
        if os.path.exists(download_file):
            self.install_download(download_file)

                
    @staticmethod
    def ignore_cb(widget, args):
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
                
    @staticmethod
    def get_latest_release_cb(widget, args):
        """FLTK callback to start the download of the latest release for
        the current platform.  Hides the widget's parent window when done.

        Args:
            widget (Fl_Widget): FLTK widget that triggered the callback
            args (list): [self, github data for the latest release in a map.]

        Returns:
            None
        """
        this = args[0]
        data = args[1]
        extension = this.get_download_extension()
        widget.parent().hide()
        Fl.check()
        found = False
        for asset in data['assets']:
            name = asset['name']
            if name.endswith(extension):
                this.download_version(name, asset['browser_download_url'])
                found = True
                break
        if not found:
            print(f"No file matching {extension} was found")


    def ask_to_update(self, current_version, latest_version, title, data):
        """Open an FLTK window to allow the user to update mrv2.

        Args:
            current_version (str): The current version that is running.
            latest_version (str): The latest version for upgrade.
            data (map): github data for the latest release in a map.

        Returns:
            None
        """
        win = Fl_Window(320, 200)
        box = Fl_Box(20, 20, win.w() - 40, 60)
        box.copy_label(f"Current version is v{current_version},\n"
                       f"Latest at Github is v{latest_version}.")
        update = Fl_Button(20, 100, 130, 40, title)
        update.callback(UpdatePlugin.get_latest_release_cb, [self, data])
        ignore = Fl_Button(update.w() + 40, 100, 130, 40, "Ignore")
        ignore.callback(UpdatePlugin.ignore_cb, None)
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


    def check_latest_release(self, user, project):
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
            version = self.match_version(release)

            result = self.compare_versions(current_version, version)
            if result == 0:
                if not UpdatePlugin.startup:
                    self.ask_to_update(current_version, version,
                                       'Update anyway', data)
                return
            elif result == 1:
                if not UpdatePlugin.startup:
                    self.ask_to_update(current_version, version, 'Downgrade',
                                       data)
                return
            else:
                self.ask_to_update(current_version, version, 'Upgrade', data)
            
        else:
            print("No release files found.")


    def run(self):
        self.check_latest_release("ggarra13", "mrv2")

    def menus(self):
        menu_entry = "Help/Update mrv2"
        try:
            if re.search('es', cmd.getLanguage()):
                menu_entry = "Ayuda/Actualizar mrv2"
        except:
            pass
        menus = {
            # Call a method and place a divider line after the menu
            menu_entry : (self.run, '__divider__')
        }
        return menus
        
