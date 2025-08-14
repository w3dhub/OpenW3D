set(GS_OPENSSL FALSE)
set(GS_WINSOCK2 FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        fe20bc06e06b4e3ead593c577c4ecbacfcfc514d
)

FetchContent_MakeAvailable(gamespy)
