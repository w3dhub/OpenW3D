// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#include "CommandLine.h"
#include "Utf8Arguments.h"
#include <cstdio>
#include <cstdlib>

static void Check(bool condition, const char *message)
{
    if (!condition) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}

int main()
{
    OpenW3D::ConsoleCommandLine lines;
    std::string command;
    lines.Feed("ga");
    Check(!lines.Pop(command), "A partial command must not execute");
    lines.Feed("me_info\r");
    lines.Feed("\nhelp\n\n \t\n");
    Check(lines.Pop(command) && command == "game_info", "Fragmented CRLF input");
    Check(lines.Pop(command) && command == "help", "Multiple lines must retain order");
    Check(!lines.Pop(command), "Blank lines must be ignored");
    lines.Feed(std::string(200, 'x') + "\n");
    Check(lines.Pop(command) && command.size() == 200, "Accept the exact size limit");
    lines.Feed(std::string(201, 'x'));
    lines.Backspace();
    lines.Feed("quit\r\nhelp\n");
    Check(lines.TakeRejected() == 1, "Report overlong input once");
    Check(lines.Pop(command) && command == "help" && !lines.Pop(command), "Discard the entire oversized line");
    lines.Feed(std::string_view("quit\0help\n", 10));
    Check(lines.TakeRejected() == 1 && !lines.Pop(command), "Never dispatch NUL-truncated input");
    lines.Feed("msg caf\xc3");
    lines.Feed("\xa9");
    Check(lines.Backspace(), "Backspace removes a UTF-8 code point");
    lines.Feed("\xc3\xa9\n");
    Check(lines.Pop(command) && command == "msg caf\xc3\xa9", "UTF-8 survives input chunks");
    lines.Feed("helq\bp\x7f" "p\n");
    Check(lines.Pop(command) && command == "help", "Backspace and delete in byte input");
    lines.Feed("quit");
    lines.Finish();
    lines.Finish();
    Check(lines.Pop(command) && command == "quit" && !lines.Pop(command), "EOF submits a final line once");
#ifdef _WIN32
    int argc = 0;
    char **argv = OpenW3D::CommandLineToArgvU(L"server.exe --ini \"C:\\caf\u00e9 folder\\server.ini\" \"\"", &argc);
    Check(argv && argc == 4 && !argv[argc], "UTF-8 argv is null-terminated");
    Check(std::string(argv[2]) == "C:\\caf\xc3\xa9 folder\\server.ini" && std::string(argv[3]).empty(), "Quoted UTF-8 and empty arguments");
    LocalFree(argv);
#endif
    return 0;
}
