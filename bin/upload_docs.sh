#!/bin/bash

rsync -avP --exclude '*~' -e ssh docs/www/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs

rsync -avP --exclude '*~' -e ssh mrv2/docs/* ggarra13@web.sourceforge.net:/home/project-web/mrv2/htdocs/docs
