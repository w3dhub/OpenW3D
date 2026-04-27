#pragma once

// Stub D3DX9 Mesh header for Linux builds
// Provides minimal declarations needed by ww3d2 code

#include "dxvk_wrapper_compat.h"

#ifdef _WIN32
#include <dxvk/d3dx9mesh.h>
#else

// Minimal D3DX9 Mesh declarations for Linux
// The actual implementation is handled by the bgfx backend

#ifndef D3DXGetFVFVertexSize
// D3DXGetFVFVertexSize - returns size of FVF vertex
// FVF format: D3DFVF_XYZ, D3DFVF_NORMAL, D3DFVF_DIFFUSE, D3DFVF_SPECULAR, D3DFVF_TEX0-8
inline UINT D3DXGetFVFVertexSize(DWORD FVF)
{
    UINT size = 0;
    
    // Position: always 3 floats = 12 bytes
    if (FVF & 0x001) { // D3DFVF_XYZ
        size += 12;
    }
    
    // RHW flag: if set, no transformed position
    // (handled separately)
    
    // Blend weights: 4 bytes each
    if (FVF & 0x002) { // D3DFVF_XYZB1
        size += 4;
    }
    if (FVF & 0x004) { // D3DFVF_XYZB2
        size += 4;
    }
    if (FVF & 0x008) { // D3DFVF_XYZB3
        size += 4;
    }
    if (FVF & 0x010) { // D3DFVF_XYZB4
        size += 4;
    }
    
    // Normal: 3 floats = 12 bytes  
    if (FVF & 0x010) { // D3DFVF_NORMAL
        size += 12;
    }
    
    // Point size: 4 bytes
    if (FVF & 0x020) { // D3DFVF_PSIZE
        size += 4;
    }
    
    // Diffuse color: 4 bytes
    if (FVF & 0x040) { // D3DFVF_DIFFUSE
        size += 4;
    }
    
    // Specular color: 4 bytes
    if (FVF & 0x080) { // D3DFVF_SPECULAR
        size += 4;
    }
    
    // Texture coordinates
    UINT texCoordSize = (FVF >> 16) & 0x0F; // Number of texture coordinate sets
    for (UINT i = 0; i < texCoordSize; i++) {
        UINT texCoordIdx = (FVF >> (16 + i)) & 0x0F;
        switch (texCoordIdx) {
            case 0: size += 8; break; // D3DFVF_TEXCOORDSIZE1 = 2 floats
            case 1: size += 12; break; // D3DFVF_TEXCOORDSIZE2 = 3 floats
            case 2: size += 16; break; // D3DFVF_TEXCOORDSIZE3 = 4 floats
            case 3: size += 16; break; // D3DFVF_TEXCOORDSIZE4 = 4 floats
            default: size += 8; break;
        }
    }
    
    return size;
}
#endif

// D3DFVF flags
#define D3DFVF_XYZ          0x001
#define D3DFVF_XYZB1        0x002
#define D3DFVF_XYZB2        0x004
#define D3DFVF_XYZB3        0x008
#define D3DFVF_XYZB4        0x010
#define D3DFVF_NORMAL       0x010
#define D3DFVF_PSIZE        0x020
#define D3DFVF_DIFFUSE      0x040
#define D3DFVF_SPECULAR     0x080
#define D3DFVF_TEXCOUNT_MASK 0x0F
#define D3DFVF_TEXCOUNT_SHIFT 16
#define D3DFVF_TEX0         0x100
#define D3DFVF_TEX1         0x200
#define D3DFVF_TEX2         0x300
#define D3DFVF_TEX3         0x400
#define D3DFVF_TEX4         0x500
#define D3DFVF_TEX5         0x600
#define D3DFVF_TEX6         0x700
#define D3DFVF_TEX7         0x800
#define D3DFVF_TEX8         0x900
#define D3DFVF_LASTBETA_UBYTE4 0x1000

// Texture coordinate sizes
#define D3DFVF_TEXCOORDSIZE1(coord) ((coord+1) << 16)
#define D3DFVF_TEXCOORDSIZE2(coord) ((coord+2) << 16)
#define D3DFVF_TEXCOORDSIZE3(coord) ((coord+3) << 16)
#define D3DFVF_TEXCOORDSIZE4(coord) ((coord+4) << 16)

#endif // _WIN32
