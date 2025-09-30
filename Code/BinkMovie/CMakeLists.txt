add_library(binkmovie STATIC
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
    target_compile_definitions(binkmovie PRIVATE W3D_HAS_FFMPEG)
    target_sources(binkmovie PRIVATE
        FFMpegPlayer.cpp
        FFMpegPlayer.h
    )
elseif(W3D_BUILD_OPTION_BINK)
    target_sources(binkmovie PRIVATE
        BinkPlayer.cpp
        BinkPlayer.h
    )
    target_compile_definitions(binkmovie PRIVATE W3D_HAS_BINK)
    target_link_libraries(binkmovie PRIVATE
        binkstub
    )
endif()

target_link_libraries(binkmovie PRIVATE
    wwcommon
)
