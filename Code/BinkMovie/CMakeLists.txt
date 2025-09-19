# Set source files
set(BINKMOVIE_SRC
    BINKMovie.cpp
    subtitle.cpp
    subtitlemanager.cpp
    subtitleparser.cpp
    BINKMovie.h
    MoviePlayer.h
    subtitle.h
    subtitlemanager.h
    subtitleparser.h
)

if(W3D_BUILD_OPTION_FFMPEG)
    list(APPEND BINKMOVIE_SRC
        FFMpegPlayer.cpp
        FFMpegPlayer.h
    )
elseif(W3D_BUILD_OPTION_BINK)
    list(APPEND BINKMOVIE_SRC
        BinkPlayer.cpp
        BinkPlayer.h
    )
endif()

# Targets to build.
add_library(binkmovie STATIC)

target_link_libraries(binkmovie PRIVATE
    wwcommon
)

if(W3D_BUILD_OPTION_BINK)
    target_link_libraries(binkmovie PRIVATE
        binkstub
    )
endif()

target_sources(binkmovie PRIVATE ${BINKMOVIE_SRC})
