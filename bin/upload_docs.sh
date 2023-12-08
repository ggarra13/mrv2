#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/bin/bash

rsync -avP --exclude '*~' -e ssh docs/www/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs

rsync -avP --exclude '*~' -e ssh mrv2/docs/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs/docs
