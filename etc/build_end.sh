#!/usr/bin/env bash


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
