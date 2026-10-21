#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <pull request number>"
    exit 1
fi

git fetch origin pull/$1/head:pr-$1
git switch pr-$1
