// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace OpenW3D {
// One host per process. Polling and command dispatch belong to the game thread.
// Borrows standard streams; only an explicitly requested slave console is owned.
class ConsoleHost {
public:
    ConsoleHost();
    ~ConsoleHost();
    ConsoleHost(const ConsoleHost &) = delete;
    ConsoleHost &operator=(const ConsoleHost &) = delete;
    bool Initialize(bool private_windows_console = false);
    void Shutdown();
    void Poll(std::vector<std::string> &commands);
    bool Write(std::string_view text, bool error = false);
    bool Shutdown_Requested() const;
    bool Input_Closed() const;
    const std::string &Last_Error() const;
private:
    struct Impl;
    std::unique_ptr<Impl> State;
};
}
