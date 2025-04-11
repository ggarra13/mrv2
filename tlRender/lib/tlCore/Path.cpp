// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Path.h>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>

namespace tl
{
    namespace file
    {
        Path::Path() {}

        Path::Path(const std::string& value, const PathOptions& options)
        {
            if (!value.empty())
            {
                // Find the request.
                const size_t size = value.size();
                size_t i = size - 1;
                for (; i > 0 && value[i] != '?'; --i)
                    ;
                if (i > 0 && '?' == value[i])
                {
                    _request = value.substr(i, size - i);
                }
                else
                {
                    i = size;
                }

                // Find the extension.
                size_t j = i;
                for (; i > 0 && value[i] != '.' && !isPathSeparator(value[i]);
                     --i)
                    ;
                if (i > 0 && '.' == value[i] && '.' != value[i - 1] &&
                    !isPathSeparator(value[i - 1]))
                {
                    _extension = value.substr(i, j - i);
                }
                else
                {
                    i = size;
                }

                // Find the number.
                j = i;
                for (; i > 0 && value[i - 1] >= '0' && value[i - 1] <= '9'; --i)
                    ;
                if (value[i] >= '0' && value[i] <= '9' &&
                    options.maxNumberDigits > 0 &&
                    (j - i) <= options.maxNumberDigits)
                {
                    _number = value.substr(i, j - i);
                }
                else
                {
                    i = j;
                }

                // Find the directory.
                j = i;
                for (; i > 0 && !isPathSeparator(value[i]); --i)
                    ;
                size_t k = 0;
                if (isPathSeparator(value[i]))
                {
                    // Find the protocol.
                    //
                    //! \bug Should this be case-insensitive?
                    size_t l = i;
                    for (; l > 0 && value[l] != ':'; --l)
                        ;
                    if (':' == value[l] && 4 == l && 'f' == value[0] &&
                        'i' == value[1] && 'l' == value[2] && 'e' == value[3] &&
                        l < size - 4 && '/' == value[l + 1] &&
                        '/' == value[l + 2] && '/' == value[l + 3])
                    {
                        _protocol = value.substr(0, l + 3);
                        l += 3;
                    }
                    else if (
                        ':' == value[l] && 4 == l && 'f' == value[0] &&
                        'i' == value[1] && 'l' == value[2] && 'e' == value[3] &&
                        l < size - 4 && '/' == value[l + 1] &&
                        '/' == value[l + 2] && '/' == value[l + 3])
                    {
                        _protocol = value.substr(0, l + 3);
                        l += 3;
                    }
                    else if (
                        ':' == value[l] && 4 == l && 'f' == value[0] &&
                        'i' == value[1] && 'l' == value[2] && 'e' == value[3] &&
                        l < size - 3 && '/' == value[l + 1] &&
                        '/' == value[l + 2])
                    {
                        _protocol = value.substr(0, l + 3);
                        l += 3;
                    }
                    else if (
                        ':' == value[l] && 4 == l && 'f' == value[0] &&
                        'i' == value[1] && 'l' == value[2] && 'e' == value[3] &&
                        l < size - 2 && '/' == value[l + 1])
                    {
                        _protocol = value.substr(0, l + 1);
                        l += 1;
                    }
                    else if (
                        ':' == value[l] && 4 == l && 'f' == value[0] &&
                        'i' == value[1] && 'l' == value[2] && 'e' == value[3])
                    {
                        _protocol = value.substr(0, l + 1);
                    }
                    else if (
                        ':' == value[l] && l > 1 && l < size - 3 &&
                        '/' == value[l + 1] && '/' == value[l + 2])
                    {
                        _protocol = value.substr(0, l + 3);
                        l += 3;
                    }
                    else
                    {
                        l = 0;
                    }

                    _directory = value.substr(l, (i - l) + 1);
                    k = i + 1;
                }

                // Find the base name.
                if (k < j)
                {
                    _baseName = value.substr(k, j - k);
                }

                // Special case for Windows drive letters.
                //
                //! \bug Should this be case-insensitive?
                if (_directory.empty() && 2 == _baseName.size() &&
                    _baseName[0] >= 'A' && _baseName[0] <= 'Z' &&
                    ':' == _baseName[1])
                {
                    _directory.swap(_baseName);
                }

                _protocolUpdate();
                _numberUpdate();
            }
        }

        Path::Path(
            const std::string& directory, const std::string& value,
            const PathOptions& options) :
            Path(appendSeparator(directory) + value, options)
        {
            _protocolUpdate();
            _numberUpdate();
        }

        Path::Path(
            const std::string& directory, const std::string& baseName,
            const std::string& number, size_t padding,
            const std::string& extension, const std::string& protocol,
            const std::string& request) :
            _protocol(protocol),
            _directory(directory),
            _baseName(baseName),
            _number(number),
            _padding(padding),
            _extension(extension),
            _request(request)
        {
            _protocolUpdate();
            _numberUpdate();
        }

        std::string Path::get(int number, PathType type) const
        {
            std::stringstream ss;
            switch (type)
            {
            case PathType::Full:
                ss << _protocol;
            case PathType::Path:
                ss << _directory;
                break;
            default:
                break;
            }
            ss << _baseName;
            if (number != -1)
            {
                ss << std::setfill('0') << std::setw(_padding) << number;
            }
            else
            {
                ss << _number;
            }
            ss << _extension;
            ss << _request;
            return ss.str();
        }

        void Path::setProtocol(const std::string& value)
        {
            if (value == _protocol)
                return;
            _protocol = value;
            _protocolUpdate();
        }

        void Path::setDirectory(const std::string& value)
        {
            _directory = value;
        }

        void Path::setBaseName(const std::string& value)
        {
            _baseName = value;
        }

        void Path::setNumber(const std::string& value)
        {
            if (value == _number)
                return;
            _number = value;
            _numberUpdate();
        }

        void Path::setPadding(size_t value)
        {
            _padding = value;
        }

        void Path::setSequence(const math::IntRange& value)
        {
            _sequence = value;
        }

        std::string Path::getSequenceString() const
        {
            std::string out;
            if (isSequence())
            {
                out = string::Format("{0}-{1}")
                          .arg(_sequence.getMin(), _padding, '0')
                          .arg(_sequence.getMax(), _padding, '0');
            }
            return out;
        }

        void Path::setExtension(const std::string& value)
        {
            _extension = value;
        }

        void Path::setRequest(const std::string& value)
        {
            _request = value;
        }

        bool Path::isAbsolute() const
        {
            const std::size_t size = _directory.size();
            if (size > 0 && isPathSeparator(_directory[0]))
            {
                return true;
            }
            if (size > 1 && _directory[0] >= 'A' && _directory[0] <= 'Z' &&
                ':' == _directory[1])
            {
                return true;
            }
            return false;
        }

        void Path::_protocolUpdate()
        {
            if (!_protocol.empty())
            {
                const auto i = _protocol.find_first_of(':');
                if (i != std::string::npos)
                {
                    _protocolName = string::toLower(_protocol.substr(0, i + 1));
                }
            }
            else
            {
                _protocolName = std::string();
            }
        }

        void Path::_numberUpdate()
        {
            _numberValue = std::atoi(_number.c_str());
            _numberDigits = math::digits(_numberValue);
            _sequence = math::IntRange(_numberValue, _numberValue);
            if (_number.size() > 1 && '0' == _number[0])
            {
                _padding = _number.size();
            }
        }

        std::string appendSeparator(const std::string& value)
        {
            std::string out = value;
            const size_t size = out.size();
            char separator = pathSeparator;
            for (size_t i = 0; i < size; ++i)
            {
                if (out[i] == pathSeparators[0])
                {
                    separator = pathSeparators[0];
                    break;
                }
                else if (out[i] == pathSeparators[1])
                {
                    separator = pathSeparators[1];
                    break;
                }
            }
            if (size > 0 && !isPathSeparator(out[size - 1]))
            {
                out += separator;
            }
            return out;
        }

        std::string getParent(const std::string& value)
        {
            char startSeparator = 0;
            if (!value.empty() && isPathSeparator(value[0]))
            {
                startSeparator = value[0];
            }
            auto v = string::split(value, pathSeparators);
            if (startSeparator || v.size() > 1)
            {
                v.pop_back();
            }
            std::string out;
            if (startSeparator)
            {
                out += startSeparator;
            }
            out += string::join(v, pathSeparator);
            return out;
        }

        TLRENDER_ENUM_IMPL(
            UserPath, "Home", "Desktop", "Documents", "Downloads");
        TLRENDER_ENUM_SERIALIZE_IMPL(UserPath);
    } // namespace file
} // namespace tl
