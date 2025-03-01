# Set source files
set(WWONLINE_SRC
    GameResField.cpp
    GameResField.h
    GameResPacket.cpp
    GameResPacket.h
    PingProfile.cpp
    PingProfile.h
    RefCounted.h
    RefPtr.h
    WaitCondition.cpp
    WaitCondition.h
    WOLAgeCheck.h
    WOLChannel.cpp
    WOLChannel.h
    WOLChatMsg.cpp
    WOLChatMsg.h
    WOLChatObserver.cpp
    WOLChatObserver.h
    WOLConnect.cpp
    WOLConnect.h
    WOLDownload.cpp
    WOLDownload.h
    WOLErrorUtil.cpp
    WOLErrorUtil.h
    WOLGame.cpp
    WOLGame.h
    WOLGameOptions.cpp
    WOLGameOptions.h
    WOLLadder.cpp
    WOLLadder.h
    WOLLangCodes.h
    WOLLoginInfo.cpp
    WOLLoginInfo.h
    WOLNetUtilObserver.cpp
    WOLNetUtilObserver.h
    WOLPageMsg.cpp
    WOLPageMsg.h
    WOLPing.h
    WOLProduct.cpp
    WOLProduct.h
    WOLServer.cpp
    WOLServer.h
    WOLSession.cpp
    WOLSession.h
    WOLSquad.cpp
    WOLSquad.h
    WOLString.cpp
    WOLString.h
    WOLUser.cpp
    WOLUser.h
)

# Targets to build.
add_library(wwonline STATIC)

target_link_libraries(wwonline PRIVATE
    wwcommon
)

target_sources(wwonline PRIVATE ${WWONLINE_SRC})