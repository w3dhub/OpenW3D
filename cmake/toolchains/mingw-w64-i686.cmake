# MinGW-w64 i686 Cross-Compilation Toolchain for Linux
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-i686.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR X86)

# MinGW-w64 compiler prefix
set(MINGW_PREFIX i686-w64-mingw32)

# Compilers
set(CMAKE_C_COMPILER ${MINGW_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${MINGW_PREFIX}-windres)
set(CMAKE_AR ${MINGW_PREFIX}-ar)
set(CMAKE_RANLIB ${MINGW_PREFIX}-ranlib)
set(CMAKE_LINKER ${MINGW_PREFIX}-ld)
set(CMAKE_NM ${MINGW_PREFIX}-nm)
set(CMAKE_OBJCOPY ${MINGW_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${MINGW_PREFIX}-objdump)
set(CMAKE_STRIP ${MINGW_PREFIX}-strip)

# Target environment
set(CMAKE_FIND_ROOT_PATH /usr/${MINGW_PREFIX})

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# MinGW-specific flags
set(CMAKE_C_FLAGS_INIT "-static-libgcc")
set(CMAKE_CXX_FLAGS_INIT "-static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Windows version target (Windows 7+)
add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601)
