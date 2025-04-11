# Find the RtAudio library.
#
# This module defines the following variables:
#
# * RtAudio_FOUND
# * RtAudio_INCLUDE_DIRS
# * RtAudio_LIBRARIES
#
# This module defines the following imported targets:
#
# * RtAudio::rtaudio
#
# This module defines the following interfaces:
#
# * RtAudio

find_path(RtAudio_INCLUDE_DIR NAMES rtaudio/RtAudio.h)
set(RtAudio_INCLUDE_DIRS ${RtAudio_INCLUDE_DIR})

find_library(RtAudio_LIBRARY NAMES rtaudio)
set(RtAudio_LIBRARIES ${RtAudio_LIBRARY} ${RtAudio_LINK_LIBRARIES})
if(WIN32)
elseif(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation REQUIRED)
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    list(APPEND RtAudio_LIBRARIES ${AUDIO_TOOLBOX} ${CORE_AUDIO} ${CORE_FOUNDATION})
else()
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
    find_package(ALSA REQUIRED)
    set(RtAudio_RPM_PACKAGE_REQUIRES "alsa-lib")
    set(RtAudio_DEBIAN_PACKAGE_DEPENDS "libasound2")
    find_library(PULSE_LIB pulse)
    find_library(PULSESIMPLE_LIB pulse-simple)
    if(PULSE_LIB AND PULSESIMPLE_LIB)
        string(APPEND RtAudio_RPM_PACKAGE_REQUIRES ", pulseaudio-libs")
        string(APPEND RtAudio_DEBIAN_PACKAGE_DEPENDS ", libpulse0")
        list(APPEND RtAudio_INCLUDE_DIRS ${PULSE_INCLUDE_DIR} ${PULSESIMPLE_INCLUDE_DIR})
        list(APPEND RtAudio_LIBRARIES ${PULSE_LIB} ${PULSESIMPLE_LIB})
    endif()
    find_library(JACK_LIB jack)
    if(JACK_LIB)
        list(APPEND RtAudio_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
        list(APPEND RtAudio_LIBRARIES ${JACK_LIB})
    endif()
    list(APPEND RtAudio_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    RtAudio
    REQUIRED_VARS RtAudio_INCLUDE_DIR RtAudio_LIBRARY)
mark_as_advanced(RtAudio_INCLUDE_DIR RtAudio_LIBRARY)

if(RtAudio_FOUND AND NOT TARGET RtAudio::rtaudio)
    add_library(RtAudio::rtaudio UNKNOWN IMPORTED)
    set(RtAudio_INTERFACE_INCLUDE_DIRECTORIES ${RtAudio_INCLUDE_DIR})
    set(RtAudio_INTERFACE_LINK_LIBRARIES)
    if(WIN32)
    elseif(APPLE)
		list(APPEND RtAudio_INTERFACE_LINK_LIBRARIES ${AUDIO_TOOLBOX} ${CORE_AUDIO} ${CORE_FOUNDATION})
    else()
		list(APPEND RtAudio_INTERFACE_INCLUDE_DIRECTORIES ${ALSA_INCLUDE_DIR})
		list(APPEND RtAudio_INTERFACE_LINK_LIBRARIES ${ALSA_LIBRARY})
        if(PULSE_LIB AND PULSESIMPLE_LIB)
    		list(APPEND RtAudio_INTERFACE_INCLUDE_DIRECTORIES ${PULSE_INCLUDE_DIR} ${PULSESIMPLE_INCLUDE_DIR})
    		list(APPEND RtAudio_INTERFACE_LINK_LIBRARIES ${PULSE_LIB} ${PULSESIMPLE_LIB})
        endif()
        if(JACK_LIB)
    		list(APPEND RtAudio_INTERFACE_INCLUDE_DIRECTORIES ${JACK_INCLUDE_DIR})
    		list(APPEND RtAudio_INTERFACE_LINK_LIBRARIES ${JACK_LIB})
        endif()
		list(APPEND RtAudio_INTERFACE_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    endif()
    set_target_properties(RtAudio::rtaudio PROPERTIES
        IMPORTED_LOCATION "${RtAudio_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS RtAudio_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${RtAudio_INTERFACE_INCLUDE_DIRECTORIES}"
        INTERFACE_LINK_LIBRARIES "${RtAudio_INTERFACE_LINK_LIBRARIES}")
endif()
if(RtAudio_FOUND AND NOT TARGET RtAudio)
    add_library(RtAudio INTERFACE)
    target_link_libraries(RtAudio INTERFACE RtAudio::rtaudio)
endif()
