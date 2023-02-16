// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Enumerations.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <tlCore/StringFormat.h>
#include <tlIO/IOSystem.h>

#include <boost/filesystem.hpp>
#include <set>
#include <string>

#include "mrViewer.h"
#include "mrvApp/App.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvString.h"
#include "mrvFLU/Flu_File_Chooser.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvSaving.h"
#include "mrvHotkeyUI.h"

namespace fs = boost::filesystem;

namespace {

const char *kModule = "filereq";

#ifdef _WIN32
#define kSeparator ";"
#else
#define kSeparator ":"
#endif

// File extension patterns

static const std::string kReelPattern = "otio";

std::string
getMoviePattern(const std::shared_ptr<tl::system::Context> &context) {
  std::string result;
  const auto ioSystem = context->getSystem<io::System>();
  const auto plugins = ioSystem->getPlugins();
  for (auto plugin : plugins) {
    auto extensions =
        plugin->getExtensions(static_cast<int>(io::FileType::Movie));
    for (auto &extension : extensions) {
      if (!result.empty())
        result += ',';
      result += extension.substr(1, extension.size());
    }
  }
  return result;
}

std::string
getImagePattern(const std::shared_ptr<tl::system::Context> &context) {
  std::string result;
  const auto ioSystem = context->getSystem<io::System>();
  const auto plugins = ioSystem->getPlugins();
  for (auto plugin : plugins) {
    auto extensions =
        plugin->getExtensions(static_cast<int>(io::FileType::Sequence));
    for (auto &extension : extensions) {
      if (!result.empty())
        result += ',';
      result += extension.substr(1, extension.size());
    }
  }
  return result;
}

std::string
getAudioPattern(const std::shared_ptr<tl::system::Context> &context) {
  std::string result;
  const auto ioSystem = context->getSystem<io::System>();
  const auto plugins = ioSystem->getPlugins();
  for (auto plugin : plugins) {
    auto extensions =
        plugin->getExtensions(static_cast<int>(io::FileType::Audio));
    for (auto &extension : extensions) {
      if (!result.empty())
        result += ',';
      result += extension.substr(1, extension.size());
    }
  }
  return result;
}

static const std::string kSubtitlePattern = "srt,sub,ass,vtt";

static const std::string kOCIOPattern = "ocio";

} // namespace

namespace mrv {

// WINDOWS crashes if pattern is sent as is.  We need to sanitize the
// pattern here.  From:
// All (*.{png,ogg})\t
// to:
// All\t{*.png,ogg}\n
std::string pattern_to_native(std::string pattern) {
  std::string ret;
  size_t pos = 0, pos2 = 0;
  while (pos != std::string::npos) {
    pos = pattern.find(' ');
    if (pos == std::string::npos)
      break;
    ret += pattern.substr(0, pos) + '\t';
    size_t pos3 = pattern.find('(', pos);
    size_t pos2 = pattern.find(')', pos);
    ret += pattern.substr(pos3 + 1, pos2 - pos3 - 1) + '\n';
    if (pos2 + 2 < pattern.size())
      pattern = pattern.substr(pos2 + 2, pattern.size());
    else
      pattern.clear();
  }
  return ret;
}

const std::string
file_save_single_requester(const std::shared_ptr<tl::system::Context> &context,
                           const char *title, const char *pattern,
                           const char *startfile,
                           const bool compact_images = true) {
  std::string file;
  try {
    bool native = mrv::Preferences::native_file_chooser;
    if (native) {
      Fl_Native_File_Chooser native;
      native.title(title);
      native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
      std::string native_pattern = pattern_to_native(pattern);
      native.filter(native_pattern.c_str());
      native.preset_file(startfile);
      // Show native chooser
      switch (native.show()) {
      case -1:
        break; // ERROR
      case 1:
        break; // CANCEL
      default: // PICKED FILE
        if (native.filename()) {
          file = native.filename();
        }
      }
    } else {
      const char *f =
          flu_save_chooser(context, title, pattern, startfile, compact_images);
      if (!f) {
        return "";
      }
      file = f;
    }
  } catch (const std::exception &e) {
  }

  return file;
}

stringArray
file_multi_requester(const std::shared_ptr<tl::system::Context> &context,
                     const char *title, const char *pattern,
                     const char *startfile, const bool compact_images) {
  stringArray filelist;

  try {
    if (!startfile)
      startfile = "";
    bool native = mrv::Preferences::native_file_chooser;
    if (native) {
      Fl_Native_File_Chooser native;
      native.title(title);
      native.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
      std::string native_pattern = pattern_to_native(pattern);
      native.filter(native_pattern.c_str());
      native.preset_file(startfile);
      // Show native chooser
      switch (native.show()) {
      case -1:
        break; // ERROR
      case 1:
        break; // CANCEL
      default: // PICKED FILE
        if (native.count() > 0) {
          for (int i = 0; i < native.count(); ++i) {
            filelist.push_back(native.filename(i));
          }
        }
      }
    } else {
      flu_multi_file_chooser(context, title, pattern, startfile, filelist,
                             compact_images);
    }
  } catch (const std::exception &e) {
  } catch (...) {
  }
  return filelist;
}

std::string
file_single_requester(const std::shared_ptr<tl::system::Context> &context,
                      const char *title, const char *pattern,
                      const char *startfile, const bool compact_files) {
  std::string file;
  try {
    if (!startfile)
      startfile = "";
    bool native = mrv::Preferences::native_file_chooser;
    if (native) {
      Fl_Native_File_Chooser native;
      native.title(title);
      native.type(Fl_Native_File_Chooser::BROWSE_FILE);
      std::string native_pattern = pattern_to_native(pattern);
      native.filter(native_pattern.c_str());
      native.preset_file(startfile);
      // Show native chooser
      switch (native.show()) {
      case -1:
        break; // ERROR
      case 1:
        break; // CANCEL
      default: // PICKED FILE
        if (native.count() > 0) {
          file = native.filename();
        }
      }
    } else {
      const char *f =
          flu_file_chooser(context, title, pattern, startfile, compact_files);
      if (f)
        file = f;
    }
  } catch (const std::exception &e) {
  } catch (...) {
  }

  return file;
}

/**
 * Opens a file requester to load a directory of image files
 *
 * @param startfile  start filename (directory)
 *
 * @return A directory to be opened or NULL
 */
std::string open_directory(const char *startfile, ViewerUI *ui) {
  std::string dir;
  std::string title = _("Load Directory");
  bool native = mrv::Preferences::native_file_chooser;
  if (native) {
    Fl_Native_File_Chooser native;
    native.title(title.c_str());
    native.directory(startfile);
    native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    // Show native chooser
    switch (native.show()) {
    case -1:
      break; // ERROR
    case 1:
      break; // CANCEL
    default: // PICKED DIR
      if (native.filename()) {
        dir = native.filename();
      }
      break;
    }
  } else {
    const auto &context = ui->app->getContext();
    const char *d = flu_dir_chooser(context, title.c_str(), startfile);
    if (d)
      dir = d;
  }
  fs::path path = dir;
  return path.generic_string();
}

/**
 * Opens a file requester to load image files
 *
 * @param startfile  start filename (directory)
 *
 * @return Each file to be opened
 */
stringArray open_image_file(const char *startfile, const bool compact_images,
                            ViewerUI *ui) {
  const auto &context = ui->app->getContext();
  const std::string kREEL_PATTERN = _("Reels (*.{") + kReelPattern + "})\t";
  const std::string kAUDIO_PATTERN =
      ("Audios (*.{") + getAudioPattern(context) + "})\t";
  const std::string kIMAGE_PATTERN =
      _("Images (*.{") + getImagePattern(context) + "})\t";
  const std::string kALL_PATTERN =
      _("All (*.{") + getImagePattern(context) + "," +
      getMoviePattern(context) + "," + kReelPattern + "," +
      getAudioPattern(context) + "," + kReelPattern + "})\t" + kIMAGE_PATTERN +
      kAUDIO_PATTERN + _("Movies (*.{") + getMoviePattern(context) + "})\t" +
      kREEL_PATTERN;

  std::string pattern = kIMAGE_PATTERN + kAUDIO_PATTERN;
  std::string title = _("Load Image");
  if (compact_images) {
    title = _("Load Movie or Sequence");
    pattern = kALL_PATTERN;
  }

  return file_multi_requester(context, title.c_str(), pattern.c_str(),
                              startfile, compact_images);
}

/**
 * Opens a file requester to load a lut file.
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened lut file or empty
 */
std::string open_lut_file(const char *startfile, ViewerUI *ui) {
  std::string lut_pattern;
  for (const auto &i : tl::timeline::getLUTFormatExtensions()) {
    if (!lut_pattern.empty())
      lut_pattern += ",";
    lut_pattern += i.substr(1, i.size()); // no period
  }
  std::string kLUT_PATTERN = _("LUTS (*.{") + lut_pattern + "})\t";

  std::string title = _("Load LUT");

  const auto &context = ui->app->getContext();
  return file_single_requester(context, title.c_str(), kLUT_PATTERN.c_str(),
                               startfile, false);
}

/**
 * Opens a file requester to load subtitle files
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened subtitle file or empty
 */
std::string open_subtitle_file(const char *startfile, ViewerUI *ui) {
  std::string kSUBTITLE_PATTERN =
      _("Subtitles (*.{") + kSubtitlePattern + "})\n";

  std::string title = _("Load Subtitle");

  const auto &context = ui->app->getContext();
  return file_single_requester(context, title.c_str(),
                               kSUBTITLE_PATTERN.c_str(), startfile, false);
}

/**
 * Opens a file requester to load audio files
 *
 * @param startfile  start filename (directory)
 *
 * @return  opened audio file or null
 */
std::string open_audio_file(const char *startfile, ViewerUI *ui) {
  const auto &context = ui->app->getContext();
  std::string kAUDIO_PATTERN =
      _("Audios (*.{") + getAudioPattern(context) + "})";

  std::string title = _("Load Audio");

  return file_single_requester(context, title.c_str(), kAUDIO_PATTERN.c_str(),
                               startfile, true);
}

void save_sequence_file(ViewerUI *ui, const char *startdir) {
  const auto &context = ui->app->getContext();
  const std::string kIMAGE_PATTERN =
      _("Images (*.{") + getImagePattern(context) + "})\t";
  const std::string kALL_PATTERN = _("All (*.{") + getImagePattern(context) +
                                   "," + getMoviePattern(context) + "})\t" +
                                   kIMAGE_PATTERN + _("Movies (*.{") +
                                   getMoviePattern(context) + "})\t";

  std::string title = _("Save Sequence");

  stringArray filelist;
  if (!startdir)
    startdir = "";

  const std::string file = file_save_single_requester(
      context, title.c_str(), kALL_PATTERN.c_str(), startdir, true);
  if (file.empty())
    return;

  // save_movie_or_sequence( file.c_str(), ui, opengl );
}

void save_movie_file(ViewerUI *ui, const char *startdir) {
  const auto &context = ui->app->getContext();
  const std::string kMOVIE_PATTERN =
      _("Movies (*.{") + getMoviePattern(context) + "})";

  std::string title = _("Save Movie");

  if (!startdir)
    startdir = "";

  const std::string file = file_save_single_requester(
      context, title.c_str(), kMOVIE_PATTERN.c_str(), startdir, true);
  if (file.empty())
    return;

  save_movie(file, ui);
}

std::string open_ocio_config(const char *startfile, ViewerUI *ui) {
  std::string kOCIO_PATTERN = _("OCIO config (*.{") + kOCIOPattern + "})";
  std::string title = _("Load OCIO Config");

  const auto &context = ui->app->getContext();
  std::string file = file_single_requester(
      context, title.c_str(), kOCIO_PATTERN.c_str(), startfile, false);
  return file;
}

void save_hotkeys(Fl_Preferences &keys) {
  keys.set("version", 11);
  for (int i = 0; hotkeys[i].name != "END"; ++i) {
    keys.set((hotkeys[i].name + " ctrl").c_str(), hotkeys[i].hotkey.ctrl);
    keys.set((hotkeys[i].name + " alt").c_str(), hotkeys[i].hotkey.alt);
    keys.set((hotkeys[i].name + " meta").c_str(), hotkeys[i].hotkey.meta);
    keys.set((hotkeys[i].name + " shift").c_str(), hotkeys[i].hotkey.shift);
    keys.set((hotkeys[i].name + " key").c_str(), (int)hotkeys[i].hotkey.key);
    keys.set((hotkeys[i].name + " text").c_str(),
             hotkeys[i].hotkey.text.c_str());
  }
}

void save_hotkeys(ViewerUI *ui, std::string filename) {
  const auto &context = ui->app->getContext();
  //
  // Hotkeys
  //
  size_t pos = filename.rfind(".prefs");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  pos = filename.rfind(".keys");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  std::string path = prefspath() + filename + ".keys.prefs";
  std::string title = _("Save Hotkeys");
  filename = file_save_single_requester(
      context, title.c_str(), _("Hotkeys (*.{keys.prefs})"), path.c_str());
  if (filename.empty())
    return;

  size_t start = filename.rfind('/');
  if (start == std::string::npos)
    start = filename.rfind('\\');
  ;
  if (start != std::string::npos)
    filename = filename.substr(start + 1, filename.size());

  pos = filename.rfind(".prefs");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  pos = filename.rfind(".keys");
  if (pos == std::string::npos)
    filename += ".keys";

  Preferences::hotkeys_file = filename;
  Fl_Preferences keys(prefspath().c_str(), "filmaura", filename.c_str());
  save_hotkeys(keys);

  HotkeyUI *h = ui->uiHotkey;
  h->uiHotkeyFile->value(mrv::Preferences::hotkeys_file.c_str());

  // Update menubar in case some hotkey shortcut changed
  ui->uiMain->fill_menu(ui->uiMenuBar);
}

void load_hotkeys(ViewerUI *ui, std::string filename) {
  const auto &context = ui->app->getContext();
  size_t pos = filename.rfind(".prefs");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  pos = filename.rfind(".keys");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  std::string path = prefspath() + filename + ".keys.prefs";
  std::string title = _("Load Hotkeys Preferences");
  filename =
      file_single_requester(context, title.c_str(),
                            _("Hotkeys (*.{keys.prefs})"), path.c_str(), false);
  if (filename.empty())
    return;

  size_t start = filename.rfind('/');
  if (start == std::string::npos)
    start = filename.rfind('\\');
  if (start != std::string::npos)
    filename = filename.substr(start + 1, filename.size());

  if (!fs::exists(prefspath() + '/' + filename)) {
    std::string err =
        tl::string::Format(_("Hotkeys file {0}/{1} does not exist!"))
            .arg(prefspath())
            .arg(filename);
    LOG_ERROR(err);
    return;
  }

  pos = filename.rfind(".prefs");
  if (pos != std::string::npos)
    filename = filename.replace(pos, filename.size(), "");

  Preferences::hotkeys_file = filename;

  Fl_Preferences *keys =
      new Fl_Preferences(prefspath().c_str(), "filmaura", filename.c_str());
  load_hotkeys(ui, keys);
}

void load_hotkeys(ViewerUI *ui, Fl_Preferences *keys) {
  int version = 0;
  keys->get("version", version, 11);
  int tmp = 0;
  char tmpS[2048];

  if (fs::exists(prefspath() + Preferences::hotkeys_file + ".prefs")) {
    for (int i = 0; hotkeys[i].name != "END"; ++i) {
      if (hotkeys[i].force == true)
        continue;
      hotkeys[i].hotkey.shift = hotkeys[i].hotkey.ctrl = hotkeys[i].hotkey.alt =
          hotkeys[i].hotkey.meta = false;
      hotkeys[i].hotkey.key = 0;
      hotkeys[i].hotkey.text.clear();
    }
  }

  for (int i = 0; hotkeys[i].name != "END"; ++i) {
    // If version >= 1 of preferences, do not set scrub

    Hotkey saved = hotkeys[i].hotkey;

    keys->get((hotkeys[i].name + " key").c_str(), tmp,
              (int)hotkeys[i].hotkey.key);
    if (tmp)
      hotkeys[i].force = false;
    hotkeys[i].hotkey.key = unsigned(tmp);

    keys->get((hotkeys[i].name + " text").c_str(), tmpS,
              hotkeys[i].hotkey.text.c_str(), 16);
    if (strlen(tmpS) > 0) {
      hotkeys[i].force = false;
      hotkeys[i].hotkey.text = tmpS;
    } else
      hotkeys[i].hotkey.text.clear();

    if (hotkeys[i].force) {
      hotkeys[i].hotkey = saved;
      continue;
    }
    keys->get((hotkeys[i].name + " ctrl").c_str(), tmp,
              (int)hotkeys[i].hotkey.ctrl);
    if (tmp)
      hotkeys[i].hotkey.ctrl = true;
    else
      hotkeys[i].hotkey.ctrl = false;
    keys->get((hotkeys[i].name + " alt").c_str(), tmp,
              (int)hotkeys[i].hotkey.alt);
    if (tmp)
      hotkeys[i].hotkey.alt = true;
    else
      hotkeys[i].hotkey.alt = false;

    keys->get((hotkeys[i].name + " meta").c_str(), tmp,
              (int)hotkeys[i].hotkey.meta);
    if (tmp)
      hotkeys[i].hotkey.meta = true;
    else
      hotkeys[i].hotkey.meta = false;

    keys->get((hotkeys[i].name + " shift").c_str(), tmp,
              (int)hotkeys[i].hotkey.shift);
    if (tmp)
      hotkeys[i].hotkey.shift = true;
    else
      hotkeys[i].hotkey.shift = false;

    for (int j = 0; hotkeys[j].name != "END"; ++j) {
      if (j != i && hotkeys[j].hotkey == hotkeys[i].hotkey) {
        std::string err =
            tl::string::Format(_("Corruption in hotkeys preferences. "
                                 "Hotkey {0} for {1} will not be available.  "
                                 "Already used in {2}"))
                .arg(hotkeys[j].hotkey.to_s())
                .arg(_(hotkeys[j].name.c_str()))
                .arg(_(hotkeys[i].name.c_str()));
        LOG_ERROR(err);
        hotkeys[j].hotkey = Hotkey();
      }
    }
  }

  HotkeyUI *h = ui->uiHotkey;
  h->uiHotkeyFile->value(mrv::Preferences::hotkeys_file.c_str());
  fill_ui_hotkeys(h->uiFunction);
}
} // namespace mrv
