
#ifdef __linux__
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#ifdef __APPLE__

#define GL_SILENCE_DEPRECATION 1

#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <mach/mach.h>
#include <OpenGL/OpenGL.h>

#endif

#include <tlGlad/gl.h>  // defines glGetString and GL_VERSION

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <zlib.h>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/version.h>
#include <libswscale/version.h>
#include <libswresample/version.h>
}




#ifdef _WIN32
#pragma warning( disable: 4275 )
#endif

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;




#include <ImfVersion.h>

#include <boost/version.hpp>

#include "mrvCore/mrvOS.h"
#undef snprintf

#include "mrvCore/mrvCPU.h"

#include "mrvWidgets/mrvVersion.h"

#include "ImathInt64.h"
#include "mrViewer.h"
#undef snprintf

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#include "mrvFl/mrvIO.h"

namespace mrv
{



  static const char* kVersion = "0.1";
  static const char* kBuild = "- Built " __DATE__ " " __TIME__;

#if INTPTR_MAX == INT64_MAX
static const char* kArch = "64";
#elif INTPTR_MAX == INT32_MAX
static const char* kArch = "32";
#else
#error Unknown pointer size or missing size macros!
#endif


  struct FormatInfo
  {
    bool encode;
    bool decode;
    bool blob;
    std::string name;
    std::string module;
    std::string description;


    FormatInfo(bool dec, bool enc, bool blo,
               const char* n, const char* mod, const char* desc ) :
      encode( enc ),
      decode( dec ),
      blob( blo ),
      name( n ),
      module( mod ),
      description( desc )
    {
    }

    bool operator<( const FormatInfo& b ) const
    {
      return ( strcasecmp( name.c_str(), b.name.c_str() ) < 0 );
    }

  };

  struct SortFormatsFunctor
  {
    bool operator()( const FormatInfo* a, const FormatInfo* b ) const
    {
      return *a < *b;
    }
  };


  typedef std::vector< FormatInfo* > FormatList;


  const char* version()
  {
    return kVersion;
  }

  const char* build_date()
  {
    return kBuild;
  }

void ffmpeg_formats( mrv::Browser& browser )
  {
    using namespace std;


    const AVInputFormat* ifmt = NULL;
    const AVOutputFormat* ofmt = NULL;
    const char* last_name = NULL;


    FormatList formats;
    FormatInfo* f = NULL;

    f = new FormatInfo( true, true, false, "EXR", "tlRender",
                        "ILM OpenEXR" );
    formats.push_back(f);

    f = new FormatInfo( true, true, false, "JPEG", "tlRender",
                        "JPEG" );
    formats.push_back(f);
    
    f = new FormatInfo( true, true, false, "PNG", "tlRender",
                        "PNG" );
    formats.push_back(f);
    
    f = new FormatInfo( true, true, false, "TIFF", "tlRender",
                        "TIFF" );
    formats.push_back(f);
    
    f = new FormatInfo( true, true, false, "Cineon", "tlRender",
                        "Cineon" );
    formats.push_back(f);


    last_name= "000";
    for(;;){
      bool decode = false;
      bool encode = false;
      const char* name=NULL;
      const char* long_name=NULL;

      void* opaque = NULL;
      while ((ofmt = av_muxer_iterate(&opaque))) {
        if((name == NULL || strcmp(ofmt->name, name)<0) &&
           strcmp(ofmt->name, last_name)>0){
          name = ofmt->name;
          long_name = ofmt->long_name;
          encode = true;
        }
      }
      opaque = NULL;
      while ((ifmt = av_demuxer_iterate(&opaque))) {
        if((name == NULL || strcmp(ifmt->name, name)<0) &&
           strcmp(ifmt->name, last_name)>0){
          name = ifmt->name;
          long_name = ifmt->long_name;
          encode = true;
        }
        if(name && strcmp(ifmt->name, name)==0)
          decode = true;
      }

      if(name==NULL)
        break;
      last_name= name;

      f = new FormatInfo( decode, encode, false, name, "FFMPEG", long_name );
      formats.push_back( f );
    }

    // Sort formats alphabetically
    std::sort( formats.begin(), formats.end(), SortFormatsFunctor() );

    // Now concatenate all the stuff into a string
    {
      FormatList::const_iterator i = formats.begin();
      FormatList::const_iterator e = formats.end();
      for ( ; i != e; ++i )
        {
          f = *i;
          std::ostringstream o;
          o << ( f->decode ? "D\t" : " \t" )
            << ( f->encode ? "E\t" : " \t" )
            << ( f->blob   ? "B\t" : " \t" )
            << f->name << "\t"
            << f->module << "\t"
            << f->description;
          browser.add( o.str().c_str() );
          delete f;
        }
    }

  }

static void ffmpeg_codecs(mrv::Browser& browser, int type)
  {
    using namespace std;

    const AVCodec *p;
    const AVCodec *p2;
    const char* last_name;

    std::ostringstream o;
    last_name= "000";
    for(;;){
      int decode=0;
      int encode=0;
      int cap=0;

      p2=NULL;
      void* opaque = NULL;
      while((p = av_codec_iterate(&opaque))) {
        if((p2==NULL || strcmp(p->name, p2->name)<0) &&
           strcmp(p->name, last_name)>0){
          p2= p;
          decode= encode= cap=0;
        }
        if(p2 && strcmp(p->name, p2->name)==0){
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            encode = av_codec_is_encoder( p );
            decode = av_codec_is_decoder( p );
#else
          if(p->decode) decode=1;
          if(p->encode2) encode=1;
#endif
          cap |= p->capabilities;
        }
      }

      if(p2==NULL)
        break;
      last_name= p2->name;

      if ( p2->type != type )
        continue;

      std::ostringstream o;
      o << ( decode ? "D\t" : " \t" )
        << ( encode ? "E\t" : " \t" )
        << "\t"
        << ( cap & AV_CODEC_CAP_DRAW_HORIZ_BAND ? "S\t":" \t" )
        << ( cap & AV_CODEC_CAP_DR1 ? "D\t":" \t" )
        << ( cap & AV_CODEC_CAP_TRUNCATED ? "T\t":" \t" )
        << p2->name;

      browser.add( o.str().c_str() );

      /* if(p2->decoder && decode==0)
         printf(" use %s for decoding", p2->decoder->name);*/
    }

  }


  void ffmpeg_audio_codecs(mrv::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_AUDIO );
  }

  void ffmpeg_video_codecs(mrv::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_VIDEO );
  }

  void ffmpeg_subtitle_codecs(mrv::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_SUBTITLE );
  }


  std::string ffmpeg_protocols()
  {
    std::ostringstream o;
#if LIBAVUTIL_VERSION_MAJOR > 50
    void* opaque = NULL;
    const char* up;
    for( up = avio_enum_protocols( &opaque, 0 ); up;
         up = avio_enum_protocols( &opaque, 0 ) )
      {
        o << " " << up << ":";
      }
#else
    URLProtocol* up;
    for(up = av_protocol_next(NULL); up; up = av_protocol_next(up) )
     {
        o << " " << up->name << ":";
     }
#endif
    return o.str();
  }

    void ffmpeg_motion_estimation_methods( mrv::Browser* b )
  {
    static const char *motion_str[] = {
      "zero",
      "esa",
      "tss",
      "tdls",
      "ntss",
      "fss",
      "ds",
      "hexds",
      "epzs",
      "umh",
      NULL,
    };

    const char** pp = motion_str;
    while (*pp) {
        b->add( *pp );
        pp++;
    }
  }

  //
  // Redirects ffmpeg's av_log messages to mrViewer's log window.
  //
  void av_log_redirect( void* ptr, int level, const char* fmt, va_list vl )
  {
    static const char* kModule = "ffmpeg";

    char buf[1024];  buf[1023] = 0;
    int c = vsnprintf( buf, 1023, fmt, vl );

    if ( buf[c-1] != '\n' )
        {
            buf[c] = '\n';
            buf[c+1] = '\0';
        }

    if ( level < AV_LOG_WARNING )
      mrvLOG_ERROR( kModule, buf );
    else if ( level < AV_LOG_INFO )
      mrvLOG_WARNING( kModule, buf );
    else if ( level < AV_LOG_VERBOSE )
      mrvLOG_INFO( kModule, buf );
    else {
      // do NOT log verbose
        // mrvLOG_INFO( kModule, buf );
    }
  }


  std::string about_message()
  {
    using namespace std;


// #ifdef DEBUG
//     av_log_set_level(99);
// #else
    av_log_set_level(-99);
// #endif

    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    //av_log_set_callback( mrv::av_log_redirect );


    avformat_network_init();



    std::ostringstream o;

    unsigned int boost_major = BOOST_VERSION / 100000;
    unsigned int boost_minor = BOOST_VERSION / 100 % 1000;
    unsigned int boost_teeny = BOOST_VERSION % 100;

    o << "mrViewerII " << kArch << " bits - v" << kVersion << " "
      << kBuild << endl
#ifdef __GLIBCXX__
      << "With gcc " << __GLIBCXX__ << endl
#elif __clang__
      << "With clang " << __clang__ << " " << __llvm__ << endl
#else
      << "With msvc " << _MSC_VER << endl
#endif
      << "(C) 2007-2023 Film Aura, LLC." << endl;

    o << endl
      << "mrViewer depends on:" << endl
      << endl
//      << "OpenGL " << glGetString( GL_VERSION ) << endl
      << endl
      << "Boost v" << boost_major << "."
      << boost_minor << "." << boost_teeny << endl
      << "http://www.boost.org/" << endl;

    o  << endl
      << "FLTK 1.4" << endl
      << "http://www.fltk.org/" << endl
      << "(C) 2000-2022 Bill Spitzak & others" << endl
      << endl
      << "FFmpeg" << endl
      << "libavutil          v" << AV_STRINGIFY( LIBAVUTIL_VERSION ) << endl
      << "libavcodec      v" << AV_STRINGIFY( LIBAVCODEC_VERSION ) << endl
      << "libavformat     v" << AV_STRINGIFY( LIBAVFORMAT_VERSION ) << endl
      << "libavfilter        v" << AV_STRINGIFY( LIBAVFILTER_VERSION ) << endl
      << "libswresample v" << AV_STRINGIFY( LIBSWRESAMPLE_VERSION ) << endl
      << "libswscale       v" << AV_STRINGIFY( LIBSWSCALE_VERSION ) << endl
      << "http://ffmpeg.mplayerhq.hu/" << endl
      << "License: " << avcodec_license() << endl
      << "(C) 2000-2022 Fabrice Bellard, et al." << endl
      << "Configuration: " << avcodec_configuration() << endl
      << endl
      << "ILM OpenEXR v" << OPENEXR_VERSION_STRING << endl
      << "http://www.openexr.org/" << endl
      << "(C) 2005-2022 Industrial Light & Magic" << endl
      << endl
      << "OpenColorIO v" << OCIO::GetVersion() << endl
      << "http://www.opencolorio.org/" << endl
      << "(C) 2005-2022 Sony Pictures Imageworks" << endl
      << endl;

    o << endl
      << "AMPAS ACES v1.0 or later" << endl
      << "https://github.com/ampas/aces-dev" << endl
      << "(C) 2019-2022 AMPAS" << endl
      << endl
      << "zlib v" << ZLIB_VERSION
      << "(C) 2008-2022 Jean-loup Gailly and Mark Adler"
      << endl
      << endl
      << endl
      << "A big thank you goes to Greg Ercolano who helped with" << endl
      << "the fltk1.4 porting and with the color schemes.";




    return o.str();
  }

  std::string cpu_information()
  {
    return GetCpuCaps(&gCpuCaps);
  }

#ifdef _WIN32
void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe)
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    totalVirtualMem = memInfo.ullTotalPageFile;
    virtualMemUsed = totalVirtualMem - memInfo.ullAvailPageFile;
    totalVirtualMem /= (1024*1024);
    virtualMemUsed /= (1024*1024);
    totalPhysMem = memInfo.ullTotalPhys;
    physMemUsed = totalPhysMem - memInfo.ullAvailPhys;
    totalPhysMem /= (1024*1024);
    physMemUsed /= (1024*1024);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(),
                         (PPROCESS_MEMORY_COUNTERS) &pmc, sizeof(pmc));
    virtualMemUsedByMe = pmc.PrivateUsage;
    virtualMemUsedByMe /= (1024*1024);
    physMemUsedByMe = pmc.WorkingSetSize;
    physMemUsedByMe /= (1024*1024);
}
#endif

#ifdef __linux__

static int parseLine(char* line){
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
}


static int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int getValue2(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe)
{

    struct sysinfo memInfo;
    sysinfo (&memInfo);

    totalVirtualMem = memInfo.totalram;
    // Add other values in next statement to avoid int overflow
    // on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    totalVirtualMem /= (1024*1024);

    virtualMemUsed = memInfo.totalram - memInfo.freeram;
    // Add other values in next statement to avoid int overflow on
    // right hand side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;
    virtualMemUsed /= (1024*1024);

    totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    totalPhysMem /= (1024*1024);

    physMemUsed = memInfo.totalram - memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    physMemUsed /= (1024*1024);

    virtualMemUsedByMe = getValue();
    virtualMemUsedByMe /= 1024;

    physMemUsedByMe = getValue2();
    physMemUsedByMe /= 1024;

}  // memory_information
#elif __APPLE__

void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe)
{
    static const char* kModule = "mem";

    //
    //  Total Virtual Memory
    //
    struct statfs stats;
    totalVirtualMem = 0;
    if (0 == statfs("/", &stats))
    {
            totalVirtualMem = (uint64_t)stats.f_bsize * stats.f_bfree;
            totalVirtualMem /= (1024 * 1024);
    }

    //
    //  Total Physical Memory
    //
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    u_int namelen = sizeof(mib) / sizeof(mib[0]);
    size_t len = sizeof(totalPhysMem);

    if (sysctl(mib, namelen, &totalPhysMem, &len, NULL, 0) < 0)
    {
        LOG_ERROR( _("sysctl failed!") );
    }

    totalPhysMem /= (1024 * 1024);

    //
    // Physical Memory Used
    //
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS != host_page_size(mach_port, &page_size) ||
        KERN_SUCCESS != host_statistics64(mach_port, HOST_VM_INFO,
                                        (host_info64_t)&vm_stats, &count))
    {
            LOG_ERROR( _("host_statistics64 failed") );
    }

    int64_t active = (int64_t) vm_stats.active_count;
    int64_t inactive = (int64_t) vm_stats.inactive_count;
    int64_t wired = (int64_t) vm_stats.wire_count;
    physMemUsed = (active + inactive + wired) * (int64_t) page_size;
    physMemUsed /= 1024 * 1024;

    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t)&t_info,
                                  &t_info_count))
    {
            LOG_ERROR( _("task info failed") );
    }

    physMemUsedByMe = t_info.resident_size;
    physMemUsedByMe /= 1024*1024;


    virtualMemUsedByMe = t_info.virtual_size;
    virtualMemUsedByMe /= 1024*1024;

}  // memory_information

#endif // LINUX

  std::string gpu_information( ViewerUI* ui )
  {
    using std::endl;
    std::ostringstream o;

    int num_monitors = Fl::screen_count();;
    o << "Monitors:\t" << num_monitors << endl;


    o << "HW Stereo:\t"
      << ( ui->uiView->can_do( FL_STEREO ) ? "Yes" : "No" )
      << endl
      << "HW Overlay:\t"
      << ( ui->uiView->can_do_overlay() ? "Yes" : "No" )
      << endl;

    return o.str();
  }


}
