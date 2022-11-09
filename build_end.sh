#!/usr/bin/env bash

. build_dir.sh

if [[ $KERNEL != *MSYS* ]]; then
    rm ~/bin/mrv2
    chmod a+x $PWD/$BUILD_DIR/install/bin/mrViewer.sh
    ln -s $PWD/$BUILD_DIR/install/bin/mrViewer.sh ~/bin/mrv2
fi

