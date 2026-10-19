// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
// Launched by subprocess_tests.py in a NEW, HIDDEN console, never the caller's.
#include "ConsoleHost.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {
HANDLE Report = INVALID_HANDLE_VALUE;
volatile LONG PreviousCalls = 0;
BOOL WINAPI PreviousHandler(DWORD event)
{
    if (event != CTRL_BREAK_EVENT && event != CTRL_C_EVENT) return FALSE;
    InterlockedIncrement(&PreviousCalls);
    return TRUE;
}
void Check(bool ok, const char *message)
{
    if (ok) return;
    DWORD written;
    WriteFile(Report, message, static_cast<DWORD>(std::strlen(message)), &written, nullptr);
    ExitProcess(1);
}
void Key(HANDLE input, wchar_t ch, WORD repeats = 1)
{
    INPUT_RECORD event {};
    event.EventType = KEY_EVENT;
    event.Event.KeyEvent.bKeyDown = TRUE;
    event.Event.KeyEvent.wRepeatCount = repeats;
    event.Event.KeyEvent.uChar.UnicodeChar = ch;
    DWORD count;
    Check(WriteConsoleInputW(input, &event, 1, &count) && count == 1, "write input event");
}
}

int main(int argc, char **argv)
{
    // Save the test report pipe before a private slave rebinds its streams.
    if (argc > 2) {
        // Leave CRT startup's terminal descriptors untouched for this case.
        Report = CreateFileA(argv[2], GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
        Check(Report != INVALID_HANDLE_VALUE, "open report");
    } else {
        Check(DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_OUTPUT_HANDLE), GetCurrentProcess(), &Report, 0, FALSE, DUPLICATE_SAME_ACCESS) != FALSE, "duplicate report");
    }
    bool private_console = argc > 1;
    HANDLE input = INVALID_HANDLE_VALUE;
    DWORD original_mode = 0;
    if (!private_console) {
        input = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        Check(input != INVALID_HANDLE_VALUE && SetStdHandle(STD_INPUT_HANDLE, input), "console input");
        Check(GetConsoleMode(input, &original_mode) != FALSE, "original mode");
        Check(SetConsoleCtrlHandler(PreviousHandler, TRUE) != FALSE, "previous handler");
    }
    OpenW3D::ConsoleHost host;
    bool initialized = host.Initialize(private_console);
    Check(initialized, host.Last_Error().c_str());
    if (private_console) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        input = GetStdHandle(STD_INPUT_HANDLE);
        DWORD processes[8];
        Check(GetConsoleProcessList(processes, 8) == 1 && processes[0] == GetCurrentProcessId(), "private input ownership");
        Check(GetConsoleMode(input, &original_mode) != FALSE, "private input mode");
    }
    std::vector<std::string> commands;
    host.Poll(commands); // Empty console queue must not block.
    Check(commands.empty(), "empty queue");
    for (wchar_t ch : std::wstring(L"caf\u00e9\b\u00e9 \xd83d\xde00\r")) Key(input, ch);
    host.Poll(commands);
    Check(commands.size() == 1 && commands[0] == "caf\xc3\xa9 \xf0\x9f\x98\x80", "unicode and backspace");
    // A repeat spanning several ticks must not lose the delimiter or next command.
    Key(input, L'x', 1000);
    Key(input, L'\r'); Key(input, L'o'); Key(input, L'k'); Key(input, L'\r');
    unsigned received = 0;
    for (int tick = 0; tick < 10; ++tick) {
        host.Poll(commands);
        for (const auto &line : commands) { Check(line == "ok", "oversized repeats"); ++received; }
    }
    Check(received == 1, "retained events");
    for (DWORD event : {CTRL_C_EVENT, CTRL_BREAK_EVENT}) {
        Check(GenerateConsoleCtrlEvent(event, 0) != FALSE, "generate control event");
        for (int i = 0; i < 100 && !host.Shutdown_Requested(); ++i) Sleep(10);
        Check(host.Shutdown_Requested(), "shutdown flag");
        Check(InterlockedCompareExchange(&PreviousCalls, 0, 0) == 0, "host handled event");
        Sleep(50);
    }
    host.Shutdown();
    if (!private_console) {
        DWORD restored_mode;
        Check(GetConsoleMode(input, &restored_mode) && restored_mode == original_mode, "restore input mode");
        Check(GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0) != FALSE, "generate restored event");
        for (int i = 0; i < 100 && !InterlockedCompareExchange(&PreviousCalls, 0, 0); ++i) Sleep(10);
        Check(InterlockedCompareExchange(&PreviousCalls, 0, 0) == 1, "restore previous handler");
        SetConsoleCtrlHandler(PreviousHandler, FALSE);

        HANDLE output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
        Check(output != INVALID_HANDLE_VALUE && SetStdHandle(STD_OUTPUT_HANDLE, output), "console output buffer");
        Check(host.Initialize(), "reinitialize");
        Check(!host.Shutdown_Requested(), "reset shutdown state");
        Check(host.Write("caf\xc3\xa9"), "unicode output");
        wchar_t text[4] {};
        DWORD read = 0;
        Check(ReadConsoleOutputCharacterW(output, text, 4, {0, 0}, &read) && read == 4 && std::wstring(text, 4) == L"caf\u00e9", "unicode output contents");
        host.Shutdown();
        Check(GetConsoleMode(output, &restored_mode) != FALSE, "inherited output remains open");
        CloseHandle(output);
        CloseHandle(input);
    }
    DWORD written;
    const char *message = "WINDOWS CONSOLE OK\n";
    Check(WriteFile(Report, message, static_cast<DWORD>(std::strlen(message)), &written, nullptr) != FALSE, "report cleanup");
    CloseHandle(Report);
    return 0;
}
