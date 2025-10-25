add_library(bandtest SHARED)

target_compile_definitions(bandtest PRIVATE
    BANDTEST_EXPORTS
)

target_link_libraries(bandtest PRIVATE
    wwcommon
)

if (WIN32)
    target_link_libraries(bandtest PRIVATE
        winmm
        ws2_32
    )
endif()

set(BANDTEST_SRC
    BandTest.cpp
    BandTest.h
)

if (WIN32)
    list(APPEND BANDTEST_SRC BandTest.rc)
endif()

target_sources(bandtest PRIVATE
    ${BANDTEST_SRC}
)
