#!/usr/bin/env bash

if [[ "$1" == "" ]]; then
    echo "$0 <file.cpp/h>"
    exit 1
fi

#
# OTIO macros
#
sed -i 's#OTIO_NS::#otio::#g' "$@"

#
# FTK macros
#
sed -i 's#namespace ftk#namespace tl#g' "$@"
sed -i 's#FTK_API_TYPE ##g' "$@"
sed -i 's#FTK_API##g' "$@"
sed -i 's#FTK_NON_COPYABLE#TLRENDER_NON_COPYABLE#g' "$@"
sed -i 's#FTK_P();#TLRENDER_P();#g' "$@"
sed -i 's#FTK_ENUM#TLRENDER_ENUM#' "$@"
sed -i 's#FTK_PRIVATE#TLRENDER_PRIVATE#g' "$@"

#
# tlRender macros
#
sed -i 's#TL_API ##' "$@"
sed -i 's#TL_API_TYPE ##' "$@"
sed -i 's#TL_ENUM#TLRENDER_ENUM#' "$@"
sed -i 's#TL_P;#TLRENDER_P;#' "$@"


#
# FTK classes
#
sed -i 's#ftk::Box2I#math::Box2i#g' "$@"
sed -i 's#ftk::Box2F#math::Box2f#g' "$@"

sed -i 's#ftk::M44F#math::Matrix4x4f#g' "$@"
sed -i 's#ftk::M33F#math::Matrix3x3f#g' "$@"

sed -i 's#ftk::V2F#math::Vector2f#g' "$@"
sed -i 's#ftk::V2I#math::Vector2i#g' "$@"

sed -i 's#ftk::Size2I#math::Size2i#g' "$@"
sed -i 's#ftk::Size2F#math::Size2f#g' "$@"
sed -i 's#ftk::Size3F#math::Size3f#g' "$@"

sed -i 's#ftk::RangeI32#math::IntRange#g' "$@"
sed -i 's#ftk::RangeI64#math::Int64Range#g' "$@"
sed -i 's#ftk::ImageInfo#image::Info#g' "$@"
sed -i 's#ftk::ImageTags#image::Tags#g' "$@"
sed -i 's#ftk::Image#image::Image#g' "$@"

sed -i 's#ftk::LogSystem#log::System#g' "$@"
sed -i 's#ftk::Context#system::Context#g' "$@"
sed -i 's#ftk::Path#file::Path#g' "$@"

sed -i 's#ftk::LRUCache#memory::LRUCache#g' "$@"

sed -i 's#ftk::MemFile#file::MemoryRead#g' "$@"

sed -i 's#ftk::FileIO#file::FileIO#g' "$@"
sed -i 's#ftk::FileMode::#file::Mode::#' "$@"
sed -i 's#ftk::FileRead::MMap#file::Read::MemoryMapped#' "$@"
sed -i 's#ftk::FileRead::#file::Read::#' "$@"
sed -i 's#ftk::FileAccess::#file::Access::#' "$@"

#
# ftk functions
#
sed -i 's#ftk::join#string::join#' "$@"
sed -i 's#ftk::toLower#string::toLower#' "$@"
sed -i 's#ftk::prefetch#file::prefetch#g' "$@"
sed -i 's#ftk::Format#string::Format#g' "$@"
sed -i 's#ftk::LogType#log::Type#g' "$@"

#
# ftk includes
#
sed -i 's#ftk/Core/Format#tlCore/StringFormat#' "$@"
sed -i 's#ftk/Core/#tlCore/#' "$@"


#
# tlRender includes
#
sed -i 's|<tlRender/|<tl|' "$@"

#
# tlRender classes
#
sed -i 's#memory::endian(#memory::swapEndian(#' "$@"
sed -i 's#\bFileType::Seq\b#io::FileType::Sequence#' "$@"
sed -i 's# IDecode# io::IDecode#' "$@"
sed -i 's#<IDecode#<io::IDecode#' "$@"
sed -i 's#<IWrite#<io::IWrite#' "$@"
sed -i 's# IReadPlugin# io::IReadPlugin#' "$@"
sed -i 's# IWritePlugin# io::IWritePlugin#' "$@"
sed -i 's#MissingFrames#io::MissingFrames#g' "$@"
sed -i 's# SeqDecode# io::SeqDecode#' "$@"
sed -i 's# VideoData readVideo(# io::VideoData readVideo(#' "$@"

#
# tlRender code
#
sed -i 's#ftk::truncateFile(#file::truncate(#g' "$@"
sed -i 's#ftk::FileMode#file::Mode#g' "$@"
sed -i 's#ftk::FileRead#file::ReadType#g' "$@"
sed -i 's#io::io::#io::#g' "$@"
sed -i 's#.getMin()#.min()#g' "$@"
sed -i 's#.getMax()#.max()#g' "$@"
sed -i 's#ftk::expandSeq#file::expandSeq#g' "$@"
sed -i 's#getExts(#getExtensions(#g' "$@"
sed -i 's#isStructural(#io::isStructural(#g' "$@"
sed -i 's#\.isSeq()#.isSequence()#g' "$@"
sed -i 's#\.getBase()#.getBaseName()#g' "$@"
sed -i 's#\.getPad()#.getPadding()#g' "$@"
sed -i 's#\.getNum()#.getNumber()#g' "$@"
sed -i 's#\.getDir()#.getDirectory()#g' "$@"
sed -i 's#\.getExt()#.getExtension()#g' "$@"
sed -i 's#\<IOInfo\>#io::Info#g' "$@"
sed -i 's#\<IOOptions\>#io::Options#g' "$@"
sed -i 's#<VideoData>#<io::VideoData>#g' "$@"
sed -i 's#<AudioData>#<io::AudioData>#g' "$@"
sed -i 's#getSystem<ReadSystem>#getSystem<io::ReadSystem>#g' "$@"
sed -i 's#getSystem<WriteSystem>#getSystem<io::WriteSystem>#g' "$@"
sed -i 's#tl::getPath#timeline::getPath#g' "$@"


sed -i 's#ftk::#file_wrong::#' "$@"
