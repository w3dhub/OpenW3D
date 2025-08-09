add_executable(launcher WIN32
    configfile.cpp
    configfile.h
    dictionary.h
    filed.h
    monod.cpp
    monod.h
    odevice.h
    streamer.cpp
    streamer.h
    wdebug.cpp
    wstring.cpp
    wstring.h


    Toolkit/Storage/File.cpp
    Toolkit/Storage/File.h
    Toolkit/Storage/Rights.h
    Toolkit/Storage/Stream.h

    Toolkit/Debug/DebugPrint.cpp
    Toolkit/Debug/DebugPrint.h

    Toolkit/Support/RefCounted.h
    Toolkit/Support/RefPtr.h
    Toolkit/Support/StringConvert.cpp
    Toolkit/Support/StringConvert.h
    Toolkit/Support/UString.cpp
    Toolkit/Support/UString.h
    Toolkit/Support/UTypes.h
    Toolkit/Support/Visualc.h

    BFISH.H
    dialog.cpp
    dialog.h
    findpatch.cpp
    findpatch.h
    launcher1.rc

    loadbmp.cpp
    loadbmp.h
    main.cpp
    patch.cpp
    patch.h
    process.cpp
    process.h
    Protect.h
    winblows.cpp
    winblows.h
    wstypes.h
)
target_include_directories(launcher PRIVATE "Toolkit")
target_link_libraries(launcher PRIVATE comctl32)
