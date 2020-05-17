ADD_SUBDIRECTORY( planetsephems ) 
ADD_SUBDIRECTORY( iniparser ) 


INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/planetsephems ${CMAKE_SOURCE_DIR}/src/iniparser )


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/appModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/appModule/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/coreModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/coreModule/*.cpp"  

        "${PROJECT_SOURCE_DIR}/src/uiModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/uiModule/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/navModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/navModule/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/bodyModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/bodyModule/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/mainModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/mainModule/*.cpp"

        "${PROJECT_SOURCE_DIR}/src/starModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/starModule/*.cpp"  

        "${PROJECT_SOURCE_DIR}/src/mediaModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/mediaModule/*.cpp"                

        "${PROJECT_SOURCE_DIR}/src/atmosphereModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/atmosphereModule/*.cpp"           

        "${PROJECT_SOURCE_DIR}/src/eventModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/eventModule/*.cpp"   

        "${PROJECT_SOURCE_DIR}/src/interfaceModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/interfaceModule/*.cpp"   

        "${PROJECT_SOURCE_DIR}/src/ojmModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/ojmModule/*.cpp"   

        "${PROJECT_SOURCE_DIR}/src/tools/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/tools/*.cpp"      
        )

# création de spacecrafter
add_executable(spacecrafter ${all_SRCS})
target_link_libraries(spacecrafter iniparser planetsephems ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${GLEW_LIBRARY} ${GLEW_LIBRARIES} ${FFMPEG_LIBRARIES} )
INSTALL(TARGETS spacecrafter DESTINATION bin)
