#!/bin/bash

#
# This fixes a problem with times when switching in the same file system from
# Windows to Linux (clock skew).
#
find mrv2 -exec touch {} +
find tlRender -exec touch {} +
