set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        112ee06236193fe5bd3f5164c6b3515eb50b437d
)

FetchContent_MakeAvailable(gamespy)
