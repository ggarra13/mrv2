// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender and feather-tk project.

namespace tl
{
    namespace file
    {
        
        inline bool isDotFile(const std::string& fileName)
        {
            return !fileName.empty() && '.' == fileName[0];
        }

        inline const PathOptions& Path::getOptions()
        {
            return _options;
        }

        inline const std::string& Path::get() const
        {
            return _path;
        }

        inline bool Path::isEmpty() const
        {
            return _path.empty();
        }

        inline bool Path::hasProtocol() const
        {
            return _protocol != _invalid;
        }

        inline bool Path::hasDirectory() const
        {
            return _dir != _invalid;
        }

        inline bool Path::hasBaseName() const
        {
            return _base != _invalid;
        }

        inline bool Path::hasNumber() const
        {
            return _num != _invalid;
        }

        inline bool Path::hasExtension() const
        {
            return _ext != _invalid;
        }

        inline bool Path::hasRequest() const
        {
            return _request != _invalid;
        }

        inline std::string Path::getProtocol() const
        {
            return _protocol != _invalid ?
                _path.substr(_protocol.first, _protocol.second) :
                std::string();
        }

        inline std::string Path::getDirectory() const
        {
            return _dir != _invalid ?
                _path.substr(_dir.first, _dir.second) :
                std::string();
        }

        inline std::string Path::getBaseName() const
        {
            return _base != _invalid ?
                _path.substr(_base.first, _base.second) :
                std::string();
        }

        inline std::string Path::getNumber() const
        {
            return _num != _invalid ?
                _path.substr(_num.first, _num.second) :
                std::string();
        }

        inline int Path::getPadding() const
        {
            return _pad;
        }

        inline std::string Path::getExtension() const
        {
            return _ext != _invalid ?
                _path.substr(_ext.first, _ext.second) :
                std::string();
        }

        inline std::string Path::getRequest() const
        {
            return _request != _invalid ?
                _path.substr(_request.first, _request.second) :
                std::string();
        }

        inline std::string Path::getFileName(bool dir) const
        {
            return dir ?
                getDirectory() + getBaseName() + getNumber() + getExtension() :
                getBaseName() + getNumber() + getExtension();
        }

        inline const std::optional<math::Int64Range>& Path::getFrames() const
        {
            return _frames;
        }

        inline bool Path::isSequence() const
        {
            return hasNumber() && _frames.has_value() &&
                !_frames.value().equal();
        }

        inline bool Path::hasSeqWildcard() const
        {
            return "#" == getNumber();
        }

        inline std::string Path::getFrame(int64_t frame, bool dir) const
        {
            return _num != _invalid ?
                ((dir ? getDirectory() : std::string()) + getBaseName() + toString(frame, _pad) + getExtension()) :
                ((dir ? getDirectory() : std::string()) + getBaseName() + getExtension());
        }

        inline std::string Path::getFrameRange() const
        {
            return _frames.has_value() ?
                !_frames.value().equal() ?
                toString(_frames.value().getMin(), _pad) + "-" + toString(_frames.value().getMax(), _pad) :
                toString(_frames.value().getMin(), _pad) :
                std::string();
        }

        inline bool Path::sequence(const Path& other) const
        {
            return
                (hasNumber() || "#" == getNumber()) &&
                other.hasNumber() &&
                _dir == other._dir &&
                _base == other._base &&
                getExtension() == other.getExtension();
        }
        
        inline bool Path::operator == (const Path& other) const
        {
            return _path == other._path && _frames == other._frames;
        }

        inline bool Path::operator != (const Path& other) const
        {
            return !(*this == other);
        }
        
    }
}
