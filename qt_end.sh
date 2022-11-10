#!/bin/bash

if [[ $KERNEL != *MSYS* ]]; then
    chmod a+x $PWD/$BUILD_DIR/install/bin/tlplay
    chmod a+x $PWD/$BUILD_DIR/install/bin/tlbake
    if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	rm -f ~/bin/tlplay-dbg
	rm -f ~/bin/tlbake-dbg
	ln -s $PWD/$BUILD_DIR/install/bin/tlplay ~/bin/tlplay-dbg
	ln -s $PWD/$BUILD_DIR/install/bin/tlbake ~/bin/tlbake-dbg
    else
	rm -f ~/bin/tlplay
	rm -f ~/bin/tlbake
	ln -s $PWD/$BUILD_DIR/install/bin/tlplay ~/bin/tlplay
	ln -s $PWD/$BUILD_DIR/install/bin/tlbake ~/bin/tlbake
    fi
fi

