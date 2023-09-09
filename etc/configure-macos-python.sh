#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash

echo "Running configure"
#./configure --enable-optimizations --with-openssl=$(brew --prefix openssl) --with-system-ffi --with-ensurepip=install $*
./configure --enable-optimizations --with-openssl=$(brew --prefix openssl) --with-ensurepip=install $*
