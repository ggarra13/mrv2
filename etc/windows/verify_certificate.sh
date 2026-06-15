#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <installer.exe>"
    exit 1
fi

signtool.exe verify -pa -v "$1"

