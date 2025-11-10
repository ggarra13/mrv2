#!/usr/bin/env bash

app=$1
if [ "$app" == "" ]; then
    app=vmrv2
fi

echo "CHECKING /Applications/${app}.app"
otool -l /Applications/${app}.app/Contents/Resources/bin/mrv2 | grep -A2 LC_RPATH
