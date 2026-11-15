// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#include "ConsoleHost.h"
#include "CommandLine.h"

#include <cerrno>
#include <cstdio>

#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace OpenW3D {
namespace {
bool HostActive = false;
volatile sig_atomic_t StopRequested = 0;
void SignalHandler(int) { StopRequested = 1; }
}

struct ConsoleHost::Impl {
    bool Initialized = false;
    bool InputOpen = false;
    ConsoleCommandLine Lines;
    std::string Error;
    struct sigaction OldInt {}, OldTerm {}, OldPipe {};
};

ConsoleHost::ConsoleHost() : State(std::make_unique<Impl>()) {}
ConsoleHost::~ConsoleHost() { Shutdown(); }

bool ConsoleHost::Initialize([[maybe_unused]] bool private_windows_console)
{
    auto &s = *State;
    if (s.Initialized) return true;
    if (HostActive) { s.Error = "A console host is already active."; return false; }
    s = Impl{};
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
    s.Initialized = HostActive = true;
    return true;
}

void ConsoleHost::Shutdown()
{
    auto &s = *State;
    if (!s.Initialized) return;
    sigaction(SIGINT, &s.OldInt, nullptr);
    sigaction(SIGTERM, &s.OldTerm, nullptr);
    sigaction(SIGPIPE, &s.OldPipe, nullptr);
    s.Initialized = s.InputOpen = HostActive = false;
}

bool ConsoleHost::Write(std::string_view text, bool error)
{
    if (!State->Initialized) return false;
    // Preserve ordering with legacy stdio users without replacing their streams.
    std::fflush(error ? stderr : stdout);
    while (!text.empty()) {
        ssize_t count = write(error ? STDERR_FILENO : STDOUT_FILENO, text.data(), text.size());
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return false;
        text.remove_prefix(static_cast<size_t>(count));
    }
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
    return State->Initialized && StopRequested != 0;
}
bool ConsoleHost::Input_Closed() const { return !State->InputOpen; }
const std::string &ConsoleHost::Last_Error() const { return State->Error; }
}
