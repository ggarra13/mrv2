#!/usr/bin/env bash


if [[ $# != 1 ]]; then
    echo "$0 [version]"
    echo ""
    echo "$0 v1.4.2"
    exit 1
fi

version=$1

for i in `ls *beta*`; do
    new=`echo $i | sed -e "s#beta#${version}#"`
    echo "$i to $new"
    mv $i $new
done
