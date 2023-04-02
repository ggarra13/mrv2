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
