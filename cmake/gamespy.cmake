set(GS_OPENSSL FALSE)
set(GS_WINSOCK2 TRUE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        82258691a44a2aaebae787dbf9dfb872ecdcb237
)

FetchContent_MakeAvailable(gamespy)
