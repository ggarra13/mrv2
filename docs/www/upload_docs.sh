#!/bin/bash

rsync -avP --exclude '*~' -e ssh * ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs

