#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <file.cpp/h>"
    exit 1
fi

#
# OTIO macros
#
sed -i 's#OTIO_NS::#otio::#' $*

#
# FTK macros
#
sed -i 's#FTK_API##' $*
sed -i 's#FTK_NON_COPYABLE#TLRENDER_NON_COPYABLE#' $*
sed -i 's#FTK_P;#TLRENDER_P;#' $*

#
# tlRender macros
#
sed -i 's#TL_API##' $*
sed -i 's#TL_ENUM#TLRENDER_ENUM#' $*
sed -i 's#TL_P;#TLRENDER_P;#' $*

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
