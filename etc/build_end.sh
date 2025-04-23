#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


if [[ "$CMAKE_TARGET" == "package" ]]; then

    echo "Target is package build.  Will copy packages..."

    #
    # Here we move the installers to the packages location.
    #
    if [[ $KERNEL == *Msys* ]]; then
	send_to_packages "mrv2-v${mrv2_VERSION}-Windows-${ARCH}.exe"
	send_to_packages "mrv2-v${mrv2_VERSION}-Windows-${ARCH}.zip"
	. etc/windows/windows_signing_installer.sh
    elif [[ $KERNEL == *Darwin* ]]; then
	send_to_packages "mrv2-v${mrv2_VERSION}-Darwin-${ARCH}.dmg"
	# . etc/macos_signing_installer.sh
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
    has_hdr=0
    if [[ -e $PWD/$BUILD_DIR/install/bin/hdr.sh ]]; then
	has_hdr=1
	chmod a+x $PWD/$BUILD_DIR/install/bin/hdr.sh
    fi
    if [ -d ~/bin ] && [ -w ~/bin ]; then
	if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	    run_cmd rm -f ~/bin/mrv2-dbg
	    run_cmd ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2-dbg
	    if [ $has_hdr == 1 ]; then
		run_cmd rm -f ~/bin/hdr-dbg
		run_cmd ln -s $PWD/$BUILD_DIR/install/bin/hdr.sh ~/bin/hdr-dbg
	    fi
	else
	    run_cmd rm -f ~/bin/mrv2
	    run_cmd ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2
	    if [ $has_hdr == 1 ]; then
		run_cmd rm -f ~/bin/hdr
		run_cmd ln -s $PWD/$BUILD_DIR/install/bin/hdr.sh ~/bin/hdr
	    fi
	fi
    fi
fi
