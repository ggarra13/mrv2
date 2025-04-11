// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline bool isPathSeparator(char value)
        {
            return value == pathSeparators[0] || value == pathSeparators[1];
        }

        constexpr bool PathOptions::operator==(const PathOptions& other) const
        {
            return maxNumberDigits == other.maxNumberDigits;
        }

        constexpr bool PathOptions::operator!=(const PathOptions& other) const
        {
            return !(*this == other);
        }

        inline const std::string& Path::getProtocol() const
        {
            return _protocol;
        }

        inline const std::string& Path::getProtocolName() const
        {
            return _protocolName;
        }

        inline bool Path::isFileProtocol() const
        {
            return _protocolName.empty() || "file:" == _protocolName;
        }

        inline const std::string& Path::getDirectory() const
        {
            return _directory;
        }

        inline const std::string& Path::getBaseName() const
        {
            return _baseName;
        }

        inline const std::string& Path::getNumber() const
        {
            return _number;
        }

        inline size_t Path::getPadding() const
        {
            return _padding;
        }

        inline const math::IntRange& Path::getSequence() const
        {
            return _sequence;
        }

        inline bool Path::sequence(const Path& value) const
        {
            return !_number.empty() && !value._number.empty() &&
                   _directory == value._directory &&
                   _baseName == value._baseName &&
                   (_padding == value._padding ||
                    _padding == value._numberDigits ||
                    _numberDigits == value._padding) &&
                   _extension == value._extension && _request == value._request;
        }

        inline bool Path::isSequence() const
        {
            return _sequence.getMin() != _sequence.getMax();
        }

        inline const std::string& Path::getExtension() const
        {
            return _extension;
        }

        inline const std::string& Path::getRequest() const
        {
            return _request;
        }

        inline bool Path::isEmpty() const
        {
            return _protocol.empty() && _directory.empty() &&
                   _baseName.empty() && _number.empty() && _extension.empty() &&
                   _request.empty();
        }

        inline bool Path::operator==(const Path& other) const
        {
            return _protocol == other._protocol &&
                   _directory == other._directory &&
                   _baseName == other._baseName && _number == other._number &&
                   _sequence == other._sequence && _padding == other._padding &&
                   _extension == other._extension && _request == other._request;
        }

        inline bool Path::operator!=(const Path& other) const
        {
            return !(*this == other);
        }
    } // namespace file
} // namespace tl
