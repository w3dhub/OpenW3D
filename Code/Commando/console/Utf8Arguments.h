// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#pragma once

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <limits>

namespace OpenW3D {

// Shared by the GUI wrapper and native console entry point. The caller owns
// one LocalAlloc block, including the terminating null argv entry.
inline char **CommandLineToArgvU(LPCWSTR command_line, int *count)
{
    wchar_t **wide = CommandLineToArgvW(command_line, count);
    if (!wide) return nullptr;
    size_t bytes = (static_cast<size_t>(*count) + 1) * sizeof(char *);
    for (int i = 0; i < *count; ++i) {
        int length = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
        if (!length || bytes > (std::numeric_limits<size_t>::max)() - static_cast<size_t>(length)) {
            LocalFree(wide);
            return nullptr;
        }
        bytes += static_cast<size_t>(length);
    }
    auto result = static_cast<char **>(LocalAlloc(LMEM_FIXED, bytes));
    if (result) {
        char *next = reinterpret_cast<char *>(result + *count + 1);
        for (int i = 0; i < *count; ++i) {
            int length = WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, nullptr, 0, nullptr, nullptr);
            result[i] = next;
            WideCharToMultiByte(CP_UTF8, 0, wide[i], -1, next, length, nullptr, nullptr);
            next += length;
        }
        result[*count] = nullptr;
    }
    LocalFree(wide);
    return result;
}

class Utf8Arguments {
public:
    Utf8Arguments() : Args(CommandLineToArgvU(GetCommandLineW(), &Count)) {}
    ~Utf8Arguments() { LocalFree(Args); }
    Utf8Arguments(const Utf8Arguments &) = delete;
    Utf8Arguments &operator=(const Utf8Arguments &) = delete;
    int Count = 0;
    char **Args;
};
}
#endif
