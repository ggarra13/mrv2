#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

echo "Removing old files..."
rm -f *.pvk *.cer *.pvk *.pfx

echo "Calling makecert to create self signed CA..."
makecert -r -pe -n "CN=Gonzalo Garramuno (ggarra13@gmail.com)" -ss CA -sr CurrentUser -a sha256 -cy authority -sky signature -sv MyCA.pvk MyCA.cer

echo "Calling certutil..."
certutil -user -addstore Root MyCA.cer

echo "Calling makecert to create SPC..."
makecert -pe -n "CN=Gonzalo Garramuno (ggarra13@gmail.com)" -a sha256 -cy end -sky signature -ic MyCA.cer -iv MyCA.pvk -sv mrv2.pvk mrv2.cer

echo "Convert to pfx file..."
pvk2pfx -pvk mrv2.pvk -spc mrv2.cer -pfx mrv2.pfx
