set(WWCONFIG_SRC
    AudioConfigDialog.cpp
    DriverVersionWarning.cpp
    EZGIMEX.cpp
    locale.cpp
    Locale_API.cpp
    PerformanceConfigDialog.cpp
    StdAfx.cpp
    VideoConfigDialog.cpp
    WWConfig.cpp
    WWConfigDlg.cpp
    AudioConfigDialog.h
    DriverVersionWarning.h
    gimex.h
    locale.h
    Locale_API.h
    PerformanceConfigDialog.h
    Resource.h
    StdAfx.h
    VideoConfigDialog.h
    WWConfig.h
    WWConfigDlg.h
    WWConfig.rc
)

# Targets to build.
add_executable(wwconfig WIN32)

target_compile_definitions(wwconfig PRIVATE
    _AFXDLL
    $<$<CONFIG:Debug>:WWDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WWDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

target_link_libraries(wwconfig PRIVATE
    vfw32
    d3d8lib
    version
    winmm
    milesstub
    wwdebug
    wwlib
    wwmath
    wwphys
    wwsaveload
    ww3d2
    wwaudio
)

target_compile_options(wwconfig PRIVATE
    ${WARNING_FLAGS}
)

target_include_directories(wwconfig PRIVATE
    ../../wwdebug
    ../../wwlib
    ../../ww3d2
    ../../WWMath
    ../../wwphys
    ../../wwsaveload
    ../../WWAudio
    ../..
)

target_sources(wwconfig PRIVATE ${WWCONFIG_SRC})

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(wwconfig PRIVATE "/NODEFAULTLIB:libci.lib")
endif()
