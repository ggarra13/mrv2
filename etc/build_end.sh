#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


if [[ $CMAKE_BUILD_TYPE == Release && "$CMAKE_TARGET" == "package" ]]; then

    echo "Target is package and Release build.  Will copy packages..."

    #
    # Here we move the installers to the packages location.
    #
    if [[ $KERNEL == *Msys* ]]; then
	send_to_packages "mrv2-v${mrv2_VERSION}-Windows-${ARCH}.exe"
	send_to_packages "mrv2-v${mrv2_VERSION}-Windows-${ARCH}.zip"
	. etc/windows_signing_installer.sh
    elif [[ $KERNEL == *Darwin* ]]; then
	send_to_packages "mrv2-v${mrv2_VERSION}-Darwin-${ARCH}.dmg"
    elif [[ $KERNEL == *Linux* ]]; then
	send_to_packages "mrv2-v${mrv2_VERSION}-Linux-${ARCH}.deb"
	send_to_packages "mrv2-v${mrv2_VERSION}-Linux-${ARCH}.rpm"
	send_to_packages "mrv2-v${mrv2_VERSION}-Linux-${ARCH}.tar.gz"
    else
	echo "Kernel not recognized.  Will not move the files to packages/ directory."
    fi
fi


#
# Create symbolic links to start-up shell scripts
#
if [[ $KERNEL != *Msys* ]]; then
    chmod a+x $PWD/$BUILD_DIR/install/bin/mrv2.sh
    if [ -d ~/bin ] && [ -w ~/bin ]; then
	if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	    run_cmd rm -f ~/bin/mrv2-dbg
	    run_cmd ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2-dbg
	else
	    run_cmd rm -f ~/bin/mrv2
	    run_cmd ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2
	fi
    fi
fi
