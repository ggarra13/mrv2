#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Rename /usr/bin/link.exe to /usr/bin/link_msys.exe so that we don't
# shallow MSVC's own link.exe (linker).
#
if [[ $KERNEL == *Msys* ]]; then
    if [[ -e /usr/bin/link.exe ]]; then
	echo "Renaming /usr/bin/link.exe as /usr/bin/link_msys.exe"
	mv /usr/bin/link.exe /usr/bin/link_msys.exe
    fi
    if [[ ! -e /usr/bin/xgettext ]]; then
	pacman -S --noconfirm gettext
    fi
fi

#
# Fix CMake/Ninja include list when compiling on non-English locale
#
export VSLANG=1033

#
# Set bits based on ARCH
#
export bits=64
if [[ "$ARCH" != "amd64" ]]; then
    export bits=32
fi

. windows/envvars/envvars.sh

#
# Extract python version from pyroot
#

pypath="Users/${USER}/AppData/Local/Programs/Python"

# Set the Python installation directory
PYTHON_DIR="C:/${pypath}"

# Find all installed Python versions
if [[ -d "$PYTHON_DIR" ]]; then
    versions=($(find "$PYTHON_DIR" -maxdepth 1 -type d -name "Python*" -exec basename {} \; | sed 's/Python//'))

    # Find the highest version
    highest_version=""
    for version in "${versions[@]}"; do
	# Check if the current version is higher than the current highest version
	if [[ "$version" > "$highest_version" ]]; then
	    # Store the current version as the highest version
	    highest_version="$version"
	fi
    done

    # Remove the architecture suffix from the highest version
    highest_version=$(echo "$highest_version" | sed 's/-.*//')

    # Store the highest version
    python_version=$highest_version

    pydir="${pypath}/Python${python_version}"
    
    #
    # Select the architecture Python to use (-32 suffix for 32 bits,
    #                                        nothing for 64 bits)
    #
    if [[ "$ARCH" == "amd64" ]]; then
	export PATH="/c/${pydir}/Scripts:/c/${pydir}:${PATH}"
	pydir="${pydir}-32"
    else
	export PATH="/c/${pydir}-32/Scripts:/c/${pydir}-32:${PATH}"
    fi

    #
    # Handle removing of other python architecture paths if installed
    #
    remove_path "/c/$pydir"
    remove_path "/c/$pydir/Scripts"
    remove_path "/C/$pydir"
    remove_path "/C/$pydir/Scripts"
fi

