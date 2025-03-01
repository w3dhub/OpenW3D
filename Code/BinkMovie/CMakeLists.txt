# Set source files
set(BINKMOVIE_SRC
    BINKMovie.cpp
    subtitle.cpp
    subtitlemanager.cpp
    subtitleparser.cpp
    BINKMovie.h
    subtitle.h
    subtitlemanager.h
    subtitleparser.h
)

# Targets to build.
add_library(binkmovie STATIC)

target_link_libraries(binkmovie PRIVATE
    wwcommon
)

target_sources(binkmovie PRIVATE ${BINKMOVIE_SRC})