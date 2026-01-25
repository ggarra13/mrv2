#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


echo "RUNNING upload_sourceforge.sh......"

. etc/build_dir.sh

upload_file()
{
    echo
    echo "Uploading $1 as $2..."
    echo

    rsync -avz -e "ssh -i $SSH_KEY -o StrictHostKeyChecking=no" $1 ggarra13@frs.sourceforge.net:/home/frs/project/mrv2/beta/$branch/$2
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
# Unset LD_LIBRARY_PATH and DYLD_LIBRARY_PATH just in case so we don't use
# an incorrect library that is not on the system
#
unset LD_LIBRARY_PATH
unset DYLD_LIBRARY_PATH

export branch=$(git rev-parse --abbrev-ref HEAD)
if [[ "$branch" != "beta" && \
          "$branch" != "peace" ]]; then
    echo "You are not on the beta, vulkan, peace or arm64 branch.  Will not make a release."
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


export mrv2_NAME=mrv2
export branch=opengl
if [[ $MRV2_BACKEND == "VK" ]]; then
    export mrv2_NAME=vmrv2
    export branch=vulkan
fi

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
package_dir=paquetes/$BUILD_DIR
mkdir -p $package_dir
cd $package_dir

#
# Remove all files if present
#
rm -f README.md
rm -f INSTALLATION_NOTES.md
rm -f VULKAN_NOTES.md

# Read all the files of this version
shopt -s nullglob
files=$(ls -1 *v${mrv2_VERSION}*)
shopt -u nullglob  # Optional: turn it off again


if [[ $KERNEL == *Windows* ]]; then
    pacman -Sy openssh rsync --noconfirm
fi

echo "OpenSSL version:"
openssl version
echo "ssh version:"
ssh -V

echo "Proceed with uploading..."

#
# Create the README.md file
#

cat <<EOF > README.md

${mrv2_NAME} v${mrv2_VERSION} ${branch}
====================

This beta release of v${mrv2_VERSION} corresponds to:
${date}.

It does not support NDI® on any platform.

It works on Windows 10+, Ubuntu 22.04 LTS+, macOS 15 (amd64) and macOS M1+ (arm64).
It may work (untested) on Windows 11 ARM64 and Linux ARM64.
Note that beta binaries perform about 15% slower than the final releases.

It may contain bugs, new untested features and more.

Enjoy!

-----------------------------------------------------------------

Donation Prices
---------------

Donationware prices of binary licenses through PayPal:

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=PSYEULZG24QHY)

I use the email information to contact you privately.  I don't sell your information, as I don't have access to it, except for your email, which I use to contact you.

- u\$  25 for a Solo license for a year.
- u\$  50 for a Standard license for a year.
- u\$  75 for an Edit license for a year.
- u\$ 150 for a Pro license for a year.
- u\$ 300 for a Pro license to own (also allowing changing machine id -
      	      	    	       	   useful for trying Linux distros or
				   upgrading machines) 

License works for both mrv2 and vmrv2 (you can have both installed).

Demo Versions
-------------

The OpenGL releases at:

https://sourceforge.net/projects/mrv2/files/beta/opengl

The Vulkan releases at:

https://sourceforge.net/projects/mrv2/files/beta/vulkan

directory are versions for you to evaluate mrv2 or vmrv2 before a donation and report bugs or performance issues.
Currently, when run as demo these versions don't have:

	   - Annotations
	   - Editing
	   - NDI support
	   - Performance as good as the release builds

They do have, however:

	   - Tone-mapping (OpenGL) and HDR (Vulkan)
	   - OpenEXR layer switching
	   - Saving
	   - A Menu Option (Help->Unlock Features) to unlock features
	     based on donations.

EOF

HISTORY=../../../src/docs/HISTORY.md
if [[ ! -f "$HISTORY" ]]; then
    echo "Error: $HISTORY not found."
    exit 1
fi

# Find the line number of "v${mrv2_VERSION}" in the file
start_lines=$(grep -n "^ChangeLog" "$HISTORY" | cut -d':' -f1)
if [[ -z "$start_lines" ]]; then
    echo "Error: Version v${mrv2_VERSION} has no ChangeLog in $HISTORY."
    exit 1
fi

# Convert end_lines to an array (compatible with Bash 3.2)
IFS=$'\n' read -d '' -r -a start_lines <<< "$start_lines" || true  # '|| true' handles potential read failure"

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
start_line=${start_lines[0]}
start_line=$((start_line))

# Use sed to extract the text between the two lines and store it in a variable
release_notes=$(sed -n "$start_line,${end_line}p" "$HISTORY")
if [[ -z "$release_notes" ]]; then
    echo "Warning: No release notes extracted between lines $start_line and $end_line."
else
    echo "Release notes extracted."
fi

echo "Appending release notes..."
echo "$release_notes" >> README.md

cat <<EOF > VULKAN_NOTES.md

Notes about Vulkan on Windows
-----------------------------

In order to get HDR, you currently need to have Windows 10+ (11 is ideal) and
an HDR monitor.

Notes about Vulkan on NVidia Linux
----------------------------------

In order to get HDR, you currently need to have the KWin6 or GNOME48 compositors and an HDR monitor.

It will likely work with any NVidia RTX 3080+, but it will require you to choose and test your hardware carefully.

The preferred NVidia driver tested for best performance is nvidia-driver-580+ (default on Ubuntu 25.04 with GNOME48+).

Notes about Vulkan on macOS Intel
---------------------------------

- Only up to Vulkan 1.3 is currently supported.
- HDR on macOS needs a MacBook Pro.
  MacBook Pro Intel will give you up to 1000 Nits of HDR depending on model.
- MacBook Pro Silicon should work with any M1 device up to HDR+.

EOF

cat <<EOF > INSTALLATION_NOTES.md

## Notes on installation

- For installation on each platform, please refer to:

  https://github.com/ggarra13/mrv2?tab=readme-ov-file#notes-on-installation


EOF

echo "Concatenating Installation notes"
cat INSTALLATION_NOTES.md >> README.md
rm INSTALLATION_NOTES.md

if [[ $branch == "vulkan" ]]; then
    echo "Concatenating Vulkan notes"
    cat VULKAN_NOTES.md >> README.md
fi
rm VULKAN_NOTES.md

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


# Iterate over the array of filenames
for src in "${file_array[@]}"; do
    dest=`echo $src | sed -e "s#v$mrv2_VERSION#beta#"`
    mv "${src}" "${dest}"
    upload_file "${dest}" "${dest}"
done

# Reset IFS to its default value (space, tab, newline)
IFS=$' \t\n'

# Go back to root directory
cd ../../..

