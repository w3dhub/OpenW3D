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
