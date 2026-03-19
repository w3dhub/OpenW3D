#include "rcfile.h"

#ifdef OPENW3D_SDL3
#include <string>
#include <unordered_map>

#if defined(__GNUC__) || defined(__clang__)
#define WEAK_SYMBOL __attribute__ ((weak))
#else
// MSVC provides weak linking through static libraries:
// https://devblogs.microsoft.com/oldnewthing/20251003-00/?p=111650
#define WEAK_SYMBOL
#endif

std::unordered_map<std::string, StaticResourceFileClass> Static_Resources WEAK_SYMBOL;

#endif
