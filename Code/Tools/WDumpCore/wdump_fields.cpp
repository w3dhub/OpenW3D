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

#include "vector3i.h"
#include "w3d_file.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace wdump
{
namespace
{
template <typename T>
bool ReadStruct(const Chunk &chunk, T &out)
{
    if (chunk.data.size() < sizeof(T)) {
        return false;
    }
    std::memcpy(&out, chunk.data.data(), sizeof(T));
    return true;
}

std::size_t CStrLen(const char *data, std::size_t max_len)
{
    if (!data || max_len == 0) {
        return 0;
    }
    const void *end = std::memchr(data, 0, max_len);
    if (!end) {
        return max_len;
    }
    return static_cast<const char *>(end) - data;
}

std::string StringFromBytes(const std::uint8_t *data, std::size_t size)
{
    if (!data || size == 0) {
        return std::string();
    }
    const char *cdata = reinterpret_cast<const char *>(data);
    return std::string(cdata, CStrLen(cdata, size));
}

std::string StringFromFixed(const char *data, std::size_t size)
{
    if (!data || size == 0) {
        return std::string();
    }
    return std::string(data, CStrLen(data, size));
}

std::string FormatHex(std::uint32_t value)
{
    std::ostringstream out;
    out << "0x" << std::uppercase << std::hex << value;
    return out.str();
}

std::string FormatVersion(std::uint32_t value)
{
    std::ostringstream out;
    out << W3D_GET_MAJOR_VERSION(value) << "." << W3D_GET_MINOR_VERSION(value);
    return out.str();
}

std::string FormatFloat(float32 value)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << value;
    return out.str();
}

std::string FormatFloatArray(const float32 *values, std::size_t count)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3);
    for (std::size_t i = 0; i < count; ++i) {
        if (i) {
            out << ' ';
        }
        out << values[i];
    }
    return out.str();
}

std::string FormatVec3(const W3dVectorStruct &vec)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << vec.X << " " << vec.Y << " " << vec.Z;
    return out.str();
}

std::string FormatQuaternion(const W3dQuaternionStruct &quat)
{
    return FormatFloatArray(quat.Q, 4);
}

std::string FormatRGB(const W3dRGBStruct &color)
{
    std::ostringstream out;
    out << static_cast<unsigned int>(color.R) << " " << static_cast<unsigned int>(color.G) << " "
        << static_cast<unsigned int>(color.B);
    return out.str();
}

std::string FormatRGBArray(const W3dRGBStruct *values, std::size_t count)
{
    std::ostringstream out;
    for (std::size_t i = 0; i < count; ++i) {
        if (i) {
            out << ' ';
        }
        out << "(" << static_cast<unsigned int>(values[i].R) << " " << static_cast<unsigned int>(values[i].G)
            << " " << static_cast<unsigned int>(values[i].B) << ")";
    }
    return out.str();
}

std::string FormatRGBA(const W3dRGBAStruct &color)
{
    std::ostringstream out;
    out << static_cast<unsigned int>(color.R) << " " << static_cast<unsigned int>(color.G) << " "
        << static_cast<unsigned int>(color.B) << " " << static_cast<unsigned int>(color.A);
    return out.str();
}

std::string FormatTexCoord(const W3dTexCoordStruct &coord)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << coord.U << " " << coord.V;
    return out.str();
}

std::string FormatVector3i(const Vector3i &value)
{
    std::ostringstream out;
    out << value.I << " " << value.J << " " << value.K;
    return out.str();
}

template <typename T, std::size_t N>
constexpr std::size_t ArrayCount(const T (&)[N])
{
    return N;
}

template <typename T>
std::string FormatUIntArray(const T *values, std::size_t count)
{
    std::ostringstream out;
    for (std::size_t i = 0; i < count; ++i) {
        if (i) {
            out << ' ';
        }
        out << static_cast<unsigned long long>(values[i]);
    }
    return out.str();
}

void AddRow(std::vector<FieldRow> &rows, const char *name, const char *type, const std::string &value)
{
    rows.push_back(FieldRow{name ? name : std::string(), type ? type : std::string(), value});
}

void AddFlag(std::vector<FieldRow> &rows, const char *group, const char *flag)
{
    AddRow(rows, group, "flag", flag ? flag : "");
}

void AddSubitems(std::vector<FieldRow> &rows, const Chunk &chunk)
{
    for (const auto &child : chunk.children) {
        const char *name = chunk_name(child->id);
        std::string label = name ? name : ("Unknown " + FormatHex(child->id));
        AddRow(rows, label.c_str(), "chunk", "");
    }
}

template <typename T>
void AddArrayCount(std::vector<FieldRow> &rows, const Chunk &chunk, const char *label)
{
    if (chunk.data.empty()) {
        AddRow(rows, label, "count", "0");
        return;
    }
    const std::size_t count = chunk.data.size() / sizeof(T);
    AddRow(rows, label, "count", std::to_string(count));
}

template <typename T>
std::size_t AddArrayCountValue(std::vector<FieldRow> &rows, const Chunk &chunk, const char *label)
{
    const std::size_t count = chunk.data.size() / sizeof(T);
    AddRow(rows, label, "uint32", std::to_string(count));
    return count;
}

void AddMeshAttributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Attributes", "uint32", FormatHex(attributes));

    switch (attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) {
    case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL");
        break;
    case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED");
        break;
    case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN");
        break;
    case W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX");
        break;
    case W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX");
        break;
    case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
        AddFlag(rows, "Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED");
        break;
    default:
        break;
    }

    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_VIS) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VIS");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_CAMERA) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_CAMERA");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE");
    if (attributes & W3D_MESH_FLAG_HIDDEN) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_HIDDEN");
    if (attributes & W3D_MESH_FLAG_TWO_SIDED) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_TWO_SIDED");
    if (attributes & W3D_MESH_FLAG_CAST_SHADOW) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_CAST_SHADOW");
    if (attributes & W3D_MESH_FLAG_SHATTERABLE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_SHATTERABLE");
    if (attributes & W3D_MESH_FLAG_NPATCHABLE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_NPATCHABLE");

    if (attributes & W3D_MESH_FLAG_PRELIT_UNLIT) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_PRELIT_UNLIT");
    if (attributes & W3D_MESH_FLAG_PRELIT_VERTEX) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_PRELIT_VERTEX");
    if (attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS");
    if (attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE");
}

void AddLegacyMeshAttributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Attributes", "uint32", FormatHex(attributes));
    if (attributes & W3D_MESH_FLAG_COLLISION_BOX) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_BOX");
    if (attributes & W3D_MESH_FLAG_SKIN) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_SKIN");
    if (attributes & W3D_MESH_FLAG_SHADOW) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_SHADOW");
    if (attributes & W3D_MESH_FLAG_ALIGNED) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_ALIGNED");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL");
    if (attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) AddFlag(rows, "Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE");
}

void AddVertexChannels(std::vector<FieldRow> &rows, std::uint32_t channels)
{
    AddRow(rows, "VertexChannels", "uint32", FormatHex(channels));
    if (channels & W3D_VERTEX_CHANNEL_LOCATION) AddFlag(rows, "VertexChannels", "W3D_VERTEX_CHANNEL_LOCATION");
    if (channels & W3D_VERTEX_CHANNEL_NORMAL) AddFlag(rows, "VertexChannels", "W3D_VERTEX_CHANNEL_NORMAL");
    if (channels & W3D_VERTEX_CHANNEL_TEXCOORD) AddFlag(rows, "VertexChannels", "W3D_VERTEX_CHANNEL_TEXCOORD");
    if (channels & W3D_VERTEX_CHANNEL_COLOR) AddFlag(rows, "VertexChannels", "W3D_VERTEX_CHANNEL_COLOR");
    if (channels & W3D_VERTEX_CHANNEL_BONEID) AddFlag(rows, "VertexChannels", "W3D_VERTEX_CHANNEL_BONEID");
}

void AddFaceChannels(std::vector<FieldRow> &rows, std::uint32_t channels)
{
    AddRow(rows, "FaceChannels", "uint32", FormatHex(channels));
    if (channels & W3D_FACE_CHANNEL_FACE) AddFlag(rows, "FaceChannels", "W3D_FACE_CHANNEL_FACE");
}

void AddTextureAttributes(std::vector<FieldRow> &rows, std::uint16_t attributes)
{
    AddRow(rows, "Attributes", "uint16", FormatHex(attributes));
    if (attributes & W3DTEXTURE_PUBLISH) AddFlag(rows, "Attributes", "W3DTEXTURE_PUBLISH");
    if (attributes & W3DTEXTURE_NO_LOD) AddFlag(rows, "Attributes", "W3DTEXTURE_NO_LOD");
    if (attributes & W3DTEXTURE_CLAMP_U) AddFlag(rows, "Attributes", "W3DTEXTURE_CLAMP_U");
    if (attributes & W3DTEXTURE_CLAMP_V) AddFlag(rows, "Attributes", "W3DTEXTURE_CLAMP_V");
    if (attributes & W3DTEXTURE_ALPHA_BITMAP) AddFlag(rows, "Attributes", "W3DTEXTURE_ALPHA_BITMAP");
}

void AddMaterial3Attributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Attributes", "uint32", FormatHex(attributes));
    if (attributes & W3DMATERIAL_USE_ALPHA) AddFlag(rows, "Attributes", "W3DMATERIAL_USE_ALPHA");
    if (attributes & W3DMATERIAL_USE_SORTING) AddFlag(rows, "Attributes", "W3DMATERIAL_USE_SORTING");
    if (attributes & W3DMATERIAL_HINT_DIT_OVER_DCT) AddFlag(rows, "Attributes", "W3DMATERIAL_HINT_DIT_OVER_DCT");
    if (attributes & W3DMATERIAL_HINT_SIT_OVER_SCT) AddFlag(rows, "Attributes", "W3DMATERIAL_HINT_SIT_OVER_SCT");
    if (attributes & W3DMATERIAL_HINT_DIT_OVER_DIG) AddFlag(rows, "Attributes", "W3DMATERIAL_HINT_DIT_OVER_DIG");
    if (attributes & W3DMATERIAL_HINT_SIT_OVER_SIG) AddFlag(rows, "Attributes", "W3DMATERIAL_HINT_SIT_OVER_SIG");
    if (attributes & W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA) AddFlag(rows, "Attributes", "W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA");
    if (attributes & W3DMATERIAL_PSX_TRANS_100) AddFlag(rows, "Attributes", "W3DMATERIAL_PSX_TRANS_100");
    if (attributes & W3DMATERIAL_PSX_TRANS_50) AddFlag(rows, "Attributes", "W3DMATERIAL_PSX_TRANS_50");
    if (attributes & W3DMATERIAL_PSX_TRANS_25) AddFlag(rows, "Attributes", "W3DMATERIAL_PSX_TRANS_25");
    if (attributes & W3DMATERIAL_PSX_TRANS_MINUS_100) AddFlag(rows, "Attributes", "W3DMATERIAL_PSX_TRANS_MINUS_100");
    if (attributes & W3DMATERIAL_PSX_NO_RT_LIGHTING) AddFlag(rows, "Attributes", "W3DMATERIAL_PSX_NO_RT_LIGHTING");
}

void AddVertexMaterialAttributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Material.Attributes", "uint32", FormatHex(attributes));
    if (attributes & W3DVERTMAT_USE_DEPTH_CUE) AddFlag(rows, "Material.Attributes", "W3DVERTMAT_USE_DEPTH_CUE");
    if (attributes & W3DVERTMAT_ARGB_EMISSIVE_ONLY) AddFlag(rows, "Material.Attributes", "W3DVERTMAT_ARGB_EMISSIVE_ONLY");
    if (attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE) AddFlag(rows, "Material.Attributes", "W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE");
    if (attributes & W3DVERTMAT_DEPTH_CUE_TO_ALPHA) AddFlag(rows, "Material.Attributes", "W3DVERTMAT_DEPTH_CUE_TO_ALPHA");

    switch (attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) {
    case W3DVERTMAT_STAGE0_MAPPING_UV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_UV");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_SCREEN:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCREEN");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_SCALE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCALE");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_GRID:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_ROTATE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ROTATE");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_RANDOM:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_RANDOM");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_EDGE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_EDGE");
        break;
    case W3DVERTMAT_STAGE0_MAPPING_BUMPENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_BUMPENV");
        break;
    default:
        break;
    }

    switch (attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) {
    case W3DVERTMAT_STAGE1_MAPPING_UV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_UV");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_SCREEN:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCREEN");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_SCALE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCALE");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_GRID:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_ROTATE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ROTATE");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_RANDOM:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_RANDOM");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_EDGE:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_EDGE");
        break;
    case W3DVERTMAT_STAGE1_MAPPING_BUMPENV:
        AddFlag(rows, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_BUMPENV");
        break;
    default:
        break;
    }

    if (attributes & W3DVERTMAT_PSX_MASK) {
        if (attributes & W3DVERTMAT_PSX_NO_RT_LIGHTING) {
            AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_NO_RT_LIGHTING");
        } else {
            switch (attributes & W3DVERTMAT_PSX_TRANS_MASK) {
            case W3DVERTMAT_PSX_TRANS_NONE:
                AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_NONE");
                break;
            case W3DVERTMAT_PSX_TRANS_100:
                AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_100");
                break;
            case W3DVERTMAT_PSX_TRANS_50:
                AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_50");
                break;
            case W3DVERTMAT_PSX_TRANS_25:
                AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_25");
                break;
            case W3DVERTMAT_PSX_TRANS_MINUS_100:
                AddFlag(rows, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_MINUS_100");
                break;
            default:
                break;
            }
        }
    }
}

void AddLightAttributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Attributes", "uint32", FormatHex(attributes));
    switch (attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK) {
    case W3D_LIGHT_ATTRIBUTE_POINT:
        AddFlag(rows, "Attributes", "W3D_LIGHT_ATTRIBUTE_POINT");
        break;
    case W3D_LIGHT_ATTRIBUTE_SPOT:
        AddFlag(rows, "Attributes", "W3D_LIGHT_ATTRIBUTE_SPOT");
        break;
    case W3D_LIGHT_ATTRIBUTE_DIRECTIONAL:
        AddFlag(rows, "Attributes", "W3D_LIGHT_ATTRIBUTE_DIRECTIONAL");
        break;
    default:
        break;
    }
    if (attributes & W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS) AddFlag(rows, "Attributes", "W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS");
}

void AddBoxAttributes(std::vector<FieldRow> &rows, std::uint32_t attributes)
{
    AddRow(rows, "Attributes", "uint32", FormatHex(attributes));
    if (attributes & W3D_BOX_ATTRIBUTE_ORIENTED) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBUTE_ORIENTED");
    if (attributes & W3D_BOX_ATTRIBUTE_ALIGNED) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBUTE_ALIGNED");
    if (attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL");
    if (attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE");
    if (attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS");
    if (attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA");
    if (attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE) AddFlag(rows, "Attributes", "W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE");
}

void AddTransformRows(std::vector<FieldRow> &rows, const float32 transform[4][3])
{
    for (int row = 0; row < 4; ++row) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(3)
            << transform[row][0] << " "
            << transform[row][1] << " "
            << transform[row][2];
        std::string label = "Transform[" + std::to_string(row) + "]";
        AddRow(rows, label.c_str(), "float3", out.str());
    }
}

void AddEmitterVolume(std::vector<FieldRow> &rows, const char *label, const W3dVolumeRandomizerStruct &vol)
{
    const std::string base = label ? label : "Volume";
    AddRow(rows, (base + ".ClassID").c_str(), "uint32", std::to_string(vol.ClassID));
    AddRow(rows, (base + ".Value1").c_str(), "float", FormatFloat(vol.Value1));
    AddRow(rows, (base + ".Value2").c_str(), "float", FormatFloat(vol.Value2));
    AddRow(rows, (base + ".Value3").c_str(), "float", FormatFloat(vol.Value3));
}

std::string LookupEnum(const char *const *table, std::size_t count, int value)
{
    if (value >= 0 && static_cast<std::size_t>(value) < count) {
        return table[value];
    }
    return std::to_string(value);
}

void AddShaderRows(std::vector<FieldRow> &rows, const std::string &base, const W3dShaderStruct &shader)
{
    static const char *kDepthCompare[] = {"Pass Never", "Pass Less", "Pass Equal", "Pass Less or Equal",
                                          "Pass Greater", "Pass Not Equal", "Pass Greater or Equal", "Pass Always"};
    static const char *kDepthMask[] = {"Write Disable", "Write Enable"};
    static const char *kDestBlend[] = {"Zero", "One", "Src Color", "One Minus Src Color", "Src Alpha",
                                       "One Minus Src Alpha", "Src Color Prefog"};
    static const char *kPriGradient[] = {"Disable", "Modulate", "Add", "Bump-Environment"};
    static const char *kSecGradient[] = {"Disable", "Enable"};
    static const char *kSrcBlend[] = {"Zero", "One", "Src Alpha", "One Minus Src Alpha"};
    static const char *kTexturing[] = {"Disable", "Enable"};
    static const char *kDetailColor[] = {"Disable", "Detail", "Scale", "InvScale", "Add", "Sub", "SubR", "Blend", "DetailBlend"};
    static const char *kDetailAlpha[] = {"Disable", "Detail", "Scale", "InvScale"};
    static const char *kAlphaTest[] = {"Alpha Test Disable", "Alpha Test Enable"};

    AddRow(rows, (base + ".DepthCompare").c_str(), "string",
           LookupEnum(kDepthCompare, ArrayCount(kDepthCompare), W3d_Shader_Get_Depth_Compare(&shader)));
    AddRow(rows, (base + ".DepthMask").c_str(), "string",
           LookupEnum(kDepthMask, ArrayCount(kDepthMask), W3d_Shader_Get_Depth_Mask(&shader)));
    AddRow(rows, (base + ".DestBlend").c_str(), "string",
           LookupEnum(kDestBlend, ArrayCount(kDestBlend), W3d_Shader_Get_Dest_Blend_Func(&shader)));
    AddRow(rows, (base + ".PriGradient").c_str(), "string",
           LookupEnum(kPriGradient, ArrayCount(kPriGradient), W3d_Shader_Get_Pri_Gradient(&shader)));
    AddRow(rows, (base + ".SecGradient").c_str(), "string",
           LookupEnum(kSecGradient, ArrayCount(kSecGradient), W3d_Shader_Get_Sec_Gradient(&shader)));
    AddRow(rows, (base + ".SrcBlend").c_str(), "string",
           LookupEnum(kSrcBlend, ArrayCount(kSrcBlend), W3d_Shader_Get_Src_Blend_Func(&shader)));
    AddRow(rows, (base + ".Texturing").c_str(), "string",
           LookupEnum(kTexturing, ArrayCount(kTexturing), W3d_Shader_Get_Texturing(&shader)));
    AddRow(rows, (base + ".DetailColor").c_str(), "string",
           LookupEnum(kDetailColor, ArrayCount(kDetailColor), W3d_Shader_Get_Detail_Color_Func(&shader)));
    AddRow(rows, (base + ".DetailAlpha").c_str(), "string",
           LookupEnum(kDetailAlpha, ArrayCount(kDetailAlpha), W3d_Shader_Get_Detail_Alpha_Func(&shader)));
    AddRow(rows, (base + ".AlphaTest").c_str(), "string",
           LookupEnum(kAlphaTest, ArrayCount(kAlphaTest), W3d_Shader_Get_Alpha_Test(&shader)));
}

void AddPS2ShaderRows(std::vector<FieldRow> &rows, const std::string &base, const W3dPS2ShaderStruct &shader)
{
    static const char *kDepthCompare[] = {"Pass Never", "Pass Less", "Pass Always", "Pass Less or Equal"};
    static const char *kDepthMask[] = {"Write Disable", "Write Enable"};
    static const char *kPriGradient[] = {"Disable", "Modulate", "Highlight", "Highlight2"};
    static const char *kTexturing[] = {"Disable", "Enable"};
    static const char *kABlend[] = {"Src Color", "Dest Color", "Zero"};
    static const char *kCBlend[] = {"Src Alpha", "Dest Alpha", "One"};

    AddRow(rows, (base + ".DepthCompare").c_str(), "string",
           LookupEnum(kDepthCompare, ArrayCount(kDepthCompare), W3d_Shader_Get_Depth_Compare(&shader)));
    AddRow(rows, (base + ".DepthMask").c_str(), "string",
           LookupEnum(kDepthMask, ArrayCount(kDepthMask), W3d_Shader_Get_Depth_Mask(&shader)));
    AddRow(rows, (base + ".PriGradient").c_str(), "string",
           LookupEnum(kPriGradient, ArrayCount(kPriGradient), W3d_Shader_Get_Pri_Gradient(&shader)));
    AddRow(rows, (base + ".Texturing").c_str(), "string",
           LookupEnum(kTexturing, ArrayCount(kTexturing), W3d_Shader_Get_Texturing(&shader)));
    AddRow(rows, (base + ".AParam").c_str(), "string",
           LookupEnum(kABlend, ArrayCount(kABlend), W3d_Shader_Get_PS2_Param_A(&shader)));
    AddRow(rows, (base + ".BParam").c_str(), "string",
           LookupEnum(kABlend, ArrayCount(kABlend), W3d_Shader_Get_PS2_Param_B(&shader)));
    AddRow(rows, (base + ".CParam").c_str(), "string",
           LookupEnum(kCBlend, ArrayCount(kCBlend), W3d_Shader_Get_PS2_Param_C(&shader)));
    AddRow(rows, (base + ".DParam").c_str(), "string",
           LookupEnum(kABlend, ArrayCount(kABlend), W3d_Shader_Get_PS2_Param_D(&shader)));
}

int GetBit(const std::uint8_t *data, std::size_t bit)
{
    if (!data) {
        return 0;
    }
    std::uint8_t mask = static_cast<std::uint8_t>(1u << (bit % 8));
    return (data[bit / 8] & mask) ? 1 : 0;
}

} // namespace

std::vector<FieldRow> describe_chunk(const Chunk &chunk)
{
    std::vector<FieldRow> rows;

    switch (chunk.id) {
    case W3D_CHUNK_MESH:
    case W3D_CHUNK_DAMAGE:
    case W3D_CHUNK_MATERIALS3:
    case W3D_CHUNK_MATERIAL3:
    case W3D_CHUNK_MATERIAL3_DC_MAP:
    case W3D_CHUNK_MATERIAL3_DI_MAP:
    case W3D_CHUNK_MATERIAL3_SC_MAP:
    case W3D_CHUNK_MATERIAL3_SI_MAP:
    case W3D_CHUNK_VERTEX_MATERIALS:
    case W3D_CHUNK_VERTEX_MATERIAL:
    case W3D_CHUNK_TEXTURES:
    case W3D_CHUNK_TEXTURE:
    case W3D_CHUNK_MATERIAL_PASS:
    case W3D_CHUNK_TEXTURE_STAGE:
    case W3D_CHUNK_AABTREE:
    case W3D_CHUNK_HIERARCHY:
    case W3D_CHUNK_ANIMATION:
    case W3D_CHUNK_HMODEL:
    case W3D_CHUNK_LODMODEL:
    case W3D_CHUNK_COLLECTION:
    case W3D_CHUNK_LIGHT:
    case W3D_CHUNK_EMITTER:
    case W3D_CHUNK_AGGREGATE:
    case W3D_CHUNK_HLOD:
    case W3D_CHUNK_HLOD_LOD_ARRAY:
    case W3D_CHUNK_HLOD_AGGREGATE_ARRAY:
    case W3D_CHUNK_HLOD_PROXY_ARRAY:
    case W3D_CHUNK_PRELIT_UNLIT:
    case W3D_CHUNK_PRELIT_VERTEX:
    case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
    case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
    case W3D_CHUNK_LIGHTSCAPE:
    case W3D_CHUNK_LIGHTSCAPE_LIGHT:
    case W3D_CHUNK_DAZZLE:
        AddSubitems(rows, chunk);
        break;
    case W3D_CHUNK_MESH_USER_TEXT:
        AddRow(rows, "UserText", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_MATERIAL3_NAME:
        AddRow(rows, "Material Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_MAP3_FILENAME:
        AddRow(rows, "Texture Filename", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_VERTEX_MATERIAL_NAME:
        AddRow(rows, "Vertex Material Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_TEXTURE_NAME:
        AddRow(rows, "Texture Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_COLLECTION_OBJ_NAME:
        AddRow(rows, "Render Object Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_DAZZLE_NAME:
        AddRow(rows, "Dazzle Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_DAZZLE_TYPENAME:
        AddRow(rows, "Dazzle Type Name", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_MESH_HEADER:
    {
        W3dMeshHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "MeshName", "string", StringFromFixed(data.MeshName, W3D_NAME_LEN));
        AddLegacyMeshAttributes(rows, data.Attributes);
        AddRow(rows, "NumTris", "uint32", std::to_string(data.NumTris));
        AddRow(rows, "NumQuads", "uint32", std::to_string(data.NumQuads));
        AddRow(rows, "NumSrTris", "uint32", std::to_string(data.NumSrTris));
        AddRow(rows, "NumPovQuads", "uint32", std::to_string(data.NumPovQuads));
        AddRow(rows, "NumVertices", "uint32", std::to_string(data.NumVertices));
        AddRow(rows, "NumNormals", "uint32", std::to_string(data.NumNormals));
        AddRow(rows, "NumSrNormals", "uint32", std::to_string(data.NumSrNormals));
        AddRow(rows, "NumTexCoords", "uint32", std::to_string(data.NumTexCoords));
        AddRow(rows, "NumMaterials", "uint32", std::to_string(data.NumMaterials));
        AddRow(rows, "NumVertColors", "uint32", std::to_string(data.NumVertColors));
        AddRow(rows, "NumVertInfluences", "uint32", std::to_string(data.NumVertInfluences));
        AddRow(rows, "NumDamageStages", "uint32", std::to_string(data.NumDamageStages));
        AddRow(rows, "FutureCounts", "uint32[5]", FormatUIntArray(data.FutureCounts, 5));
        AddRow(rows, "LODMin", "float", FormatFloat(data.LODMin));
        AddRow(rows, "LODMax", "float", FormatFloat(data.LODMax));
        AddRow(rows, "Min", "vec3", FormatVec3(data.Min));
        AddRow(rows, "Max", "vec3", FormatVec3(data.Max));
        AddRow(rows, "SphCenter", "vec3", FormatVec3(data.SphCenter));
        AddRow(rows, "SphRadius", "float", FormatFloat(data.SphRadius));
        AddRow(rows, "Translation", "vec3", FormatVec3(data.Translation));
        AddRow(rows, "Rotation", "float[9]", FormatFloatArray(data.Rotation, 9));
        AddRow(rows, "MassCenter", "vec3", FormatVec3(data.MassCenter));
        AddRow(rows, "Inertia", "float[9]", FormatFloatArray(data.Inertia, 9));
        AddRow(rows, "Volume", "float", FormatFloat(data.Volume));
        AddRow(rows, "HierarchyTreeName", "string", StringFromFixed(data.HierarchyTreeName, W3D_NAME_LEN));
        AddRow(rows, "HierarchyModelName", "string", StringFromFixed(data.HierarchyModelName, W3D_NAME_LEN));
        AddRow(rows, "FutureUse", "uint32[24]", FormatUIntArray(data.FutureUse, 24));
        break;
    }
    case O_W3D_CHUNK_MATERIALS:
    {
        const auto count = AddArrayCountValue<W3dMaterialStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMaterialStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Material[" + std::to_string(i) + "].";
            AddRow(rows, (base + "MaterialName").c_str(), "string",
                   StringFromFixed(data[i].MaterialName, W3D_NAME_LEN));
            AddRow(rows, (base + "PrimaryName").c_str(), "string",
                   StringFromFixed(data[i].PrimaryName, W3D_NAME_LEN));
            AddRow(rows, (base + "SecondaryName").c_str(), "string",
                   StringFromFixed(data[i].SecondaryName, W3D_NAME_LEN));
            AddRow(rows, (base + "RenderFlags").c_str(), "uint32", std::to_string(data[i].RenderFlags));
            AddRow(rows, (base + "Red").c_str(), "uint8", std::to_string(data[i].Red));
            AddRow(rows, (base + "Green").c_str(), "uint8", std::to_string(data[i].Green));
            AddRow(rows, (base + "Blue").c_str(), "uint8", std::to_string(data[i].Blue));
        }
        break;
    }
    case O_W3D_CHUNK_MATERIALS2:
    {
        const auto count = AddArrayCountValue<W3dMaterial2Struct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMaterial2Struct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Material[" + std::to_string(i) + "].";
            AddRow(rows, (base + "MaterialName").c_str(), "string",
                   StringFromFixed(data[i].MaterialName, W3D_NAME_LEN));
            AddRow(rows, (base + "PrimaryName").c_str(), "string",
                   StringFromFixed(data[i].PrimaryName, W3D_NAME_LEN));
            AddRow(rows, (base + "SecondaryName").c_str(), "string",
                   StringFromFixed(data[i].SecondaryName, W3D_NAME_LEN));
            AddRow(rows, (base + "RenderFlags").c_str(), "uint32", std::to_string(data[i].RenderFlags));
            AddRow(rows, (base + "Red").c_str(), "uint8", std::to_string(data[i].Red));
            AddRow(rows, (base + "Green").c_str(), "uint8", std::to_string(data[i].Green));
            AddRow(rows, (base + "Blue").c_str(), "uint8", std::to_string(data[i].Blue));
            AddRow(rows, (base + "Alpha").c_str(), "uint8", std::to_string(data[i].Alpha));
            AddRow(rows, (base + "PrimaryNumFrames").c_str(), "uint16", std::to_string(data[i].PrimaryNumFrames));
            AddRow(rows, (base + "SecondaryNumFrames").c_str(), "uint16", std::to_string(data[i].SecondaryNumFrames));
        }
        break;
    }
    case O_W3D_CHUNK_TRIANGLES:
        AddRow(rows, "Obsolete structure", "string", "");
        break;
    case O_W3D_CHUNK_QUADRANGLES:
        AddRow(rows, "Outdated structure", "string", "");
        break;
    case O_W3D_CHUNK_POV_TRIANGLES:
    case O_W3D_CHUNK_POV_QUADRANGLES:
        AddRow(rows, "Contact Greg if you need to look at this!", "string", "unsupported");
        break;
    case O_W3D_CHUNK_SURRENDER_TRIANGLES:
    {
        const auto count = AddArrayCountValue<W3dSurrenderTriStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dSurrenderTriStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Triangle[" + std::to_string(i) + "].";
            AddRow(rows, (base + "Attributes").c_str(), "uint32", std::to_string(data[i].Attributes));
            AddRow(rows, (base + "Gouraud").c_str(), "RGB[3]", FormatRGBArray(data[i].Gouraud, 3));
            AddRow(rows, (base + "VertexIndices").c_str(), "uint32[3]", FormatUIntArray(data[i].Vindex, 3));
            AddRow(rows, (base + "MaterialIdx").c_str(), "uint32", std::to_string(data[i].MaterialIdx));
            AddRow(rows, (base + "Normal").c_str(), "vector", FormatVec3(data[i].Normal));
            for (std::size_t t = 0; t < 3; ++t) {
                std::string label = base + "TexCoord[" + std::to_string(t) + "]";
                AddRow(rows, label.c_str(), "UV", FormatTexCoord(data[i].TexCoord[t]));
            }
        }
        break;
    }
    case W3D_CHUNK_DAMAGE_HEADER:
    {
        const auto count = AddArrayCountValue<W3dMeshDamageStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMeshDamageStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "DamageStruct[" + std::to_string(i) + "].";
            AddRow(rows, (base + "NumDamageMaterials").c_str(), "uint32", std::to_string(data[i].NumDamageMaterials));
            AddRow(rows, (base + "NumDamageVerts").c_str(), "uint32", std::to_string(data[i].NumDamageVerts));
            AddRow(rows, (base + "NumDamageColors").c_str(), "uint32", std::to_string(data[i].NumDamageColors));
            AddRow(rows, (base + "DamageIndex").c_str(), "uint32", std::to_string(data[i].DamageIndex));
        }
        break;
    }
    case W3D_CHUNK_DAMAGE_VERTICES:
    {
        const auto count = AddArrayCountValue<W3dMeshDamageVertexStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMeshDamageVertexStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "DamageVertexStruct[" + std::to_string(i) + "].";
            AddRow(rows, (base + "VertexIndex").c_str(), "uint32", std::to_string(data[i].VertexIndex));
            AddRow(rows, (base + "NewVertex").c_str(), "vec3", FormatVec3(data[i].NewVertex));
        }
        break;
    }
    case W3D_CHUNK_DAMAGE_COLORS:
    {
        const auto count = AddArrayCountValue<W3dMeshDamageColorStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMeshDamageColorStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "DamageColorStruct[" + std::to_string(i) + "].";
            AddRow(rows, (base + "VertexIndex").c_str(), "uint32", std::to_string(data[i].VertexIndex));
            AddRow(rows, (base + "NewColor").c_str(), "rgb", FormatRGB(data[i].NewColor));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
        AddRow(rows, "Stage0 Mapper Args:", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
        AddRow(rows, "Stage1 Mapper Args:", "string", StringFromBytes(chunk.data.data(), chunk.data.size()));
        break;
    case W3D_CHUNK_MESH_HEADER3:
    {
        W3dMeshHeader3Struct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "MeshName", "string", StringFromFixed(data.MeshName, W3D_NAME_LEN));
        AddRow(rows, "ContainerName", "string", StringFromFixed(data.ContainerName, W3D_NAME_LEN));
        AddMeshAttributes(rows, data.Attributes);
        AddRow(rows, "NumTris", "uint32", std::to_string(data.NumTris));
        AddRow(rows, "NumVertices", "uint32", std::to_string(data.NumVertices));
        AddRow(rows, "NumMaterials", "uint32", std::to_string(data.NumMaterials));
        AddRow(rows, "NumDamageStages", "uint32", std::to_string(data.NumDamageStages));
        if (data.SortLevel == SORT_LEVEL_NONE) {
            AddRow(rows, "SortLevel", "string", "NONE");
        } else {
            AddRow(rows, "SortLevel", "int32", std::to_string(data.SortLevel));
        }

        if ((data.Attributes & W3D_MESH_FLAG_PRELIT_MASK) != 0) {
            AddRow(rows, "PrelitVersion", "version", FormatVersion(data.PrelitVersion));
        } else {
            AddRow(rows, "PrelitVersion", "string", "N/A");
        }

        AddRow(rows, "FutureCounts[0]", "uint32", std::to_string(data.FutureCounts[0]));
        AddVertexChannels(rows, data.VertexChannels);
        AddFaceChannels(rows, data.FaceChannels);
        AddRow(rows, "Min", "vec3", FormatVec3(data.Min));
        AddRow(rows, "Max", "vec3", FormatVec3(data.Max));
        AddRow(rows, "SphCenter", "vec3", FormatVec3(data.SphCenter));
        AddRow(rows, "SphRadius", "float", FormatFloat(data.SphRadius));
        break;
    }
    case W3D_CHUNK_MATERIAL_INFO:
    {
        W3dMaterialInfoStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "PassCount", "uint32", std::to_string(data.PassCount));
        AddRow(rows, "VertexMaterialCount", "uint32", std::to_string(data.VertexMaterialCount));
        AddRow(rows, "ShaderCount", "uint32", std::to_string(data.ShaderCount));
        AddRow(rows, "TextureCount", "uint32", std::to_string(data.TextureCount));
        break;
    }
    case W3D_CHUNK_VERTEX_MATERIAL_INFO:
    {
        W3dVertexMaterialStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddVertexMaterialAttributes(rows, data.Attributes);
        AddRow(rows, "Material.Ambient", "rgb", FormatRGB(data.Ambient));
        AddRow(rows, "Material.Diffuse", "rgb", FormatRGB(data.Diffuse));
        AddRow(rows, "Material.Specular", "rgb", FormatRGB(data.Specular));
        AddRow(rows, "Material.Emissive", "rgb", FormatRGB(data.Emissive));
        AddRow(rows, "Material.Shininess", "float", FormatFloat(data.Shininess));
        AddRow(rows, "Material.Opacity", "float", FormatFloat(data.Opacity));
        AddRow(rows, "Material.Translucency", "float", FormatFloat(data.Translucency));
        break;
    }
    case W3D_CHUNK_SHADERS:
    {
        const auto count = AddArrayCountValue<W3dShaderStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dShaderStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "shader[" + std::to_string(i) + "]";
            AddShaderRows(rows, label, data[i]);
        }
        break;
    }
    case W3D_CHUNK_PS2_SHADERS:
    {
        const auto count = AddArrayCountValue<W3dPS2ShaderStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dPS2ShaderStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "shader[" + std::to_string(i) + "]";
            AddPS2ShaderRows(rows, label, data[i]);
        }
        break;
    }
    case W3D_CHUNK_MATERIAL3_INFO:
    {
        W3dMaterial3Struct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddMaterial3Attributes(rows, data.Attributes);
        AddRow(rows, "Diffuse Color", "rgb", FormatRGB(data.DiffuseColor));
        AddRow(rows, "Specular Color", "rgb", FormatRGB(data.SpecularColor));
        AddRow(rows, "Emissive Coefficients", "rgb", FormatRGB(data.EmissiveCoefficients));
        AddRow(rows, "Ambient Coefficients", "rgb", FormatRGB(data.AmbientCoefficients));
        AddRow(rows, "Diffuse Coefficients", "rgb", FormatRGB(data.DiffuseCoefficients));
        AddRow(rows, "Specular Coefficients", "rgb", FormatRGB(data.SpecularCoefficients));
        AddRow(rows, "Shininess", "float", FormatFloat(data.Shininess));
        AddRow(rows, "Opacity", "float", FormatFloat(data.Opacity));
        AddRow(rows, "Translucency", "float", FormatFloat(data.Translucency));
        AddRow(rows, "Fog Coefficient", "float", FormatFloat(data.FogCoeff));
        break;
    }
    case W3D_CHUNK_MAP3_INFO:
    {
        W3dMap3Struct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Mapping Type", "uint16", std::to_string(data.MappingType));
        AddRow(rows, "Frame Count", "uint16", std::to_string(data.FrameCount));
        AddRow(rows, "Frame Rate", "float", FormatFloat(data.FrameRate));
        break;
    }
    case W3D_CHUNK_TEXTURE_INFO:
    {
        W3dTextureInfoStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddTextureAttributes(rows, data.Attributes);
        AddRow(rows, "AnimType", "uint16", std::to_string(data.AnimType));
        AddRow(rows, "FrameCount", "uint32", std::to_string(data.FrameCount));
        AddRow(rows, "FrameRate", "float", FormatFloat(data.FrameRate));
        break;
    }
    case W3D_CHUNK_ANIMATION_HEADER:
    {
        W3dAnimHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "HierarchyName", "string", StringFromFixed(data.HierarchyName, W3D_NAME_LEN));
        AddRow(rows, "NumFrames", "uint32", std::to_string(data.NumFrames));
        AddRow(rows, "FrameRate", "uint32", std::to_string(data.FrameRate));
        break;
    }
    case W3D_CHUNK_ANIMATION_CHANNEL:
    {
        static const char *kChannelTypes[] = {"X Translation", "Y Translation", "Z Translation",
                                              "X Rotation", "Y Rotation", "Z Rotation", "Quaternion"};
        const std::size_t header_size = offsetof(W3dAnimChannelStruct, Data);
        if (chunk.data.size() < header_size) {
            break;
        }
        W3dAnimChannelStruct header{};
        std::memcpy(&header, chunk.data.data(), header_size);
        AddRow(rows, "FirstFrame", "uint16", std::to_string(header.FirstFrame));
        AddRow(rows, "LastFrame", "uint16", std::to_string(header.LastFrame));
        if (header.Flags >= ANIM_CHANNEL_X && header.Flags <= ANIM_CHANNEL_Q) {
            AddRow(rows, "ChannelType", "string",
                   kChannelTypes[header.Flags - ANIM_CHANNEL_X]);
        } else {
            AddRow(rows, "ChannelType", "uint16", std::to_string(header.Flags));
        }
        AddRow(rows, "Pivot", "uint16", std::to_string(header.Pivot));
        AddRow(rows, "VectorLen", "uint16", std::to_string(header.VectorLen));

        const float32 *values = reinterpret_cast<const float32 *>(chunk.data.data() + header_size);
        const std::size_t available = (chunk.data.size() - header_size) / sizeof(float32);
        std::size_t frame_count = 0;
        if (header.LastFrame >= header.FirstFrame) {
            frame_count = static_cast<std::size_t>(header.LastFrame - header.FirstFrame + 1);
        }
        const std::size_t expected = frame_count * header.VectorLen;
        const std::size_t count = std::min(available, expected);
        std::size_t index = 0;
        for (std::size_t frame = 0; frame < frame_count; ++frame) {
            for (std::size_t vidx = 0; vidx < header.VectorLen; ++vidx) {
                if (index >= count) {
                    frame = frame_count;
                    break;
                }
                std::string label = "Data[" + std::to_string(frame) + "][" + std::to_string(vidx) + "]";
                AddRow(rows, label.c_str(), "float", FormatFloat(values[index]));
                ++index;
            }
        }
        break;
    }
    case W3D_CHUNK_BIT_CHANNEL:
    {
        static const char *kChannelTypes[] = {"Visibility"};
        const std::size_t header_size = offsetof(W3dBitChannelStruct, Data);
        if (chunk.data.size() < header_size) {
            break;
        }
        W3dBitChannelStruct header{};
        std::memcpy(&header, chunk.data.data(), header_size);
        AddRow(rows, "FirstFrame", "uint16", std::to_string(header.FirstFrame));
        AddRow(rows, "LastFrame", "uint16", std::to_string(header.LastFrame));
        if (header.Flags == BIT_CHANNEL_VIS) {
            AddRow(rows, "ChannelType", "string", kChannelTypes[0]);
        } else {
            AddRow(rows, "ChannelType", "uint16", std::to_string(header.Flags));
        }
        AddRow(rows, "Pivot", "uint16", std::to_string(header.Pivot));
        AddRow(rows, "Default Value", "uint8", std::to_string(header.DefaultVal));

        const std::uint8_t *bits = chunk.data.data() + header_size;
        const std::size_t available_bits = (chunk.data.size() - header_size) * 8;
        std::size_t frame_count = 0;
        if (header.LastFrame >= header.FirstFrame) {
            frame_count = static_cast<std::size_t>(header.LastFrame - header.FirstFrame + 1);
        }
        const std::size_t count = std::min(available_bits, frame_count);
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Data[" + std::to_string(header.FirstFrame + i) + "]";
            AddRow(rows, label.c_str(), "uint8", std::to_string(GetBit(bits, i)));
        }
        break;
    }
    case W3D_CHUNK_COMPRESSED_ANIMATION_HEADER:
    {
        W3dCompressedAnimHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "HierarchyName", "string", StringFromFixed(data.HierarchyName, W3D_NAME_LEN));
        AddRow(rows, "NumFrames", "uint32", std::to_string(data.NumFrames));
        AddRow(rows, "FrameRate", "uint16", std::to_string(data.FrameRate));
        AddRow(rows, "Flavor", "uint16", std::to_string(data.Flavor));
        break;
    }
    case W3D_CHUNK_HIERARCHY_HEADER:
    {
        W3dHierarchyStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "NumPivots", "uint32", std::to_string(data.NumPivots));
        AddRow(rows, "Center", "vec3", FormatVec3(data.Center));
        break;
    }
    case W3D_CHUNK_HMODEL_HEADER:
    {
        W3dHModelHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "HierarchyName", "string", StringFromFixed(data.HierarchyName, W3D_NAME_LEN));
        AddRow(rows, "NumConnections", "uint16", std::to_string(data.NumConnections));
        break;
    }
    case W3D_CHUNK_NODE:
    {
        W3dHModelNodeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "RenderObjName", "string", StringFromFixed(data.RenderObjName, W3D_NAME_LEN));
        AddRow(rows, "PivotIdx", "uint32", std::to_string(data.PivotIdx));
        break;
    }
    case W3D_CHUNK_COLLISION_NODE:
    {
        W3dHModelNodeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "CollisionMeshName", "string", StringFromFixed(data.RenderObjName, W3D_NAME_LEN));
        AddRow(rows, "PivotIdx", "uint32", std::to_string(data.PivotIdx));
        break;
    }
    case W3D_CHUNK_SKIN_NODE:
    {
        W3dHModelNodeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "SkinMeshName", "string", StringFromFixed(data.RenderObjName, W3D_NAME_LEN));
        AddRow(rows, "PivotIdx", "uint32", std::to_string(data.PivotIdx));
        break;
    }
    case OBSOLETE_W3D_CHUNK_SHADOW_NODE:
    {
        W3dHModelNodeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "ShadowMeshName", "string", StringFromFixed(data.RenderObjName, W3D_NAME_LEN));
        AddRow(rows, "PivotIdx", "uint32", std::to_string(data.PivotIdx));
        break;
    }
    case OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA:
    {
        W3dHModelAuxDataStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Attributes", "uint32", FormatHex(data.Attributes));
        AddRow(rows, "MeshCount", "uint32", std::to_string(data.MeshCount));
        AddRow(rows, "CollisionCount", "uint32", std::to_string(data.CollisionCount));
        AddRow(rows, "SkinCount", "uint32", std::to_string(data.SkinCount));
        AddRow(rows, "ShadowCount", "uint32", std::to_string(data.ShadowCount));
        AddRow(rows, "NullCount", "uint32", std::to_string(data.NullCount));
        AddRow(rows, "FutureCounts", "uint32[6]", FormatUIntArray(data.FutureCounts, ArrayCount(data.FutureCounts)));
        AddRow(rows, "LODMin", "float", FormatFloat(data.LODMin));
        AddRow(rows, "LODMax", "float", FormatFloat(data.LODMax));
        AddRow(rows, "FutureUse", "uint32[32]", FormatUIntArray(data.FutureUse, ArrayCount(data.FutureUse)));
        break;
    }
    case W3D_CHUNK_LODMODEL_HEADER:
    {
        W3dLODModelHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "NumLODs", "uint16", std::to_string(data.NumLODs));
        break;
    }
    case W3D_CHUNK_LOD:
    {
        W3dLODStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Render Object Name", "string", StringFromFixed(data.RenderObjName, W3D_NAME_LEN));
        AddRow(rows, "LOD Min Distance", "float", FormatFloat(data.LODMin));
        AddRow(rows, "LOD Max Distance", "float", FormatFloat(data.LODMax));
        break;
    }
    case W3D_CHUNK_COLLECTION_HEADER:
    {
        W3dCollectionHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "RenderObjectCount", "uint32", std::to_string(data.RenderObjectCount));
        break;
    }
    case W3D_CHUNK_EMITTER_HEADER:
    {
        W3dEmitterHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "uint32", std::to_string(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        break;
    }
    case W3D_CHUNK_EMITTER_USER_DATA:
    {
        const std::size_t header_size = offsetof(W3dEmitterUserInfoStruct, StringParam);
        if (chunk.data.size() < header_size) {
            break;
        }
        W3dEmitterUserInfoStruct data{};
        std::memcpy(&data, chunk.data.data(), header_size);
        AddRow(rows, "Type", "uint32", std::to_string(data.Type));
        AddRow(rows, "StringSize", "uint32", std::to_string(data.SizeofStringParam));
        if (chunk.data.size() > header_size) {
            const std::size_t offset = header_size;
            const std::size_t remaining = chunk.data.size() - offset;
            const std::size_t length = std::min<std::size_t>(data.SizeofStringParam, remaining);
            AddRow(rows, "StringParam", "string",
                   StringFromBytes(chunk.data.data() + offset, length));
        }
        break;
    }
    case W3D_CHUNK_EMITTER_INFO:
    {
        W3dEmitterInfoStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "TextureFilename", "string", StringFromFixed(data.TextureFilename, sizeof(data.TextureFilename)));
        AddRow(rows, "StartSize", "float", FormatFloat(data.StartSize));
        AddRow(rows, "EndSize", "float", FormatFloat(data.EndSize));
        AddRow(rows, "Lifetime", "float", FormatFloat(data.Lifetime));
        AddRow(rows, "EmissionRate", "float", FormatFloat(data.EmissionRate));
        AddRow(rows, "MaxEmissions", "float", FormatFloat(data.MaxEmissions));
        AddRow(rows, "VelocityRandom", "float", FormatFloat(data.VelocityRandom));
        AddRow(rows, "PositionRandom", "float", FormatFloat(data.PositionRandom));
        AddRow(rows, "FadeTime", "float", FormatFloat(data.FadeTime));
        AddRow(rows, "Gravity", "float", FormatFloat(data.Gravity));
        AddRow(rows, "Elasticity", "float", FormatFloat(data.Elasticity));
        AddRow(rows, "Velocity", "vec3", FormatVec3(data.Velocity));
        AddRow(rows, "Acceleration", "vec3", FormatVec3(data.Acceleration));
        AddRow(rows, "StartColor", "rgba", FormatRGBA(data.StartColor));
        AddRow(rows, "EndColor", "rgba", FormatRGBA(data.EndColor));
        break;
    }
    case W3D_CHUNK_EMITTER_INFOV2:
    {
        W3dEmitterInfoStructV2 data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "BurstSize", "uint32", std::to_string(data.BurstSize));
        AddEmitterVolume(rows, "CreationVolume", data.CreationVolume);
        AddEmitterVolume(rows, "VelRandom", data.VelRandom);
        AddRow(rows, "OutwardVel", "float", FormatFloat(data.OutwardVel));
        AddRow(rows, "VelInherit", "float", FormatFloat(data.VelInherit));
        AddShaderRows(rows, "Shader", data.Shader);
        AddRow(rows, "RenderMode", "uint32", std::to_string(data.RenderMode));
        AddRow(rows, "FrameMode", "uint32", std::to_string(data.FrameMode));
        break;
    }
    case W3D_CHUNK_EMITTER_PROPS:
    {
        W3dEmitterPropertyStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "ColorKeyframes", "uint32", std::to_string(data.ColorKeyframes));
        AddRow(rows, "OpacityKeyframes", "uint32", std::to_string(data.OpacityKeyframes));
        AddRow(rows, "SizeKeyframes", "uint32", std::to_string(data.SizeKeyframes));
        AddRow(rows, "ColorRandom", "rgba", FormatRGBA(data.ColorRandom));
        AddRow(rows, "OpacityRandom", "float", FormatFloat(data.OpacityRandom));
        AddRow(rows, "SizeRandom", "float", FormatFloat(data.SizeRandom));
        break;
    }
    case W3D_CHUNK_PLACEHOLDER:
    {
        W3dPlaceholderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.version));
        AddRow(rows, "NameLength", "uint32", std::to_string(data.name_len));
        AddTransformRows(rows, data.transform);

        const std::size_t offset = sizeof(W3dPlaceholderStruct);
        if (chunk.data.size() > offset) {
            const std::size_t remaining = chunk.data.size() - offset;
            const std::size_t length = std::min<std::size_t>(data.name_len, remaining);
            AddRow(rows, "Name", "string", StringFromBytes(chunk.data.data() + offset, length));
        }
        break;
    }
    case W3D_CHUNK_TRANSFORM_NODE:
    {
        W3dTransformNodeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.version));
        AddRow(rows, "NameLength", "uint32", std::to_string(data.name_len));
        AddTransformRows(rows, data.transform);

        const std::size_t offset = sizeof(W3dTransformNodeStruct);
        if (chunk.data.size() > offset) {
            const std::size_t remaining = chunk.data.size() - offset;
            const std::size_t length = std::min<std::size_t>(data.name_len, remaining);
            AddRow(rows, "Name", "string", StringFromBytes(chunk.data.data() + offset, length));
        }
        break;
    }
    case W3D_CHUNK_VERTICES:
    {
        const auto count = AddArrayCountValue<W3dVectorStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dVectorStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "vector", FormatVec3(data[i]));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_NORMALS:
    {
        const auto count = AddArrayCountValue<W3dVectorStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dVectorStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Normal[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "vector", FormatVec3(data[i]));
        }
        break;
    }
    case W3D_CHUNK_SURRENDER_NORMALS:
    {
        const auto count = AddArrayCountValue<W3dVectorStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dVectorStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "SRNormal[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "vector", FormatVec3(data[i]));
        }
        break;
    }
    case W3D_CHUNK_POINTS:
    {
        const auto count = AddArrayCountValue<W3dVectorStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dVectorStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Point[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "vector", FormatVec3(data[i]));
        }
        break;
    }
    case W3D_CHUNK_TEXCOORDS:
    {
        const auto count = AddArrayCountValue<W3dTexCoordStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dTexCoordStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "TexCoord[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "UV", FormatTexCoord(data[i]));
        }
        break;
    }
    case W3D_CHUNK_STAGE_TEXCOORDS:
    {
        const auto count = AddArrayCountValue<W3dTexCoordStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dTexCoordStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "].UV";
            AddRow(rows, label.c_str(), "UV", FormatTexCoord(data[i]));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_COLORS:
    {
        const auto count = AddArrayCountValue<W3dRGBStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dRGBStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "].RGB";
            AddRow(rows, label.c_str(), "RGB", FormatRGB(data[i]));
        }
        break;
    }
    case W3D_CHUNK_DCG:
    {
        const auto count = AddArrayCountValue<W3dRGBAStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dRGBAStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "].DCG";
            AddRow(rows, label.c_str(), "RGBA", FormatRGBA(data[i]));
        }
        break;
    }
    case W3D_CHUNK_DIG:
    {
        const auto count = AddArrayCountValue<W3dRGBStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dRGBStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "].DIG";
            AddRow(rows, label.c_str(), "RGB", FormatRGB(data[i]));
        }
        break;
    }
    case W3D_CHUNK_SCG:
    {
        const auto count = AddArrayCountValue<W3dRGBStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dRGBStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "].SCG";
            AddRow(rows, label.c_str(), "RGB", FormatRGB(data[i]));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_INFLUENCES:
    {
        const auto count = AddArrayCountValue<W3dVertInfStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dVertInfStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "VertexInfluence[" + std::to_string(i) + "]";
            AddRow(rows, (base + ".BoneIdx").c_str(), "uint16", std::to_string(data[i].BoneIdx));
            AddRow(rows, (base + ".Pad").c_str(), "int8[6]", FormatUIntArray(data[i].Pad, 6));
        }
        break;
    }
    case W3D_CHUNK_TRIANGLES:
    {
        const auto count = AddArrayCountValue<W3dTriStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dTriStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Triangle[" + std::to_string(i) + "].";
            AddRow(rows, (base + "VertexIndices").c_str(), "uint32[3]", FormatUIntArray(data[i].Vindex, 3));
            AddRow(rows, (base + "Attributes").c_str(), "uint32", std::to_string(data[i].Attributes));
            AddRow(rows, (base + "Normal").c_str(), "vector", FormatVec3(data[i].Normal));
            AddRow(rows, (base + "Dist").c_str(), "float", FormatFloat(data[i].Dist));
        }
        break;
    }
    case W3D_CHUNK_PER_TRI_MATERIALS:
    {
        const auto count = AddArrayCountValue<uint16>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint16 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Triangle[" + std::to_string(i) + "].MaterialIdx";
            AddRow(rows, label.c_str(), "uint16", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_SHADE_INDICES:
    {
        const auto count = AddArrayCountValue<uint32>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint32 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Index[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "uint32", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_VERTEX_MATERIAL_IDS:
    {
        const auto count = AddArrayCountValue<uint32>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint32 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Vertex[" + std::to_string(i) + "] Vertex Material Index";
            AddRow(rows, label.c_str(), "uint32", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_SHADER_IDS:
    {
        const auto count = AddArrayCountValue<uint32>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint32 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Face[" + std::to_string(i) + "] Shader Index";
            AddRow(rows, label.c_str(), "uint32", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_TEXTURE_IDS:
    {
        const auto count = AddArrayCountValue<uint32>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint32 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Face[" + std::to_string(i) + "] Texture Index";
            AddRow(rows, label.c_str(), "uint32", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_PER_FACE_TEXCOORD_IDS:
    {
        const auto count = AddArrayCountValue<Vector3i>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const Vector3i *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Face[" + std::to_string(i) + "] UV Indices";
            AddRow(rows, label.c_str(), "IJK", FormatVector3i(data[i]));
        }
        break;
    }
    case W3D_CHUNK_AABTREE_HEADER:
    {
        W3dMeshAABTreeHeader data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "NodeCount", "uint32", std::to_string(data.NodeCount));
        AddRow(rows, "PolyCount", "uint32", std::to_string(data.PolyCount));
        break;
    }
    case W3D_CHUNK_AABTREE_POLYINDICES:
    {
        const auto count = AddArrayCountValue<uint32>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const uint32 *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string label = "Polygon Index[" + std::to_string(i) + "]";
            AddRow(rows, label.c_str(), "uint32", std::to_string(data[i]));
        }
        break;
    }
    case W3D_CHUNK_AABTREE_NODES:
    {
        const auto count = AddArrayCountValue<W3dMeshAABTreeNode>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dMeshAABTreeNode *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Node[" + std::to_string(i) + "].";
            AddRow(rows, (base + "Min").c_str(), "vec3", FormatVec3(data[i].Min));
            AddRow(rows, (base + "Max").c_str(), "vec3", FormatVec3(data[i].Max));
            if (data[i].FrontOrPoly0 & 0x80000000) {
                AddRow(rows, (base + "Poly0").c_str(), "uint32",
                       std::to_string(data[i].FrontOrPoly0 & 0x7FFFFFFF));
                AddRow(rows, (base + "PolyCount").c_str(), "uint32", std::to_string(data[i].BackOrPolyCount));
            } else {
                AddRow(rows, (base + "Front").c_str(), "uint32", std::to_string(data[i].FrontOrPoly0));
                AddRow(rows, (base + "Back").c_str(), "uint32", std::to_string(data[i].BackOrPolyCount));
            }
        }
        break;
    }
    case W3D_CHUNK_PIVOTS:
    {
        const auto count = AddArrayCountValue<W3dPivotStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dPivotStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "Pivot[" + std::to_string(i) + "].";
            AddRow(rows, (base + "Name").c_str(), "string", StringFromFixed(data[i].Name, W3D_NAME_LEN));
            AddRow(rows, (base + "ParentIdx").c_str(), "uint32", std::to_string(data[i].ParentIdx));
            AddRow(rows, (base + "Translation").c_str(), "vec3", FormatVec3(data[i].Translation));
            AddRow(rows, (base + "EulerAngles").c_str(), "vec3", FormatVec3(data[i].EulerAngles));
            AddRow(rows, (base + "Rotation").c_str(), "quaternion", FormatQuaternion(data[i].Rotation));
        }
        break;
    }
    case W3D_CHUNK_PIVOT_FIXUPS:
    {
        const auto count = AddArrayCountValue<W3dPivotFixupStruct>(rows, chunk, "Count");
        const auto *data = reinterpret_cast<const W3dPivotFixupStruct *>(chunk.data.data());
        for (std::size_t i = 0; i < count; ++i) {
            for (int row = 0; row < 4; ++row) {
                std::string label = "Transform " + std::to_string(i) + ", Row[" + std::to_string(row) + "]";
                AddRow(rows, label.c_str(), "float[3]", FormatFloatArray(data[i].TM[row], 3));
            }
        }
        break;
    }
    case W3D_CHUNK_LIGHT_INFO:
    {
        W3dLightStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddLightAttributes(rows, data.Attributes);
        AddRow(rows, "Ambient", "rgb", FormatRGB(data.Ambient));
        AddRow(rows, "Diffuse", "rgb", FormatRGB(data.Diffuse));
        AddRow(rows, "Specular", "rgb", FormatRGB(data.Specular));
        AddRow(rows, "Intensity", "float", FormatFloat(data.Intensity));
        break;
    }
    case W3D_CHUNK_SPOT_LIGHT_INFO:
    {
        W3dSpotLightStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "SpotDirection", "vec3", FormatVec3(data.SpotDirection));
        AddRow(rows, "SpotAngle", "float", FormatFloat(data.SpotAngle));
        AddRow(rows, "SpotExponent", "float", FormatFloat(data.SpotExponent));
        break;
    }
    case W3D_CHUNK_NEAR_ATTENUATION:
    {
        W3dLightAttenuationStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Near Atten Start", "float", FormatFloat(data.Start));
        AddRow(rows, "Near Atten End", "float", FormatFloat(data.End));
        break;
    }
    case W3D_CHUNK_FAR_ATTENUATION:
    {
        W3dLightAttenuationStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Far Atten Start", "float", FormatFloat(data.Start));
        AddRow(rows, "Far Atten End", "float", FormatFloat(data.End));
        break;
    }
    case W3D_CHUNK_LIGHT_TRANSFORM:
    {
        W3dLightTransformStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        for (int row = 0; row < 3; ++row) {
            std::string label = "Transform[" + std::to_string(row) + "]";
            AddRow(rows, label.c_str(), "float[4]", FormatFloatArray(data.Transform[row], 4));
        }
        break;
    }
    case OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME:
    {
        W3dEmitterColorKeyframeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Time", "float", FormatFloat(data.Time));
        AddRow(rows, "Color", "rgba", FormatRGBA(data.Color));
        break;
    }
    case OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME:
    {
        W3dEmitterOpacityKeyframeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Time", "float", FormatFloat(data.Time));
        AddRow(rows, "Opacity", "float", FormatFloat(data.Opacity));
        break;
    }
    case OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME:
    {
        W3dEmitterSizeKeyframeStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Time", "float", FormatFloat(data.Time));
        AddRow(rows, "Size", "float", FormatFloat(data.Size));
        break;
    }
    case W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES:
    {
        if (chunk.data.size() < sizeof(W3dEmitterRotationHeaderStruct)) {
            break;
        }
        const auto *header = reinterpret_cast<const W3dEmitterRotationHeaderStruct *>(chunk.data.data());
        AddRow(rows, "KeyframeCount", "uint32", std::to_string(header->KeyframeCount));
        AddRow(rows, "Random", "float", FormatFloat(header->Random));
        AddRow(rows, "OrientationRandom", "float", FormatFloat(header->OrientationRandom));

        const auto *keys = reinterpret_cast<const W3dEmitterRotationKeyframeStruct *>(
            chunk.data.data() + sizeof(W3dEmitterRotationHeaderStruct));
        const std::size_t available = (chunk.data.size() - sizeof(W3dEmitterRotationHeaderStruct)) /
                                      sizeof(W3dEmitterRotationKeyframeStruct);
        const std::size_t count = std::min<std::size_t>(header->KeyframeCount + 1, available);
        for (std::size_t i = 0; i < count; ++i) {
            std::string time_label = "Time[" + std::to_string(i) + "]";
            std::string rot_label = "Rotation[" + std::to_string(i) + "]";
            AddRow(rows, time_label.c_str(), "float", FormatFloat(keys[i].Time));
            AddRow(rows, rot_label.c_str(), "float", FormatFloat(keys[i].Rotation));
        }
        break;
    }
    case W3D_CHUNK_EMITTER_FRAME_KEYFRAMES:
    {
        if (chunk.data.size() < sizeof(W3dEmitterFrameHeaderStruct)) {
            break;
        }
        const auto *header = reinterpret_cast<const W3dEmitterFrameHeaderStruct *>(chunk.data.data());
        AddRow(rows, "KeyframeCount", "uint32", std::to_string(header->KeyframeCount));
        AddRow(rows, "Random", "float", FormatFloat(header->Random));

        const auto *keys = reinterpret_cast<const W3dEmitterFrameKeyframeStruct *>(
            chunk.data.data() + sizeof(W3dEmitterFrameHeaderStruct));
        const std::size_t available = (chunk.data.size() - sizeof(W3dEmitterFrameHeaderStruct)) /
                                      sizeof(W3dEmitterFrameKeyframeStruct);
        const std::size_t count = std::min<std::size_t>(header->KeyframeCount + 1, available);
        for (std::size_t i = 0; i < count; ++i) {
            std::string time_label = "Time[" + std::to_string(i) + "]";
            std::string frame_label = "Frame[" + std::to_string(i) + "]";
            AddRow(rows, time_label.c_str(), "float", FormatFloat(keys[i].Time));
            AddRow(rows, frame_label.c_str(), "float", FormatFloat(keys[i].Frame));
        }
        break;
    }
    case W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES:
    {
        if (chunk.data.size() < sizeof(W3dEmitterBlurTimeHeaderStruct)) {
            break;
        }
        const auto *header = reinterpret_cast<const W3dEmitterBlurTimeHeaderStruct *>(chunk.data.data());
        AddRow(rows, "KeyframeCount", "uint32", std::to_string(header->KeyframeCount));
        AddRow(rows, "Random", "float", FormatFloat(header->Random));

        const auto *keys = reinterpret_cast<const W3dEmitterBlurTimeKeyframeStruct *>(
            chunk.data.data() + sizeof(W3dEmitterBlurTimeHeaderStruct));
        const std::size_t available = (chunk.data.size() - sizeof(W3dEmitterBlurTimeHeaderStruct)) /
                                      sizeof(W3dEmitterBlurTimeKeyframeStruct);
        const std::size_t count = std::min<std::size_t>(header->KeyframeCount + 1, available);
        for (std::size_t i = 0; i < count; ++i) {
            std::string time_label = "Time[" + std::to_string(i) + "]";
            std::string blur_label = "BlurTime[" + std::to_string(i) + "]";
            AddRow(rows, time_label.c_str(), "float", FormatFloat(keys[i].Time));
            AddRow(rows, blur_label.c_str(), "float", FormatFloat(keys[i].BlurTime));
        }
        break;
    }
    case W3D_CHUNK_AGGREGATE_HEADER:
    {
        W3dAggregateHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        break;
    }
    case W3D_CHUNK_AGGREGATE_INFO:
    {
        if (chunk.data.size() < sizeof(W3dAggregateInfoStruct)) {
            break;
        }
        const auto *info = reinterpret_cast<const W3dAggregateInfoStruct *>(chunk.data.data());
        AddRow(rows, "BaseModelName", "string", StringFromFixed(info->BaseModelName, W3D_NAME_LEN * 2));
        AddRow(rows, "SubobjectCount", "uint32", std::to_string(info->SubobjectCount));

        const std::size_t offset = sizeof(W3dAggregateInfoStruct);
        const std::size_t available = (chunk.data.size() - offset) / sizeof(W3dAggregateSubobjectStruct);
        const std::size_t count = std::min<std::size_t>(info->SubobjectCount, available);
        const auto *defs = reinterpret_cast<const W3dAggregateSubobjectStruct *>(chunk.data.data() + offset);
        for (std::size_t i = 0; i < count; ++i) {
            std::string base = "SubObject[" + std::to_string(i) + "].";
            AddRow(rows, (base + "SubobjectName").c_str(), "string",
                   StringFromFixed(defs[i].SubobjectName, W3D_NAME_LEN * 2));
            AddRow(rows, (base + "BoneName").c_str(), "string",
                   StringFromFixed(defs[i].BoneName, W3D_NAME_LEN * 2));
        }
        break;
    }
    case W3D_CHUNK_TEXTURE_REPLACER_INFO:
    {
        if (chunk.data.size() < sizeof(W3dTextureReplacerHeaderStruct)) {
            break;
        }
        const auto *header = reinterpret_cast<const W3dTextureReplacerHeaderStruct *>(chunk.data.data());
        AddRow(rows, "ReplacedTexturesCount", "uint32", std::to_string(header->ReplacedTexturesCount));

        const std::size_t offset = sizeof(W3dTextureReplacerHeaderStruct);
        const std::size_t available = (chunk.data.size() - offset) / sizeof(W3dTextureReplacerStruct);
        const std::size_t count = std::min<std::size_t>(header->ReplacedTexturesCount, available);
        const auto *data = reinterpret_cast<const W3dTextureReplacerStruct *>(chunk.data.data() + offset);

        for (std::size_t i = 0; i < count; ++i) {
            for (int path = 0; path < MESH_PATH_ENTRIES; ++path) {
                std::string label = "Replacer[" + std::to_string(i) + "].MeshPath[" + std::to_string(path) + "]";
                AddRow(rows, label.c_str(), "string",
                       StringFromFixed(data[i].MeshPath[path], MESH_PATH_ENTRY_LEN));
            }
            for (int path = 0; path < MESH_PATH_ENTRIES; ++path) {
                std::string label = "Replacer[" + std::to_string(i) + "].BonePath[" + std::to_string(path) + "]";
                AddRow(rows, label.c_str(), "string",
                       StringFromFixed(data[i].BonePath[path], MESH_PATH_ENTRY_LEN));
            }
            AddRow(rows, "OldTextureName", "string",
                   StringFromFixed(data[i].OldTextureName, sizeof(data[i].OldTextureName)));
            AddRow(rows, "NewTextureName", "string",
                   StringFromFixed(data[i].NewTextureName, sizeof(data[i].NewTextureName)));
            AddRow(rows, "TextureParams.Attributes", "uint16", FormatHex(data[i].TextureParams.Attributes));
            if (data[i].TextureParams.Attributes & W3DTEXTURE_PUBLISH) {
                AddFlag(rows, "TextureParams.Attributes", "W3DTEXTURE_PUBLISH");
            }
            if (data[i].TextureParams.Attributes & W3DTEXTURE_NO_LOD) {
                AddFlag(rows, "TextureParams.Attributes", "W3DTEXTURE_NO_LOD");
            }
            if (data[i].TextureParams.Attributes & W3DTEXTURE_CLAMP_U) {
                AddFlag(rows, "TextureParams.Attributes", "W3DTEXTURE_CLAMP_U");
            }
            if (data[i].TextureParams.Attributes & W3DTEXTURE_CLAMP_V) {
                AddFlag(rows, "TextureParams.Attributes", "W3DTEXTURE_CLAMP_V");
            }
            if (data[i].TextureParams.Attributes & W3DTEXTURE_ALPHA_BITMAP) {
                AddFlag(rows, "TextureParams.Attributes", "W3DTEXTURE_ALPHA_BITMAP");
            }
            AddRow(rows, "TextureParams.FrameCount", "uint32", std::to_string(data[i].TextureParams.FrameCount));
            AddRow(rows, "TextureParams.FrameRate", "float", FormatFloat(data[i].TextureParams.FrameRate));
        }
        break;
    }
    case W3D_CHUNK_AGGREGATE_CLASS_INFO:
    {
        W3dAggregateMiscInfo data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "OriginalClassID", "uint32", std::to_string(data.OriginalClassID));
        AddRow(rows, "Flags", "uint32", FormatHex(data.Flags));
        break;
    }
    case W3D_CHUNK_HLOD_HEADER:
    {
        W3dHLodHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "LodCount", "uint32", std::to_string(data.LodCount));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, W3D_NAME_LEN));
        AddRow(rows, "HTree Name", "string", StringFromFixed(data.HierarchyName, W3D_NAME_LEN));
        break;
    }
    case W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER:
    {
        W3dHLodArrayHeaderStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "ModelCount", "uint32", std::to_string(data.ModelCount));
        AddRow(rows, "MaxScreenSize", "float", FormatFloat(data.MaxScreenSize));
        break;
    }
    case W3D_CHUNK_HLOD_SUB_OBJECT:
    {
        W3dHLodSubObjectStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, sizeof(data.Name)));
        AddRow(rows, "BoneIndex", "uint32", std::to_string(data.BoneIndex));
        break;
    }
    case W3D_CHUNK_BOX:
    {
        W3dBoxStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddBoxAttributes(rows, data.Attributes);
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, sizeof(data.Name)));
        AddRow(rows, "Color", "rgb", FormatRGB(data.Color));
        AddRow(rows, "Center", "vec3", FormatVec3(data.Center));
        AddRow(rows, "Extent", "vec3", FormatVec3(data.Extent));
        break;
    }
    case W3D_CHUNK_NULL_OBJECT:
    {
        W3dNullObjectStruct data{};
        if (!ReadStruct(chunk, data)) {
            break;
        }
        AddRow(rows, "Version", "version", FormatVersion(data.Version));
        AddRow(rows, "Attributes", "uint32", FormatHex(data.Attributes));
        AddRow(rows, "Name", "string", StringFromFixed(data.Name, sizeof(data.Name)));
        break;
    }
    default:
        break;
    }

    return rows;
}

} // namespace wdump
