/*
**  Command & Conquer Renegade(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ChunkLoadClass;

namespace wdump
{

struct FieldRow
{
    std::string name;
    std::string type;
    std::string value;
};

struct Chunk
{
    uint32_t id = 0;
    uint32_t length = 0;
    std::vector<std::uint8_t> data;
    std::vector<std::unique_ptr<Chunk>> children;
};

class ChunkFile
{
public:
    bool load(const std::string &path);
    void clear();

    const std::vector<std::unique_ptr<Chunk>> &roots() const { return _roots; }

private:
    std::unique_ptr<Chunk> load_chunk(ChunkLoadClass &cload);

    std::vector<std::unique_ptr<Chunk>> _roots;
};

// Render the raw chunk bytes into a simple hex+ASCII view; useful for UI panes.
std::string build_hex_view(const Chunk &chunk, std::size_t bytes_per_line = 16);
// Lookup a display name for known chunk IDs; returns nullptr for unknown values.
const char *chunk_name(uint32_t id);
// Build a set of human-readable fields for the selected chunk.
std::vector<FieldRow> describe_chunk(const Chunk &chunk);

} // namespace wdump
