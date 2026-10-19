// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenW3D Contributors.
#pragma once

#include <deque>
#include <string>
#include <string_view>

namespace OpenW3D {

// Shared by terminal and redirected input. Never execute a truncated line.
class ConsoleCommandLine {
public:
    static constexpr size_t MaxBytes = 200;
    void Feed(std::string_view bytes)
    {
        for (char ch : bytes) {
            if (ch == '\n' && LastWasCR) { LastWasCR = false; continue; }
            LastWasCR = ch == '\r';
            if (ch == '\r' || ch == '\n') {
                Finish();
            } else if (ch == '\b' || ch == 127) {
                Backspace();
            } else if (!Discarding) {
                if (ch == '\0' || Line.size() == MaxBytes) {
                    Line.clear();
                    Discarding = true;
                    ++Rejected;
                } else {
                    Line += ch;
                }
            }
        }
    }
    bool Backspace()
    {
        if (Discarding || Line.empty()) return false;
        size_t start = Line.size() - 1;
        while (start > 0 && (static_cast<unsigned char>(Line[start]) & 0xc0) == 0x80) --start;
        Line.resize(start);
        return true;
    }
    void Finish()
    {
        if (!Discarding && Line.find_first_not_of(" \t") != std::string::npos) Commands.push_back(Line);
        Line.clear();
        Discarding = false;
    }
    bool Pop(std::string &command)
    {
        if (Commands.empty()) return false;
        command = std::move(Commands.front());
        Commands.pop_front();
        return true;
    }
    bool HasCommands() const { return !Commands.empty(); }
    unsigned TakeRejected() { unsigned count = Rejected; Rejected = 0; return count; }
private:
    std::string Line;
    std::deque<std::string> Commands;
    bool Discarding = false;
    bool LastWasCR = false;
    unsigned Rejected = 0;
};
}
