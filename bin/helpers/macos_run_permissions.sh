#!/usr/bin/env bash

apps="mrv2 vmrv2 hdr"
for app in $apps; do
    sudo xattr -rd com.apple.quarantine /Applications/${app}.app/
done
