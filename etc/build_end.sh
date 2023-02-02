#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.





if [[ $KERNEL != *Msys* ]]; then
    chmod a+x $PWD/$BUILD_DIR/install/bin/mrv2.sh
    if [[ -d ~/bin ]]; then
	if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	    rm -f ~/bin/mrv2-dbg
	    ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2-dbg
	else
	    rm -f ~/bin/mrv2
	    ln -s $PWD/$BUILD_DIR/install/bin/mrv2.sh ~/bin/mrv2
	fi
    fi
fi
