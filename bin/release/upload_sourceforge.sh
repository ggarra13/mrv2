#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


echo "RUNNING upload_sourceforge.sh......"

. etc/build_dir.sh

#
# Unset LD_LIBRARY_PATH and DYLD_LIBRARY_PATH just in case so we don't use
# an incorrect library that is not on the system
#
unset LD_LIBRARY_PATH
unset DYLD_LIBRARY_PATH

branch=$(git rev-parse --abbrev-ref HEAD)
if [[ "$branch" != "beta" && "$branch" != "upload_test" ]]; then
    echo "You are not on the beta branch.  Will not make a release."
    exit 0
fi

if [[ -e ssh/id_rsa ]]; then
    export SSH_KEY=$PWD/ssh/id_rsa
else
    if [[ -e $HOME/.ssh/id256_rsa ]]; then
	export SSH_KEY=$HOME/.ssh/id256_rsa
    elif [[  -e $HOME/.ssh/id_rsa ]]; then
	export SSH_KEY=$HOME/.ssh/id_rsa
    else
	export SSH_KEY=$HOME/.ssh/mrv2_sourceforge
	if [[ ! -e $SSH_KEY ]]; then
	    echo "Unknown SSH key file. Aborting..."
	    exit 1
	fi
    fi
fi

echo "SSH KEY IS: ${SSH_KEY}"

get_kernel


#
# Get the date in English
#
export oldTZ=$TZ
export oldTIME=$LC_TIME
export oldALL=$LC_ALL
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8

#
# Report the date for 
#
export TZ="America/Argentina/Buenos_Aires"
date=`date "+%e of %B of %Y at %H:%M:%S Buenos Aires, Argentina (%z UTC/GMT)" | awk '{sub(/^ */, "", $1); print}'`
git_hash=`git rev-parse HEAD`
export TZ=$oldTZ
export LC_TIME=$oldTIME
export LC_ALL=$oldALL

echo "DATE: ${date}"


# Extract cmake/version.cmake into mrv2_VERSION
extract_version
echo "mrv2 VERSION=$mrv2_VERSION"


#
# Go to packages directory
#
package_dir=packages/$BUILD_DIR
echo "Looking for files in ${package_dir}"
mkdir -p $package_dir
cd $package_dir

# Read all the files of this version
files=$(ls -1 *v${mrv2_VERSION}*)


if [[ $KERNEL == *Msys* ]]; then
    pacman -Sy openssh rsync --noconfirm
fi

echo "OpenSSL version:"
openssl version
echo "ssh version:"
ssh -V

echo "Proceed with uploading..."

upload_file()
{
    echo
    echo "Uploading $1 as $2..."
    echo
    
    rsync -avz -e "ssh -i $SSH_KEY -o StrictHostKeyChecking=no" $1 ggarra13@frs.sourceforge.net:/home/frs/project/mrv2/beta/$2 2>&1 | tee rsync_error.log
    if [[ $? -ne 0 ]]; then
        echo "rsync command failed. Error log:"
	cat rsync_error.log
        exit 1
    fi
    echo
    echo "Upload was successful."
    echo
}

#
# Create the README.md file
#
cat <<EOF > README.md

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=679N8GWCNDFSJ)

mrv2 v${mrv2_VERSION} Beta
================

This beta release of v${mrv2_VERSION} corresponds to:
${date}.

It does not support NDI® on any platform.

It works on Windows 8.1+ (without USD support), Ubuntu 20.04 LTS+,
macOS 13 (amd64 also without USD support) and macOS M1/M2/M3 (arm64). 

It may contain bugs, new untested features and more.

You can extract it to compile yourself with:

\`\`\`
   git clone https://github.com/ggarra13/mrv2.git
   cd mrv2
   git checkout ${git_hash}
   ./runme.sh clean
\`\`\`


Enjoy!

----------------------------------------------------------------

## v${mrv2_VERSION} Notes

EOF

HISTORY=../../../src/docs/HISTORY.md
if [[ ! -f "$HISTORY" ]]; then
    echo "Error: $HISTORY not found."
    exit 1
fi

# Find the line number of "v${mrv2_VERSION}" in the file
start_line=$(grep -n "^v${mrv2_VERSION}" "$HISTORY" | cut -d':' -f1)
if [[ -z "$start_line" ]]; then
    echo "Error: Version v${mrv2_VERSION} not found in $HISTORY."
    exit 1
fi

# Find the line numbers of all "v*.*.*" occurrences in the file
end_lines=$(grep -n "^v[0-9]\+\.[0-9]\+\.[0-9]\+" "$HISTORY" | cut -d':' -f1)
if [[ -z "$end_lines" ]]; then
    echo "Error: No version lines found in $HISTORY."
    exit 1
fi

# Convert end_lines to an array (compatible with Bash 3.2)
IFS=$'\n' read -d '' -r -a end_lines <<< "$end_lines" || true  # '|| true' handles potential read failure"

# Initialize the end_line to 0
end_line=${end_lines[1]}
end_line=$((end_line - 1))
start_line=$((start_line + 2))

# Use sed to extract the text between the two lines and store it in a variable
release_notes=$(sed -n "$start_line,${end_line}p" "$HISTORY")
if [[ -z "$release_notes" ]]; then
    echo "Warning: No release notes extracted between lines $start_line and $end_line."
else
    echo "Release notes extracted."
fi

echo "$release_notes" >> README.md


cat <<"EOF" >> README.md

## Notes on installation

- On macOS you install it by opening the .dmg file, and dragging the mrv2
  icon to the Applications directory.  If there's already an mrv2 version,
  we recommend you overwrite it.
  The macOS application is currently not notarized, so when you launch it you
  will not be able to run it as macOS will warn you that the file is not secure
  as it was downloaded from internet.
  To avoid that, you need to open the Apple Logo->Settings->Privacy and Security
  and go to Security and allow "Opening Anyway".
  Alternatively, you can do it from the Terminal, by:
  
```
  sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/
```

- Windows and Chrome, like macOS, also protect you from installing files
  from the Internet.  When you first download it with Chrome it may warn
  you that it is not an usual archive to be downloaded.  Make sure to click
  on the right up arrow menu to Save it anyway.
  You cannot open the .exe from Chrome directly.  You will need to open
  Windows Explorer and go to the Downloads directory.  You should then
  run it from there.
  Then Windows will popup a Blue box telling you Windows SmartScreen
  prevented the start of an unknown application and that you can place your
  PC at risk.
  Click on the More Information text and a Button that says Run anyway or
  similar should appear.  Click on it and follow the standard instructions
  to any Windows installer.


- On Linux, in order to install the .rpm or .deb packages requires your user to
  have sudo permissions.

  On Debian (Ubuntu, etc) systems, you would install with:

```
  sudo dpkg -i mrv2-beta-Linux-amd64.tar.gz
```

  On Red Hat (Rocky Linux, etc), you would install it with:
  
```
  sudo rpm -i mrv2-beta-Linux-amd64.tar.gz
```

  Once you install it, you can run mrv2 by just typing mrv2 in the shell, as
  a symlink to the executable is placed in /usr/bin.  The installers will also
  associate file extensions and install an icon for easy starting up in the
  Desktop icon of the user that installed it.  For running mrv2 with the icon,
  you need to select it and use the right mouse button to open the menu and
  choose Allow Launch.
  
  If you lack sudo permissions in your organization, you should download the
  .tar.gz file and you can uncompress it with:
  
```
  tar -xf mrv2-beta-Linux-amd64.tar.gz
```

  That will create a folder in the directory you uncompress it from.  You can
  then run mrv2 by using the mrv2.sh shell script in the bin/ subdirectory.
EOF

echo "Upload README.md"
upload_file README.md README.md
rm README.md

if [[ "$files" == "" ]]; then
    echo
    echo "No v${mrv2_VERSION} files to upload.  Something probably went wrong in the compilation.  Only README.md file was updated."
    echo
    exit 1
fi

# Set the IFS to newline to split the variable by lines
IFS=$'\n'

# Convert the variable into an array
file_array=($files)

echo "FILES=$files"

# Iterate over the array of filenames
for src in "${file_array[@]}"; do
    dest=`echo $src | sed -e "s/v$mrv2_VERSION/beta/"`
    upload_file "${src}" "${dest}"
done

# Reset IFS to its default value (space, tab, newline)
IFS=$' \t\n'

# Go back to root directory
cd ../../..

