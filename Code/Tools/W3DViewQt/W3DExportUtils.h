/*
** Copyright 2026 OpenW3D contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed WITHOUT ANY WARRANTY; without even the
** implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <QString>

#include <cstdint>
#include <functional>

class ChunkSaveClass;

namespace W3DExportUtils
{
using ChunkWriter = std::function<bool(ChunkSaveClass &)>;

bool SaveChunkFileAtomically(const QString &target_path,
                             std::uint32_t expected_top_level_chunk,
                             const ChunkWriter &writer,
                             QString *error_message = nullptr);
}
