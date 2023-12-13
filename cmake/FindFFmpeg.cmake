# FindFFmpeg
# --------
# Finds FFmpeg libraries
#
# This module will first look for the required library versions on the system.
# If they are not found, it will fall back to downloading and building kodi's own version
#
# --------
# the following variables influence behaviour:
# ENABLE_INTERNAL_FFmpeg - if enabled, kodi's own version will always be built
#
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
set(_postproc_ver ">=54.5.100")


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
  unset(_postproc_ver)
endif()

# Allows building with external ffmpeg not found in system paths,
# with library version checks
if(FFmpeg_PATH)
  set(ENABLE_INTERNAL_FFmpeg OFF)
endif()

# external FFmpeg
if(NOT ENABLE_INTERNAL_FFmpeg OR KODI_DEPENDSBUILD)
  if(FFmpeg_PATH)
    set(ENV{PKG_CONFIG_PATH} "${FFmpeg_PATH}/lib/pkgconfig")
    list(APPEND CMAKE_PREFIX_PATH ${FFmpeg_PATH})
  endif()

  set(FFmpeg_PKGS libavcodec${_avcodec_ver}
                  libavfilter${_avfilter_ver}
                  libavformat${_avformat_ver}
                  libavutil${_avutil_ver}
                  libswscale${_swscale_ver}
                  libswresample${_swresample_ver}
                  libpostproc${_postproc_ver})

  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_FFmpeg ${FFmpeg_PKGS} QUIET)
    string(REGEX REPLACE "framework;" "framework " PC_FFmpeg_LDFLAGS "${PC_FFmpeg_LDFLAGS}")
  endif()

  find_path(FFmpeg_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                                libavutil/avutil.h libswscale/swscale.h libpostproc/postprocess.h
            PATH_SUFFIXES ffmpeg
            PATHS ${PC_FFmpeg_INCLUDE_DIRS}
            NO_DEFAULT_PATH)
  find_path(FFmpeg_INCLUDE_DIRS libavcodec/avcodec.h libavfilter/avfilter.h libavformat/avformat.h
                                libavutil/avutil.h libswscale/swscale.h libpostproc/postprocess.h)

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

  find_library(FFmpeg_LIBPOSTPROC
               NAMES postproc libpostproc
               PATH_SUFFIXES ffmpeg/libpostproc
               PATHS ${PC_FFmpeg_libpostproc_LIBDIR}
               NO_DEFAULT_PATH)
  find_library(FFmpeg_LIBPOSTPROC NAMES postproc libpostproc PATH_SUFFIXES ffmpeg/libpostproc)

  if((PC_FFmpeg_FOUND
      AND PC_FFmpeg_libavcodec_VERSION
      AND PC_FFmpeg_libavfilter_VERSION
      AND PC_FFmpeg_libavformat_VERSION
      AND PC_FFmpeg_libavutil_VERSION
      AND PC_FFmpeg_libswscale_VERSION
      AND PC_FFmpeg_libswresample_VERSION
      AND PC_FFmpeg_libpostproc_VERSION)
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
                                                    FFmpeg_LIBPOSTPROC
                                                    FFmpeg_VERSION
                                      FAIL_MESSAGE "FFmpeg ${REQUIRED_FFmpeg_VERSION} not found, please consider using -DENABLE_INTERNAL_FFmpeg=ON")

  else()
    message(STATUS "FFmpeg ${REQUIRED_FFmpeg_VERSION} not found, falling back to internal build")
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
                         ${FFmpeg_LIBPOSTPROC} ${FFmpeg_LDFLAGS})
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
endif()

# Internal FFmpeg
if(NOT FFmpeg_FOUND)
  include(ExternalProject)
  file(STRINGS ${CMAKE_SOURCE_DIR}/tools/depends/target/ffmpeg/FFmpeg-VERSION VER)
  string(REGEX MATCH "VERSION=[^ ]*$.*" FFmpeg_VER "${VER}")
  list(GET FFmpeg_VER 0 FFmpeg_VER)
  string(SUBSTRING "${FFmpeg_VER}" 8 -1 FFmpeg_VER)
  string(REGEX MATCH "BASE_URL=([^ ]*)" FFmpeg_BASE_URL "${VER}")
  list(GET FFmpeg_BASE_URL 0 FFmpeg_BASE_URL)
  string(SUBSTRING "${FFmpeg_BASE_URL}" 9 -1 FFmpeg_BASE_URL)

  # allow user to override the download URL with a local tarball
  # needed for offline build envs
  if(FFmpeg_URL)
    get_filename_component(FFmpeg_URL "${FFmpeg_URL}" ABSOLUTE)
  else()
    set(FFmpeg_URL ${FFmpeg_BASE_URL}/archive/${FFmpeg_VER}.tar.gz)
  endif()
  if(VERBOSE)
    message(STATUS "FFmpeg_URL: ${FFmpeg_URL}")
  endif()

  if(KODI_DEPENDSBUILD)
    set(CROSS_ARGS -DDEPENDS_PATH=${DEPENDS_PATH}
                   -DPKG_CONFIG_EXECUTABLE=${PKG_CONFIG_EXECUTABLE}
                   -DCROSSCOMPILING=${CMAKE_CROSSCOMPILING}
                   -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                   -DOS=${OS}
                   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_AR=${CMAKE_AR})
  endif()

  externalproject_add(ffmpeg
                      URL ${FFmpeg_URL}
                      DOWNLOAD_NAME ffmpeg-${FFmpeg_VER}.tar.gz
                      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/download
                      PREFIX ${CORE_BUILD_DIR}/ffmpeg
                      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DFFmpeg_VER=${FFmpeg_VER}
                                 -DCORE_SYSTEM_NAME=${CORE_SYSTEM_NAME}
                                 -DCORE_PLATFORM_NAME=${CORE_PLATFORM_NAME_LC}
                                 -DCPU=${CPU}
                                 -DENABLE_NEON=${ENABLE_NEON}
                                 -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                                 -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                                 -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
                                 ${CROSS_ARGS}
                      PATCH_COMMAND ${CMAKE_COMMAND} -E copy
                                    ${CMAKE_SOURCE_DIR}/tools/depends/target/ffmpeg/CMakeLists.txt
                                    <SOURCE_DIR> &&
                                    ${CMAKE_COMMAND} -E copy
                                    ${CMAKE_SOURCE_DIR}/tools/depends/target/ffmpeg/FindGnuTls.cmake
                                    <SOURCE_DIR>)

  file(WRITE ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
"#!/bin/bash
if [[ $@ == *${APP_NAME_LC}.bin* || $@ == *${APP_NAME_LC}.so* || $@ == *${APP_NAME_LC}-test* ]]
then
  avformat=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libavcodec`
  avcodec=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libavformat`
  avfilter=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libavfilter`
  avutil=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libavutil`
  swscale=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libswscale`
  swresample=`PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/lib/pkgconfig ${PKG_CONFIG_EXECUTABLE} --libs --static libswresample`
  gnutls=`PKG_CONFIG_PATH=${DEPENDS_PATH}/lib/pkgconfig/ ${PKG_CONFIG_EXECUTABLE}  --libs-only-l --static --silence-errors gnutls`
  $@ $avcodec $avformat $avcodec $avfilter $swscale $swresample -lpostproc $gnutls
else
  $@
fi")
  file(COPY ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg/ffmpeg-link-wrapper
       DESTINATION ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}
       FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
  set(FFmpeg_LINK_EXECUTABLE "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg-link-wrapper <CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
  set(FFmpeg_CREATE_SHARED_LIBRARY "${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/ffmpeg-link-wrapper <CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>" PARENT_SCOPE)
  set(FFmpeg_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/include)
  list(APPEND FFmpeg_DEFINITIONS -DFFmpeg_VER_SHA=\"${FFmpeg_VER}\"
                                 -DUSE_STATIC_FFmpeg=1)
  set(FFmpeg_FOUND 1)
  set_target_properties(ffmpeg PROPERTIES FOLDER "External Projects")
endif()

mark_as_advanced(FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARIES FFmpeg_LDFLAGS FFmpeg_DEFINITIONS FFmpeg_FOUND)
