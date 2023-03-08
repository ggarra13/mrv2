#!/usr/bin/env bash

otool -l $1 | grep -E -A4 '(LC_VERSION_MIN_MACOSX|LC_BUILD_VERSION)' | grep -B1 sdk
