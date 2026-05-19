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

#include "wdump_core.h"

#include "chunkio.h"
#include "rawfile.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace wdump
{
namespace
{

std::unique_ptr<Chunk> LoadChunk(ChunkLoadClass &cload)
{
    auto chunk = std::make_unique<Chunk>();
    chunk->id = cload.Cur_Chunk_ID();
    chunk->length = cload.Cur_Chunk_Length();

    if (cload.Contains_Chunks())
    {
        while (cload.Open_Chunk())
        {
            if (auto child = LoadChunk(cload))
            {
                chunk->children.push_back(std::move(child));
            }
            cload.Close_Chunk();
        }
    }
    else if (chunk->length > 0)
    {
        chunk->data.resize(chunk->length);
        const auto bytes_read = cload.Read(chunk->data.data(), chunk->data.size());
        if (bytes_read < chunk->data.size())
        {
            chunk->data.resize(bytes_read);
        }
    }

    return chunk;
}

} // namespace

bool ChunkFile::load(const std::string &path)
{
    clear();

    RawFileClass file(path.c_str());
    if (!file.Open(path.c_str()))
    {
        return false;
    }

    ChunkLoadClass cload(&file);
    while (cload.Open_Chunk())
    {
        if (auto chunk = load_chunk(cload))
        {
            _roots.push_back(std::move(chunk));
        }
        cload.Close_Chunk();
    }

    file.Close();
    return true;
}

void ChunkFile::clear()
{
    _roots.clear();
}

std::unique_ptr<Chunk> ChunkFile::load_chunk(ChunkLoadClass &cload)
{
    return LoadChunk(cload);
}

std::string build_hex_view(const Chunk &chunk, std::size_t bytes_per_line)
{
    if (chunk.data.empty())
    {
        std::ostringstream message;
        message << "This chunk is a wrapper chunk for other chunks. It's total length is "
                << chunk.length;
        return message.str();
    }

    std::ostringstream out;
    const auto &data = chunk.data;

    for (std::size_t i = 0; i < data.size(); i += bytes_per_line)
    {
        const auto line_bytes = std::min<std::size_t>(bytes_per_line, data.size() - i);

        for (std::size_t j = 0; j < line_bytes; ++j)
        {
            out.width(2);
            out.fill('0');
            out << std::hex << static_cast<int>(data[i + j]) << ' ';
        }

        // pad the hex field so ASCII column lines up
        if (line_bytes < bytes_per_line)
        {
            out << std::string((bytes_per_line - line_bytes) * 3, ' ');
        }

        out << "  ";
        for (std::size_t j = 0; j < line_bytes; ++j)
        {
            const auto c = data[i + j];
            if (c >= 32 && c <= 192)
            {
                out << static_cast<char>(c);
            }
            else
            {
                out << '.';
            }
        }

        if (i + line_bytes < data.size())
        {
            out << '\n';
        }
    }

    return out.str();
}

} // namespace wdump
