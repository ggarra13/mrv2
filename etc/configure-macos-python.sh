#!/usr/bin/env bash

echo "Running configure"
./configure --enable-optimizations --with-openssl=$(brew --prefix openssl) --with-system-ffi --with-ensurepip=install $*
