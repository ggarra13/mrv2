// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/Timeline.h>

#include <tlTimelineUI/IItem.h>

#include <FL/Fl.H>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvMemory.h"
#include "mrvCore/mrvOS.h"

#include "mrvFl/mrvIO.h"

#if defined(TLRENDER_USD)
#    include "mrvOptions/mrvUSD.h"
#endif // TLRENDER_USD

#include "mrvApp/mrvSettingsObject.h"

namespace mrv
{
    namespace
    {
        const char* kModule = "settings";
        const int kRecentFilesMax = 10;
        const int kRecentHostsMax = 10;
        const int kRecentPythonScriptsMax = 10;
    } // namespace

    namespace
    {
    }

    struct SettingsObject::Private
    {
        std::map<std::string, std_any> defaultValues;
        std::map<std::string, std_any> settings;
        std::vector<std::string> recentFiles;
        std::vector<std::string> recentHosts;
        std::vector<std::string> pythonScripts;
    };

    SettingsObject::SettingsObject() :
        _p(new Private)
    {
        TLRENDER_P();

        // GUI Window defaults (all panels)
        p.defaultValues["gui/Annotations/Window"] = 0;
        p.defaultValues["gui/Background/Window"] = 0;
        p.defaultValues["gui/Color Area/Window"] = 0;
        p.defaultValues["gui/Color/Window"] = 0;
        p.defaultValues["gui/Compare/Window"] = 0;
        p.defaultValues["gui/Devices/Window"] = 0;
        p.defaultValues["gui/Environment Map/Window"] = 0;
        p.defaultValues["gui/Files/Window"] = 0;
        p.defaultValues["gui/Histogram/Window"] = 0;
        p.defaultValues["gui/Media Info/Window"] = 0;
        p.defaultValues["gui/NDI/Window"] = 0;
        p.defaultValues["gui/Network/Window"] = 0;
        p.defaultValues["gui/Playlist/Window"] = 0;
        p.defaultValues["gui/Settings/Window"] = 0;
        p.defaultValues["gui/Stereo 3D/Window"] = 0;
        p.defaultValues["gui/USD/Window"] = 0;

        p.defaultValues["gui/Files/WindowW"] = 400;
        p.defaultValues["gui/Compare/WindowW"] = 400;
        p.defaultValues["gui/Stereo 3D/WindowW"] = 400;

        p.defaultValues["gui/Logs/Window"] = 1;
        p.defaultValues["gui/Logs/WindowW"] = 800;
        p.defaultValues["gui/Logs/WindowH"] = 400;

        p.defaultValues["gui/Python/Window"] = 1;
        p.defaultValues["gui/Python/WindowW"] = 640;
        p.defaultValues["gui/Python/WindowH"] = 400;

        p.defaultValues["NDI/Output/Stream Name"] = "mrv2";
        p.defaultValues["NDI/HDRData"] = "";
        p.defaultValues["BMD/HDRData"] = "";
        
        uint64_t totalVirtualMem = 0;
        uint64_t virtualMemUsed = 0;
        uint64_t virtualMemUsedByMe = 0;
        uint64_t totalPhysMem = 0;
        uint64_t physMemUsed = 0;
        uint64_t physMemUsedByMe = 0;
        memory_information(
            totalVirtualMem, virtualMemUsed, virtualMemUsedByMe, totalPhysMem,
            physMemUsed, physMemUsedByMe);
        totalPhysMem /= 1024;

        p.defaultValues["Timeline/Editable"] = true;
        p.defaultValues["Timeline/TrackInfo"] =
            TIMELINEUI::DisplayOptions().trackInfo;
        p.defaultValues["Timeline/ClipInfo"] =
            TIMELINEUI::DisplayOptions().clipInfo;
        p.defaultValues["Timeline/ScrollToCurrentFrame"] = true;
        p.defaultValues["Timeline/StopOnScrub"] = true;
        p.defaultValues["Timeline/FirstTrack"] = false;
        p.defaultValues["Audio/Volume"] = 1.0F;
        p.defaultValues["Audio/Mute"] = false;
        p.defaultValues["Cache/GBytes"] = static_cast<int>(totalPhysMem / 2);
        p.defaultValues["Cache/ReadAhead"] =
            timeline::PlayerCacheOptions().readAhead.value();
        p.defaultValues["Cache/ReadBehind"] =
            timeline::PlayerCacheOptions().readBehind.value();
        p.defaultValues["FileSequence/Audio"] =
            static_cast<int>(timeline::FileSequenceAudio::BaseName);
        p.defaultValues["FileSequence/AudioFileName"] = std::string();
        p.defaultValues["FileSequence/AudioDirectory"] = std::string();
        const timeline::PlayerOptions playerOptions;
        p.defaultValues["Performance/TimerMode"] =
            static_cast<int>(playerOptions.timerMode);
        p.defaultValues["Performance/AudioBufferFrameCount"] =
            static_cast<int>(playerOptions.audioBufferFrameCount);
        p.defaultValues["SequenceIO/ThreadCount"] = 16;
        p.defaultValues["Performance/VideoRequestCount"] = 16;
        p.defaultValues["Performance/AudioRequestCount"] = 16;
        p.defaultValues["Performance/FFmpegThreadCount"] = 0;
        p.defaultValues["Performance/FFmpegYUVToRGBConversion"] = 0;
        p.defaultValues["Performance/FFmpegColorAccuracy"] = 0;
        p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;
        p.defaultValues["EnvironmentMap/Sphere/SubdivisionX"] = 36;
        p.defaultValues["EnvironmentMap/Sphere/SubdivisionY"] = 36;
        p.defaultValues["EnvironmentMap/Spin"] = 1;
        p.defaultValues["TCP/Control/Port"] = std::string("55150");

        Fl_Color c;
        p.defaultValues["Background/Type"] = 0;
        c = fl_rgb_color(128, 128, 128);
        p.defaultValues["Background/color1"] = static_cast<int>(c);
        c = fl_rgb_color(255, 255, 255);
        p.defaultValues["Background/color0"] = static_cast<int>(c);
        p.defaultValues["Background/CheckersSize"] = 100;

#if defined(TLRENDER_USD)
        p.defaultValues["USD/rendererName"] = usd::RenderOptions().rendererName;
        p.defaultValues["USD/renderWidth"] =
            static_cast<int>(usd::RenderOptions().renderWidth);
        p.defaultValues["USD/complexity"] =
            static_cast<float>(usd::RenderOptions().complexity);
        p.defaultValues["USD/drawMode"] =
            static_cast<int>(usd::RenderOptions().drawMode);
        p.defaultValues["USD/enableLighting"] =
            static_cast<int>(usd::RenderOptions().enableLighting);
        p.defaultValues["USD/enableSceneLights"] = 
            static_cast<int>(usd::RenderOptions().enableSceneLights);
        p.defaultValues["USD/enableSceneMaterials"] = 
            static_cast<int>(usd::RenderOptions().enableSceneMaterials);
        p.defaultValues["USD/sRGB"] =
            static_cast<int>(usd::RenderOptions().sRGB);
        p.defaultValues["USD/stageCacheByteCount"] =
            static_cast<int>(usd::RenderOptions().stageCache);
        p.defaultValues["USD/diskCacheByteCount"] =
            static_cast<int>(usd::RenderOptions().diskCache / memory::gigabyte);
#endif

#if defined(TLRENDER_NDI)
        p.defaultValues["NDI/SourceIndex"] = -1;
        p.defaultValues["NDI/Preroll"] = 3;
        p.defaultValues["NDI/Audio"] = 0;
#endif

        p.defaultValues[kTextFont] = 0;
        p.defaultValues[kFontSize] = 52;

        p.defaultValues[kPenColorR] = 255;
        p.defaultValues[kPenColorG] = 255;
        p.defaultValues[kPenColorB] = 0;
        p.defaultValues[kPenColorA] = 255;

        p.defaultValues[kOldPenColorR] = 255;
        p.defaultValues[kOldPenColorG] = 0;
        p.defaultValues[kOldPenColorB] = 0;
        p.defaultValues[kOldPenColorA] = 255;

        p.defaultValues[kLaser] = false;
        p.defaultValues[kPenSize] = 10;
        p.defaultValues[kSoftBrush] = false;

        p.defaultValues[kGhostPrevious] = 15;
        p.defaultValues[kGhostNext] = 15;

        p.defaultValues[kAllFrames] = 0;

        // Image saving
        p.defaultValues["SaveImage/Annotations"] = 0;
        p.defaultValues["SaveImage/Compression"] = std::string("ZIPS");
        p.defaultValues["SaveImage/PixelType"] = std::string("Half");

        // Movie saving
        p.defaultValues["SaveMovie/Annotations"] = 0;
        p.defaultValues["SaveMovie/HardwareEncode"] = 0;
        p.defaultValues["SaveMovie/Profile"] = std::string("ProRes_4444");
        p.defaultValues["SaveMovie/Resolution"] = 0;
        p.defaultValues["SaveMovie/PixelFormat"] =
            std::string("YUVA_444P_LE10");
        p.defaultValues["SaveMovie/Preset"] = std::string("good");

        p.defaultValues["SaveMovie/AudioCodec"] = std::string("AAC");

        bool has_terminal = os::runningInTerminal();

        std::string command = "vim"; // Use vim as default
        char* editor = fl_getenv("EDITOR");
        if (editor && file::isInPath(editor))

        {
            command = editor;
        }
        else
        {
#ifdef _WIN32
            command = "emacs.exe";
            if (!file::isInPath(command))
                command = "gvim.exe";
            if (!file::isInPath(command))
                command = "notepad++.exe";
            if (!file::isInPath(command))
                command = "notepad.exe";
#elif defined(__linux__)
            command = "emacs";
            if (!file::isInPath(command))
                command = "gvim";
            if (!file::isInPath(command))
                command = "code";
            if (!file::isInPath(command) && has_terminal)
            {
                command = "vim";
                if (!file::isInPath(command))
                    command = "nano";
            }
#elif defined(__APPLE__)
            command = "emacs";
            if (!file::isInPath(command))
                command = "code";
            if (!file::isInPath(command) && has_terminal)
            {
                command = "vim";
            }
#else
#    error Unknown OS to set editor
#endif
        }

        if (command.substr(0, 5) == "emacs" || command.substr(0, 3) == "vim" ||
            command.substr(0, 4) == "nano" || command.substr(0, 4) == "gvim" ||
            command.substr(0, 3) == "zed")
        {
            command += " +{0} '{1}'";
        }
        else if (command.substr(0, 9) == "notepad++")
        {
            command += " -n{0} '{1}'";
        }
        else if (command.substr(0, 4) == "code" ||
                 command.substr(0, 6) == "cursor")
        {
            command += " '{1}:{0}'";
        }
        else if (command.substr(0, 7) == "pycharm")
        {
            command += " --line {0} '{1}'";
        }
        else if (command.substr(0, 7) == "notepad")
        {
            command += " '{1}'";
        }

        p.defaultValues["Python/Editor"] = command;
    }

    SettingsObject::~SettingsObject() {}

    const std::vector<std::string> SettingsObject::keys() const
    {
        TLRENDER_P();
        std::vector< std::string > ret;
        ret.reserve(p.settings.size() + p.defaultValues.size());
        for (const auto& m : p.settings)
        {
            ret.push_back(m.first);
        }
        for (const auto& m : p.defaultValues)
        {
            if (std::find(ret.begin(), ret.end(), m.first) != ret.end())
                continue;
            ret.push_back(m.first);
        }
        return ret;
    }

    std::any SettingsObject::_value(const std::string& name)
    {
        TLRENDER_P();
        std_any defaultValue;
        auto i = p.settings.find(name);
        if (i != p.settings.end())
        {
            return p.settings[name];
        }

        i = p.defaultValues.find(name);
        if (i != p.defaultValues.end())
        {
            defaultValue = i->second;
        }
        return p.settings[name] = defaultValue;
    }

    std::any SettingsObject::_defaultValue(const std::string& name)
    {
        TLRENDER_P();
        std::any defaultValue;
        auto i = p.defaultValues.find(name);
        if (i != p.defaultValues.end())
        {
            defaultValue = i->second;
        }
        return p.settings[name] = defaultValue;
    }

    bool SettingsObject::hasValue(const std::string& name)
    {
        TLRENDER_P();
        auto i = p.settings.find(name);
        if (i != p.settings.end())
        {
            return true;
        }
        return false;
    }

    template < typename T > T SettingsObject::getValue(const std::string& name)
    {
        T out;
        std::any value = _value(name);
        try
        {
            out = std::any_cast<T>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " " << e.what() << " is " << anyName(value));
        }
        return out;
    }

    template <> std::string SettingsObject::getValue(const std::string& name)
    {
        std::string out;
        std::any value = _value(name);
        try
        {
            out = std::any_cast<std::string>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                out = std::any_cast<const char*>(value);
            }
            catch (const std::bad_any_cast& e)
            {
                LOG_ERROR(
                    "For " << name << " " << e.what() << " should be string is "
                    << anyName(value));
                setValue(name, out);
            }
        }
        return out;
    }

    template <> std::any SettingsObject::getValue(const std::string& name)
    {
        return _value(name);
    }

    template <> bool SettingsObject::getValue(const std::string& name)
    {
        bool out = false;
        std_any value = _value(name);
        try
        {
            out = std_any_empty(value) ? false : std::any_cast<bool>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                out = static_cast<bool>(std::any_cast<int>(value));
            }
            catch (const std::bad_any_cast& e)
            {
                LOG_ERROR(
                    "For " << name << " " << e.what() << " should be bool is "
                           << anyName(value));
                setValue(name, out);
            }
        }
        return out;
    }

    template <> int SettingsObject::getValue(const std::string& name)
    {
        int out = 0;
        std_any value = _value(name);
        try
        {
            out = std_any_empty(value) ? 0 : std::any_cast<int>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                out = static_cast<int>(std::any_cast<bool>(value));
            }
            catch (const std::bad_any_cast& e)
            {
                try
                {
                    value = _defaultValue(name);
                    out = std_any_empty(value) ? 0 : std::any_cast<int>(value);
                }
                catch (const std::bad_any_cast& e)
                {
                    LOG_ERROR(
                        "For " << name << " should be int " << e.what()
                               << " is " << anyName(value));
                }
            }
        }
        return out;
    }

    template <> float SettingsObject::getValue(const std::string& name)
    {
        float out = 0.F;
        std_any value = _value(name);
        try
        {
            out = std_any_empty(value) ? 0.F : std::any_cast<float>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                out = static_cast<float>(std::any_cast<double>(value));
            }
            catch (const std::bad_any_cast& e)
            {
                try
                {
                    value = _defaultValue(name);
                    out =
                        std_any_empty(value) ? 0 : std::any_cast<float>(value);
                }
                catch (const std::bad_any_cast& e)
                {
                    LOG_ERROR(
                        "For " << name << " should be float " << e.what()
                               << " is " << anyName(value));
                }
            }
        }
        return out;
    }

    template <> double SettingsObject::getValue(const std::string& name)
    {
        double out = 0.F;
        std_any value = _value(name);
        try
        {
            out = std_any_empty(value) ? 0.F : std::any_cast<double>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                value = _defaultValue(name);
                out = std_any_empty(value) ? 0 : std::any_cast<double>(value);
            }
            catch (const std::bad_any_cast& e)
            {
                LOG_ERROR(
                    "For " << name << " should be double " << e.what() << " is "
                           << anyName(value));
            }
        }
        return out;
    }

    //
    // Get default values
    //
    template < typename T >
    T SettingsObject::getDefaultValue(const std::string& name)
    {
        T out;
        std::any value = _defaultValue(name);
        try
        {
            out = std::any_cast<T>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " " << e.what() << " is " << anyName(value));
        }
        return out;
    }

    template <>
    std::string SettingsObject::getDefaultValue(const std::string& name)
    {
        std::string out;
        std::any value = _defaultValue(name);
        try
        {
            out = std::any_cast<std::string>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " " << e.what() << " should be string is "
                       << anyName(value));
            setValue(name, out);
        }
        return out;
    }

    template <>
    std::any SettingsObject::getDefaultValue(const std::string& name)
    {
        return _defaultValue(name);
    }

    template <> bool SettingsObject::getDefaultValue(const std::string& name)
    {
        bool out = false;
        std_any value = _defaultValue(name);
        try
        {
            out = std_any_empty(value) ? false : std::any_cast<bool>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            try
            {
                out = static_cast<bool>(std::any_cast<int>(value));
            }
            catch (const std::bad_any_cast& e)
            {
                LOG_ERROR(
                    "For " << name << " " << e.what() << " should be bool is "
                           << anyName(value));
                setDefaultValue(name, out);
            }
        }
        return out;
    }

    template <> int SettingsObject::getDefaultValue(const std::string& name)
    {
        int out = 0;
        std_any value;
        try
        {
            value = _defaultValue(name);
            out = std_any_empty(value) ? 0 : std::any_cast<int>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " should be int " << e.what() << " is "
                       << anyName(value));
        }
        return out;
    }

    template <> float SettingsObject::getDefaultValue(const std::string& name)
    {
        float out = 0.F;
        std_any value;
        try
        {
            value = _defaultValue(name);
            out = std_any_empty(value) ? 0 : std::any_cast<float>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " should be float " << e.what() << " is "
                       << anyName(value));
        }
        return out;
    }

    template <> double SettingsObject::getDefaultValue(const std::string& name)
    {
        double out = 0.F;
        std_any value;
        try
        {
            value = _defaultValue(name);
            out = std_any_empty(value) ? 0 : std::any_cast<double>(value);
        }
        catch (const std::bad_any_cast& e)
        {
            LOG_ERROR(
                "For " << name << " should be double " << e.what() << " is "
                       << anyName(value));
        }
        return out;
    }

    const std::vector<std::string>& SettingsObject::recentFiles() const
    {
        return _p->recentFiles;
    }

    const std::vector<std::string>& SettingsObject::recentHosts() const
    {
        return _p->recentHosts;
    }

    const std::vector<std::string>& SettingsObject::pythonScripts() const
    {
        return _p->pythonScripts;
    }

    const std::string SettingsObject::pythonScript(size_t index) const
    {
        if (index >= _p->pythonScripts.size())
            return "";
        return _p->pythonScripts[index];
    }

    void SettingsObject::setValue(const std::string& name, const std_any& value)
    {
        _p->settings[name] = value;
    }

    void SettingsObject::setDefaultValue(
        const std::string& name, const std_any& value)
    {
        _p->defaultValues[name] = value;
    }

    void SettingsObject::reset()
    {
        TLRENDER_P();
        p.settings.clear();
        for (auto i = p.defaultValues.begin(); i != p.defaultValues.end(); ++i)
        {
            p.settings[i->first] = i->second;
        }
        p.recentFiles.clear();
    }

    void SettingsObject::addRecentFile(const std::string& fileName)
    {
        TLRENDER_P();

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.recentFiles.insert(p.recentFiles.begin(), fileName);

        for (const auto& str : p.recentFiles)
        {
            fs::path filePath(str);
            if (fs::exists(filePath))
            {
                auto path = fs::absolute(filePath);
                if (set.find(path.u8string()) == set.end())
                {
                    set.insert(path.u8string());
                    result.push_back(str);
                }
            }
        }

        while (result.size() > kRecentFilesMax)
        {
            result.pop_back();
        }

        p.recentFiles = result;
    }

    void SettingsObject::addRecentHost(const std::string& fileName)
    {
        TLRENDER_P();

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.recentHosts.insert(p.recentHosts.begin(), fileName);

        for (const auto& str : p.recentHosts)
        {
            if (set.find(str) == set.end())
            {
                set.insert(str);
                result.push_back(str);
            }
        }

        while (result.size() > kRecentHostsMax)
        {
            result.pop_back();
        }

        p.recentHosts = result;
    }

    void SettingsObject::addPythonScript(const std::string& fileName)
    {
        TLRENDER_P();

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.pythonScripts.insert(p.pythonScripts.begin(), fileName);

        for (const auto& str : p.pythonScripts)
        {
            if (set.find(str) == set.end())
            {
                set.insert(str);
                result.push_back(str);
            }
        }

        while (result.size() > kRecentPythonScriptsMax)
        {
            result.pop_back();
        }

        p.pythonScripts = result;
    }
} // namespace mrv
