#!/usr/bin/env bash

git for-each-ref --sort=creatordate \
  --format="%(refname:short) %(creatordate)" refs/tags
