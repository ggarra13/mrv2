#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <file.cpp/h>"
    exit 1
fi

#
# FTK macros
#
sed -i 's#FTK_API##' $*
sed -i 's#FTK_NON_COPYABLE#TLRENDER_NON_COPYABLE#' $*

#
# tlRender macros
#

sed -i 's#TL_API##' $*
sed -i 's#TL_ENUM#TLRENDER_ENUM#' $*

#
# FTK classes
#
sed -i 's#ftk::M44F#math::Matrix4x4f#' $*
sed -i 's#ftk::M33F#math::Matrix3x3f#' $*

sed -i 's#ftk::V2F#math::Vector2f#' $*
sed -i 's#ftk::V2I#math::Vector2i#' $*

sed -i 's#ftk::Size2I#math::Size2i#' $*
sed -i 's#ftk::Size2F#math::Size2f#' $*
sed -i 's#ftk::Size3F#math::Size3f#' $*

#
# tlRender code
#
