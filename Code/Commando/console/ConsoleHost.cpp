// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#include "ConsoleHost.h"
#include "CommandLine.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#endif

namespace OpenW3D {
namespace {
bool HostActive = false;
#ifdef _WIN32
volatile LONG StopRequested = 0;
BOOL WINAPI ControlHandler(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&StopRequested, 1);
    return TRUE;
}
bool BindPrivateStream(FILE *stream, const char *device, DWORD standard_handle)
{
    // A child launched without standard handles can have CRT descriptor -2.
    if (_fileno(stream) < 0 && !std::freopen("NUL", "r+", stream)) return false;
    const int source = _open(device, _O_RDWR | _O_TEXT);
    if (source < 0) return false;
    const int destination = _fileno(stream);
    const bool bound = source == destination || _dup2(source, destination) == 0;
    if (source != destination) _close(source);
    std::clearerr(stream);
    return bound && SetStdHandle(standard_handle, reinterpret_cast<HANDLE>(_get_osfhandle(destination)));
}
#else
volatile sig_atomic_t StopRequested = 0;
void SignalHandler(int) { StopRequested = 1; }
#endif
}

struct ConsoleHost::Impl {
    bool Initialized = false;
    bool InputOpen = false;
    ConsoleCommandLine Lines;
    std::string Error;
#ifdef _WIN32
    HANDLE Input = INVALID_HANDLE_VALUE;
    HANDLE Output[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    bool TerminalInput = false;
    bool TerminalOutput[2] = {false, false};
    bool OwnConsole = false;
    bool ModeChanged = false;
    DWORD SavedMode = 0;
    wchar_t HighSurrogate = 0;
    INPUT_RECORD Events[64] {};
    DWORD EventCount = 0;
    DWORD NextEvent = 0;
#else
    struct sigaction OldInt {}, OldTerm {}, OldPipe {};
#endif
};

ConsoleHost::ConsoleHost() : State(std::make_unique<Impl>()) {}
ConsoleHost::~ConsoleHost() { Shutdown(); }

bool ConsoleHost::Initialize(bool private_windows_console)
{
    auto &s = *State;
    if (s.Initialized) return true;
    if (HostActive) { s.Error = "A console host is already active."; return false; }
    s = Impl{};
#ifdef _WIN32
    // Only slave children replace their inherited console and streams. This must
    // precede handler registration: AllocConsole resets the handler table.
    if (private_windows_console) {
        std::fflush(stdout);
        std::fflush(stderr);
        // Retire CRT references before detaching, keeping valid descriptors for
        // subsequent rebinding. AllocConsole may reuse old console handle values.
        if (!BindPrivateStream(stdin, "NUL", STD_INPUT_HANDLE) ||
            !BindPrivateStream(stdout, "NUL", STD_OUTPUT_HANDLE) ||
            !BindPrivateStream(stderr, "NUL", STD_ERROR_HANDLE)) {
            s.Error = "Could not release slave console streams.";
            return false;
        }
        FreeConsole();
        if (!AllocConsole()) { s.Error = "Could not allocate slave console."; return false; }
        s.OwnConsole = true;
        // Console mode APIs need read/write handles, including for stdout.
        if (!BindPrivateStream(stdin, "CONIN$", STD_INPUT_HANDLE) ||
            !BindPrivateStream(stdout, "CONOUT$", STD_OUTPUT_HANDLE) ||
            !BindPrivateStream(stderr, "CONOUT$", STD_ERROR_HANDLE)) {
            s.Error = "Could not bind slave console streams.";
            FreeConsole();
            s.OwnConsole = false;
            return false;
        }
    }
    s.Input = GetStdHandle(STD_INPUT_HANDLE);
    s.InputOpen = s.Input && s.Input != INVALID_HANDLE_VALUE;
    s.TerminalInput = s.InputOpen && GetConsoleMode(s.Input, &s.SavedMode);
    if (s.TerminalInput) {
        // Quick Edit can suspend the whole process. Input is read as events;
        // cooked input and echo flags do not apply to ReadConsoleInputW.
        DWORD mode = (s.SavedMode | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE;
        if (!SetConsoleMode(s.Input, mode)) {
            s.Error = "Could not set console input mode.";
            if (s.OwnConsole) FreeConsole();
            s.OwnConsole = false;
            return false;
        }
        s.ModeChanged = true;
    }
    for (int i = 0; i != 2; ++i) {
        s.Output[i] = GetStdHandle(i == 0 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
        DWORD mode = 0;
        s.TerminalOutput[i] = GetConsoleMode(s.Output[i], &mode) != FALSE;
    }
    InterlockedExchange(&StopRequested, 0);
    if (!SetConsoleCtrlHandler(ControlHandler, TRUE)) {
        s.Error = "Could not register console control handler.";
        if (s.ModeChanged) SetConsoleMode(s.Input, s.SavedMode);
        if (s.OwnConsole) FreeConsole();
        s.OwnConsole = false;
        s.ModeChanged = false;
        return false;
    }
#else
    (void)private_windows_console;
    s.InputOpen = fcntl(STDIN_FILENO, F_GETFD) != -1;
    struct sigaction action {};
    action.sa_handler = SignalHandler;
    sigemptyset(&action.sa_mask);
    StopRequested = 0;
    if (sigaction(SIGINT, &action, &s.OldInt) != 0) {
        s.Error = "Could not register SIGINT handler."; return false;
    }
    if (sigaction(SIGTERM, &action, &s.OldTerm) != 0) {
        sigaction(SIGINT, &s.OldInt, nullptr);
        s.Error = "Could not register SIGTERM handler."; return false;
    }
    // A log consumer closing its pipe must not terminate the game in write().
    action.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &action, &s.OldPipe) != 0) {
        sigaction(SIGTERM, &s.OldTerm, nullptr);
        sigaction(SIGINT, &s.OldInt, nullptr);
        s.Error = "Could not disable SIGPIPE."; return false;
    }
#endif
    s.Initialized = HostActive = true;
    return true;
}

void ConsoleHost::Shutdown()
{
    auto &s = *State;
    if (!s.Initialized) return;
#ifdef _WIN32
    SetConsoleCtrlHandler(ControlHandler, FALSE);
    if (s.ModeChanged) SetConsoleMode(s.Input, s.SavedMode);
    if (s.OwnConsole) FreeConsole();
    s.OwnConsole = s.ModeChanged = false;
#else
    sigaction(SIGINT, &s.OldInt, nullptr);
    sigaction(SIGTERM, &s.OldTerm, nullptr);
    sigaction(SIGPIPE, &s.OldPipe, nullptr);
#endif
    s.Initialized = s.InputOpen = HostActive = false;
}

bool ConsoleHost::Write(std::string_view text, bool error)
{
    if (!State->Initialized) return false;
    // Preserve ordering with legacy stdio users without replacing their streams.
    std::fflush(error ? stderr : stdout);
#ifdef _WIN32
    auto &s = *State;
    HANDLE output = s.Output[error ? 1 : 0];
    if (!output || output == INVALID_HANDLE_VALUE) return false;
    while (!text.empty()) {
        size_t length = std::min<size_t>(text.size(), 16384);
        if (s.TerminalOutput[error ? 1 : 0]) {
            // Do not split a UTF-8 code point at the chunk boundary.
            if (length < text.size()) {
                while (length && (static_cast<unsigned char>(text[length]) & 0xc0) == 0x80) --length;
                if (!length) length = std::min<size_t>(text.size(), 16384);
            }
            int count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(length), nullptr, 0);
            if (!count) return false;
            std::wstring wide(static_cast<size_t>(count), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(length), wide.data(), count);
            size_t offset = 0;
            while (offset < wide.size()) {
                DWORD written = 0;
                if (!WriteConsoleW(output, wide.data() + offset, static_cast<DWORD>(wide.size() - offset), &written, nullptr) || !written) return false;
                offset += written;
            }
        } else {
            DWORD written = 0;
            if (!WriteFile(output, text.data(), static_cast<DWORD>(length), &written, nullptr) || !written) return false;
            length = written;
        }
        text.remove_prefix(length);
    }
#else
    while (!text.empty()) {
        ssize_t count = write(error ? STDERR_FILENO : STDOUT_FILENO, text.data(), text.size());
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return false;
        text.remove_prefix(static_cast<size_t>(count));
    }
#endif
    return true;
}

void ConsoleHost::Poll(std::vector<std::string> &commands)
{
    commands.clear();
    auto &s = *State;
    if (!s.Initialized) return;
    bool eof = false;
    bool failed = false;
    // A read can produce many short commands. Drain them before another read,
    // bounding both storage and the amount of work performed by any game tick.
    if (s.InputOpen && !s.Lines.HasCommands()) {
        char bytes[256];
#ifdef _WIN32
        if (s.TerminalInput) {
            DWORD available = 0;
            if (s.NextEvent == s.EventCount) {
                s.NextEvent = s.EventCount = 0;
                if (!GetNumberOfConsoleInputEvents(s.Input, &available)) failed = true;
                // ReadConsoleInput blocks on an empty queue; always check first.
                if (available && !ReadConsoleInputW(s.Input, s.Events, std::min<DWORD>(available, 64), &s.EventCount)) {
                    failed = true;
                }
            }
            if (!failed) {
                unsigned budget = 256;
                while (s.NextEvent < s.EventCount && budget && !eof) {
                    auto &event = s.Events[s.NextEvent];
                    auto &key = event.Event.KeyEvent;
                    if (event.EventType != KEY_EVENT || !key.bKeyDown) { ++s.NextEvent; continue; }
                    wchar_t ch = key.uChar.UnicodeChar;
                    if (!ch || ch == 27) { ++s.NextEvent; continue; }
                    unsigned repeats = std::min<unsigned>(key.wRepeatCount, budget);
                    budget -= repeats;
                    key.wRepeatCount = static_cast<WORD>(key.wRepeatCount - repeats);
                    if (!key.wRepeatCount) ++s.NextEvent;
                    for (unsigned repeat = 0; repeat < repeats && !eof; ++repeat) {
                        if (ch == 26) { eof = true; break; } // Ctrl+Z
                        if (ch == L'\b') {
                            s.HighSurrogate = 0;
                            if (s.Lines.Backspace() && s.TerminalOutput[0]) Write("\b \b");
                            continue;
                        }
                        if (ch >= 0xd800 && ch <= 0xdbff) { s.HighSurrogate = ch; continue; }
                        wchar_t wide[2] = {ch, 0};
                        int length = 1;
                        if (s.HighSurrogate && ch >= 0xdc00 && ch <= 0xdfff) {
                            wide[0] = s.HighSurrogate; wide[1] = ch; length = 2;
                        }
                        s.HighSurrogate = 0;
                        int encoded = WideCharToMultiByte(CP_UTF8, 0, wide, length, bytes, sizeof(bytes), nullptr, nullptr);
                        if (encoded) {
                            std::string_view input(bytes, static_cast<size_t>(encoded));
                            s.Lines.Feed(input);
                            if (s.TerminalOutput[0]) Write(ch == L'\r' ? std::string_view("\r\n") : input);
                        }
                    }
                }
            }
        } else {
            DWORD type = GetFileType(s.Input);
            DWORD available = sizeof(bytes);
            if (type == FILE_TYPE_PIPE && !PeekNamedPipe(s.Input, nullptr, 0, nullptr, &available, nullptr)) {
                eof = GetLastError() == ERROR_BROKEN_PIPE;
                failed = !eof;
            } else if (type == FILE_TYPE_CHAR) {
                // NUL is a common stdin for background launches. Other character
                // devices (e.g. serial ports) are not command input for this host.
                eof = true;
            } else if (type != FILE_TYPE_PIPE && type != FILE_TYPE_DISK) {
                failed = true;
            } else if (available) {
                DWORD count = 0;
                if (ReadFile(s.Input, bytes, std::min<DWORD>(available, sizeof(bytes)), &count, nullptr)) {
                    eof = count == 0;
                    s.Lines.Feed(std::string_view(bytes, count));
                } else {
                    eof = GetLastError() == ERROR_BROKEN_PIPE || GetLastError() == ERROR_HANDLE_EOF;
                    failed = !eof;
                }
            }
        }
#else
        struct pollfd input {STDIN_FILENO, POLLIN, 0};
        int ready = poll(&input, 1, 0);
        if (ready < 0) {
            failed = errno != EINTR;
        } else if (ready > 0) {
            if (input.revents & (POLLIN | POLLHUP)) {
                ssize_t count = read(STDIN_FILENO, bytes, sizeof(bytes));
                if (count > 0) s.Lines.Feed(std::string_view(bytes, static_cast<size_t>(count)));
                else if (count == 0) eof = true;
                else failed = errno != EINTR && errno != EAGAIN;
            } else if (input.revents & (POLLERR | POLLNVAL)) {
                failed = true;
            }
        }
#endif
    }
    if (eof) { s.Lines.Finish(); s.InputOpen = false; }
    if (failed) {
        s.InputOpen = false;
        s.Error = "Console input unavailable; continuing without stdin.\n";
        Write(s.Error, true);
    }
    if (s.Lines.TakeRejected()) Write("Ignoring console command: exceeds 200 UTF-8 bytes or contains NUL.\n", true);
    std::string command;
    for (unsigned i = 0; i < 16 && s.Lines.Pop(command); ++i) commands.push_back(std::move(command));
}

bool ConsoleHost::Shutdown_Requested() const
{
#ifdef _WIN32
    return State->Initialized && InterlockedCompareExchange(&StopRequested, 0, 0) != 0;
#else
    return State->Initialized && StopRequested != 0;
#endif
}
bool ConsoleHost::Input_Closed() const { return !State->InputOpen; }
const std::string &ConsoleHost::Last_Error() const { return State->Error; }
}
