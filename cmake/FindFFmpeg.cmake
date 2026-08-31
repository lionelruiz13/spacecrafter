# FindFFmpeg
# --------
# Finds FFmpeg libraries
#
# This module will first look for the required library versions on the system.
# If they are not found, it will emit a fatal error with install instructions.
#
# --------
# the following variables influence behaviour:
# FFmpeg_PATH - use external ffmpeg not found in system paths
#               usage: -DFFmpeg_PATH=/path/to/ffmpeg_install_prefix
#
# WITH_FFmpeg - use external ffmpeg not found in system paths
#               WARNING: this option is for developers as it will _disable ffmpeg version checks_!
#               Consider using FFmpeg_PATH instead, which _does_ check library versions
#               usage: -DWITH_FFmpeg=/path/to/ffmpeg_install_prefix
#
# --------
# This module will will define the following variables:
#
# FFmpeg_FOUND - system has FFmpeg
# FFmpeg_INCLUDE_DIRS - FFmpeg include directory
# FFmpeg_LIBRARIES - FFmpeg libraries
# FFmpeg_DEFINITIONS - pre-processor definitions
# FFmpeg_LDFLAGS - linker flags
#
# and the following imported targets::
#
# ffmpeg  - The FFmpeg libraries
# --------
#

# required ffmpeg library versions
set(REQUIRED_FFmpeg_VERSION 3.3)
set(_avcodec_ver ">=57.89.100")
set(_avfilter_ver ">=6.82.100")
set(_avformat_ver ">=57.71.100")
set(_avutil_ver ">=55.58.100")
set(_swscale_ver ">=4.6.100")
set(_swresample_ver ">=2.7.100")


# Allows building with external ffmpeg not found in system paths,
# without library version checks
if(WITH_FFmpeg)
  set(FFmpeg_PATH ${WITH_FFmpeg})
  message(STATUS "Warning: FFmpeg version checking disabled")
  set(REQUIRED_FFmpeg_VERSION undef)
  unset(_avcodec_ver)
  unset(_avfilter_ver)
  unset(_avformat_ver)
  unset(_avutil_ver)
  unset(_swscale_ver)
  unset(_swresample_ver)
endif()

# Allows building with external ffmpeg not found in system paths,
# with library version checks
if(FFmpeg_PATH)
  set(ENV{PKG_CONFIG_PATH} "${FFmpeg_PATH}/lib/pkgconfig")
  list(APPEND CMAKE_PREFIX_PATH ${FFmpeg_PATH})
endif()

set(FFmpeg_PKGS libavcodec${_avcodec_ver}
                  libavfilter${_avfilter_ver}
                  libavformat${_avformat_ver}
                  libavutil${_avutil_ver}
                  libswscale${_swscale_ver}
                  libswresample${_swresample_ver})

  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_FFmpeg ${FFmpeg_PKGS} QUIET)
    string(REGEX REPLACE "framework;" "framework " PC_FFmpeg_LDFLAGS "${PC_FFmpeg_LDFLAGS}")
  endif()

  find_path(FFmpeg_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                                libavutil/avutil.h libswscale/swscale.h
            PATH_SUFFIXES ffmpeg
            PATHS ${PC_FFmpeg_INCLUDE_DIRS}
            NO_DEFAULT_PATH)
  find_path(FFmpeg_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                                libavutil/avutil.h libswscale/swscale.h)

  find_library(FFmpeg_LIBAVCODEC
               NAMES avcodec libavcodec
               PATH_SUFFIXES ffmpeg/libavcodec
               PATHS ${PC_FFmpeg_libavcodec_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBAVCODEC NAMES avcodec libavcodec PATH_SUFFIXES ffmpeg/libavcodec)

  find_library(FFmpeg_LIBAVFILTER
               NAMES avfilter libavfilter
               PATH_SUFFIXES ffmpeg/libavfilter
               PATHS ${PC_FFmpeg_libavfilter_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBAVFILTER NAMES avfilter libavfilter PATH_SUFFIXES ffmpeg/libavfilter)

  find_library(FFmpeg_LIBAVFORMAT
               NAMES avformat libavformat
               PATH_SUFFIXES ffmpeg/libavformat
               PATHS ${PC_FFmpeg_libavformat_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBAVFORMAT NAMES avformat libavformat PATH_SUFFIXES ffmpeg/libavformat)

  find_library(FFmpeg_LIBAVUTIL
               NAMES avutil libavutil
               PATH_SUFFIXES ffmpeg/libavutil
               PATHS ${PC_FFmpeg_libavutil_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBAVUTIL NAMES avutil libavutil PATH_SUFFIXES ffmpeg/libavutil)

  find_library(FFmpeg_LIBSWSCALE
               NAMES swscale libswscale
               PATH_SUFFIXES ffmpeg/libswscale
               PATHS ${PC_FFmpeg_libswscale_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBSWSCALE NAMES swscale libswscale PATH_SUFFIXES ffmpeg/libswscale)

  find_library(FFmpeg_LIBSWRESAMPLE
               NAMES swresample libswresample
               PATH_SUFFIXES ffmpeg/libswresample
               PATHS ${PC_FFmpeg_libswresample_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBSWRESAMPLE NAMES NAMES swresample libswresample PATH_SUFFIXES ffmpeg/libswresample)

  # libpostproc is optional: present in FFmpeg <=7, removed in FFmpeg 8
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_FFmpeg_POSTPROC libpostproc QUIET)
  endif()
  find_library(FFmpeg_LIBPOSTPROC
               NAMES postproc libpostproc
               PATH_SUFFIXES ffmpeg/libpostproc
               PATHS ${PC_FFmpeg_POSTPROC_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBPOSTPROC NAMES postproc libpostproc PATH_SUFFIXES ffmpeg/libpostproc)
  if(FFmpeg_LIBPOSTPROC)
    message(STATUS "FFmpeg: libpostproc found, enabling")
  else()
    message(STATUS "FFmpeg: libpostproc not found (FFmpeg 8+), building without it")
  endif()

  if((PC_FFmpeg_FOUND
      AND PC_FFmpeg_libavcodec_VERSION
      AND PC_FFmpeg_libavfilter_VERSION
      AND PC_FFmpeg_libavformat_VERSION
      AND PC_FFmpeg_libavutil_VERSION
      AND PC_FFmpeg_libswscale_VERSION
      AND PC_FFmpeg_libswresample_VERSION)
     OR WIN32)
    set(FFmpeg_VERSION ${REQUIRED_FFmpeg_VERSION})


    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(FFmpeg
                                      VERSION_VAR FFmpeg_VERSION
                                      REQUIRED_VARS FFmpeg_INCLUDE_DIRS
                                                    FFmpeg_LIBAVCODEC
                                                    FFmpeg_LIBAVFILTER
                                                    FFmpeg_LIBAVFORMAT
                                                    FFmpeg_LIBAVUTIL
                                                    FFmpeg_LIBSWSCALE
                                                    FFmpeg_LIBSWRESAMPLE
                                                    FFmpeg_VERSION
                                      FAIL_MESSAGE "FFmpeg ${REQUIRED_FFmpeg_VERSION} not found, install system packages or use -DFFmpeg_PATH")

  else()
    message(STATUS "FFmpeg ${REQUIRED_FFmpeg_VERSION} not found via pkg-config")
    unset(FFmpeg_INCLUDE_DIRS)
    unset(FFmpeg_INCLUDE_DIRS CACHE)
    unset(FFmpeg_LIBRARIES)
    unset(FFmpeg_LIBRARIES CACHE)
    unset(FFmpeg_DEFINITIONS)
    unset(FFmpeg_DEFINITIONS CACHE)
  endif()

  if(FFmpeg_FOUND)
    set(FFmpeg_LDFLAGS ${PC_FFmpeg_LDFLAGS} CACHE STRING "ffmpeg linker flags")

    # check if ffmpeg libs are statically linked
    set(FFmpeg_LIB_TYPE SHARED)
    foreach(_fflib IN LISTS FFmpeg_LIBRARIES)
      if(${_fflib} MATCHES ".+\.a$" AND PC_FFmpeg_STATIC_LDFLAGS)
        set(FFmpeg_LDFLAGS ${PC_FFmpeg_STATIC_LDFLAGS} CACHE STRING "ffmpeg linker flags" FORCE)
        set(FFmpeg_LIB_TYPE STATIC)
        break()
      endif()
    endforeach()

    set(FFmpeg_LIBRARIES ${FFmpeg_LIBAVCODEC} ${FFmpeg_LIBAVFILTER}
                         ${FFmpeg_LIBAVFORMAT} ${FFmpeg_LIBAVUTIL}
                         ${FFmpeg_LIBSWSCALE} ${FFmpeg_LIBSWRESAMPLE}
                         ${FFmpeg_LDFLAGS})
    if(FFmpeg_LIBPOSTPROC)
      list(APPEND FFmpeg_LIBRARIES ${FFmpeg_LIBPOSTPROC})
      list(APPEND FFmpeg_DEFINITIONS -DHAVE_LIBPOSTPROC=1)
    endif()
    list(APPEND FFmpeg_DEFINITIONS -DFFmpeg_VER_SHA=\"${FFmpeg_VERSION}\")

    if(NOT TARGET ffmpeg)
      add_library(ffmpeg ${FFmpeg_LIB_TYPE} IMPORTED)
      set_target_properties(ffmpeg PROPERTIES
                                   FOLDER "External Projects"
                                   IMPORTED_LOCATION "${FFmpeg_LIBRARIES}"
                                   INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIRS}"
                                   INTERFACE_LINK_LIBRARIES "${FFmpeg_LDFLAGS}"
                                   INTERFACE_COMPILE_DEFINITIONS "${FFmpeg_DEFINITIONS}")
    endif()
  endif()

# No internal FFmpeg fallback -- tools/depends no longer exists in spacecrafter.
if(NOT FFmpeg_FOUND)
  message(FATAL_ERROR
    "FFmpeg ${REQUIRED_FFmpeg_VERSION} was not found on the system.\n"
    "Install the required development packages:\n"
    "  sudo apt install libavcodec-dev libavformat-dev libavutil-dev"
    " libswscale-dev libavfilter-dev libswresample-dev\n"
    "Then re-run CMake. Alternatively, point CMake at a custom FFmpeg prefix:\n"
    "  -DFFmpeg_PATH=/path/to/ffmpeg   (keeps version checks)\n"
    "  -DWITH_FFmpeg=/path/to/ffmpeg   (skips version checks)")
endif()

mark_as_advanced(FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARIES FFmpeg_LDFLAGS FFmpeg_DEFINITIONS FFmpeg_FOUND)
