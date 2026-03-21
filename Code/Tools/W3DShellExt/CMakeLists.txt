set(W3DSHELLEXT_SRC
    ../wdump/chunk_d.cpp
    ../wdump/FindDialog.cpp
    ../wdump/wdumpdoc.cpp
    W3DShellExt.def
    W3DView.ico
    copyhook.cpp
    ctxmenu.cpp
    iconhdlr.cpp
    propshet.cpp
    resource.h
    shellext.RC
    shellext.cpp
    shellext.h
    shexinit.cpp
)

# Targets to build.
add_library(W3DShellExt MODULE
)

target_compile_definitions(W3DShellExt PRIVATE
    _AFXDLL
    _W3DSHELLEXT
    $<$<CONFIG:Debug>:WWDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

target_include_directories(W3DShellExt PRIVATE
    ../wdump
    ../../wwdebug
    ../../wwlib
    ../../ww3d2
    ../../WWMath
    ../../wwphys
    ../../wwsaveload
    ../../WWAudio
    ../..
)

target_link_libraries(W3DShellExt PRIVATE
    wwcommon
    wwdebug
    wwlib
)

target_sources(W3DShellExt PRIVATE ${W3DSHELLEXT_SRC})

if(MSVC)
    target_link_options(W3DShellExt PRIVATE "/NODEFAULTLIB:libci.lib")
endif()
