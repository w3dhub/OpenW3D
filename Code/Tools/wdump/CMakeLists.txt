set(WDUMP_SRC
    FindDialog.cpp
    FindDialog.h
    mainfrm.cpp
    mainfrm.h
    resource.h
    stdafx.cpp
    stdafx.h
    wdeview.cpp
    wdeview.h
    wdlview.cpp
    wdlview.h
    wdtview.cpp
    wdtview.h
    chunk_d.cpp
    chunk_d.h
    res/toolbar.bmp
    res/wdump.ico
    res/wdump.rc2
    res/wdumpdoc.ico
    wdview.cpp
    wdview.h
    wdump.cpp
    wdump.h
    wdump.rc
    wdumpdoc.cpp
    wdumpdoc.h
)

# Targets to build.
add_executable(wdump WIN32)

target_compile_definitions(wdump PRIVATE
        _AFXDLL
        $<$<CONFIG:Debug>:WWDEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
)

target_include_directories(wdump PRIVATE
        ../../wwdebug
        ../../wwlib
        ../../ww3d2
        ../../WWMath
        ../../wwphys
        ../../wwsaveload
        ../../WWAudio
        ../..
)

target_link_libraries(wdump PRIVATE
    wwcommon
    wwdebug
    wwlib
)

target_sources(wdump PRIVATE ${WDUMP_SRC})

if(MSVC)
    target_link_options(wdump PRIVATE "/NODEFAULTLIB:libci.lib")
endif()
