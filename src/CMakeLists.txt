INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/planetsephems ${CMAKE_SOURCE_DIR}/iniparser )

file(GLOB_RECURSE all_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR} *.h *.hpp *.cpp )

# creation of spacecrafter
add_executable(spacecrafter ${all_SRCS})
target_link_libraries(spacecrafter PRIVATE iniparser planetsephems ${Vulkan_LIBRARIES})

if (UNIX)
target_link_libraries(spacecrafter PRIVATE ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_ttf_LIBRARY} ${SDL2_NET_LIBRARY} ${FFmpeg_LIBRARIES})
else()
# target_link_libraries(spacecrafter PRIVATE SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf SDL2_net::SDL2_net)

target_link_libraries(spacecrafter PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
target_link_libraries(spacecrafter PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(spacecrafter PRIVATE $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>)
target_link_libraries(spacecrafter PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)

target_include_directories(spacecrafter PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(spacecrafter PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(spacecrafter PRIVATE ${FFMPEG_LIBRARIES})

# target_include_directories(spacecrafter PRIVATE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
# target_link_libraries(spacecrafter SDL2 SDL2_mixer SDL2_ttf SDL2_net avdevice avfilter avformat avcodec avutil swscale)
# file(GLOB CARRIED_DLL ${PROJECT_SOURCE_DIR}/lib/*.dll)
# INSTALL(FILES ${CARRIED_DLL} DESTINATION bin)
endif()
INSTALL(TARGETS spacecrafter DESTINATION bin)

if (WIN32)
    install(DIRECTORY "$<TARGET_FILE_DIR:spacecrafter>/"
        DESTINATION bin
        FILES_MATCHING
            # --- FFmpeg core + modules
            PATTERN "avcodec*.dll"
            PATTERN "avformat*.dll"
            PATTERN "avutil*.dll"
            PATTERN "swscale*.dll"
            PATTERN "swresample*.dll"
            PATTERN "avdevice*.dll"
            PATTERN "avfilter*.dll"

            # --- SDL2 + add-ons
            PATTERN "SDL2*.dll"

            # --- Ogg family
            PATTERN "libogg-*.dll"
            PATTERN "libogg*.dll"
            PATTERN "ogg*.dll"

            # --- Opus
            PATTERN "libopus-*.dll"
            PATTERN "libopus*.dll"
            PATTERN "opus*.dll"

            # --- Opusfile
            PATTERN "libopusfile-*.dll"
            PATTERN "libopusfile*.dll"
            PATTERN "opusfile*.dll"

            # --- Vorbis (often pulled by SDL_mixer)
            PATTERN "libvorbis-*.dll"
            PATTERN "libvorbis*.dll"
            PATTERN "vorbis*.dll"
            PATTERN "libvorbisfile-*.dll"
            PATTERN "libvorbisfile*.dll"
            PATTERN "vorbisfile*.dll"

            # --- WavPack (all variants)
            PATTERN "libwavpack-*.dll"
            PATTERN "libwavpack*.dll"
            PATTERN "wavpack*.dll"
            PATTERN "wavpackdll*.dll"

            # --- Game Music Emu
            PATTERN "libgme*.dll"
            PATTERN "gme*.dll"

            # --- XMP (libxmp / xmp / libxmp-lite depending on port)
            PATTERN "libxmp*.dll"
            PATTERN "xmp*.dll"

            # --- FreeType
            PATTERN "freetype*.dll"
            PATTERN "libfreetype*.dll"
            PATTERN "freetype-6*.dll"

            # --- Brotli (according to FreeType build)
            PATTERN "brotli*.dll"
            PATTERN "brotlidec*.dll"
            PATTERN "brotlicommon*.dll"

            # --- zlib / libpng (often needed by FreeType)
            PATTERN "zlib*.dll"
            PATTERN "libpng*.dll"

            # --- BZip2
            PATTERN "bzip2*.dll"
            PATTERN "libbz2*.dll"
            PATTERN "bz2*.dll"

            # --- Exclude MSVC runtime libraries (already installed by CMake and MSVC automatically)
            PATTERN "msvcp*.dll" EXCLUDE
            PATTERN "vcruntime*.dll" EXCLUDE
            PATTERN "concrt*.dll" EXCLUDE
            PATTERN "api-ms-win-*.dll" EXCLUDE
            PATTERN "ext-ms-*.dll" EXCLUDE
    )
endif()
