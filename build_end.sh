#!/usr/bin/env bash


if [[ $KERNEL != *MSYS* ]]; then
    chmod a+x $PWD/$BUILD_DIR/install/bin/mrViewer.sh
    if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
	rm -f ~/bin/mrv2-dbg
	ln -s $PWD/$BUILD_DIR/install/bin/mrViewer.sh ~/bin/mrv2-dbg
    else
	rm -f ~/bin/mrv2
	ln -s $PWD/$BUILD_DIR/install/bin/mrViewer.sh ~/bin/mrv2
    fi
fi

