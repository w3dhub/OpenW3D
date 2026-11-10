// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#include "ConsoleHost.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <csignal>
#include <unistd.h>
namespace {
volatile sig_atomic_t PreviousCalls = 0;
void PreviousHandler(int) { PreviousCalls = 1; }
}
#endif

int main(int argc, char **argv)
{
    int ticks = 100;
    bool missing = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--missing-stdin") == 0) missing = true;
        else ticks = std::atoi(argv[i]);
    }
    if (missing) {
#ifdef _WIN32
        SetStdHandle(STD_INPUT_HANDLE, nullptr);
#else
        close(STDIN_FILENO);
#endif
    }
#ifndef _WIN32
    struct sigaction previous {}, original_int {}, original_term {}, original_pipe {};
    previous.sa_handler = PreviousHandler;
    sigemptyset(&previous.sa_mask);
    sigaction(SIGINT, &previous, &original_int);
    sigaction(SIGTERM, &previous, &original_term);
    sigaction(SIGPIPE, nullptr, &original_pipe);
#endif
    OpenW3D::ConsoleHost host;
    if (!host.Initialize()) { std::fprintf(stderr, "%s\n", host.Last_Error().c_str()); return 1; }
    if (!host.Initialize()) return 2; // Reinitializing the active host is harmless.
    OpenW3D::ConsoleHost second;
    if (second.Initialize()) return 3;
    host.Write("READY\n");
    host.Write("STDERR caf\xc3\xa9\n", true);
    int stop_tick = -1;
    bool closed = false;
    for (int tick = 0; tick < ticks; ++tick) {
        std::vector<std::string> commands;
        host.Poll(commands);
        for (const auto &command : commands) host.Write("COMMAND " + command + "\n");
        if (host.Input_Closed() && !closed) { host.Write("EOF\n"); closed = true; }
        if (host.Shutdown_Requested() && stop_tick < 0) {
            host.Write("STOP\n");
            stop_tick = tick;
        }
        host.Write("TICK\n");
        if (stop_tick >= 0 && tick - stop_tick >= 20) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    host.Shutdown();
    host.Shutdown();
#ifndef _WIN32
    // The handlers installed before the host must be back in place.
    raise(SIGINT);
    if (!PreviousCalls) return 4;
    PreviousCalls = 0;
    raise(SIGTERM);
    if (!PreviousCalls) return 5;
    struct sigaction restored_pipe {};
    sigaction(SIGPIPE, nullptr, &restored_pipe);
    if (restored_pipe.sa_handler != original_pipe.sa_handler) return 6;
    sigaction(SIGINT, &original_int, nullptr);
    sigaction(SIGTERM, &original_term, nullptr);
#endif
    // Borrowed streams remain usable after shutdown.
    std::printf("CLEANUP\n");
    std::fprintf(stderr, "STDERR CLEANUP\n");
    return 0;
}
