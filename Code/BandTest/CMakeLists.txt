add_library(bandtest SHARED)

target_compile_definitions(bandtest PRIVATE
    BANDTEST_EXPORTS
)

target_link_libraries(bandtest PRIVATE
    winmm
    ws2_32
)

set(BANDTEST_SRC
    BandTest.cpp
    BandTest.h
    BandTest.rc
)

target_include_directories(bandtest PRIVATE ../Commando)

target_sources(bandtest PRIVATE
    ${BANDTEST_SRC}
)
