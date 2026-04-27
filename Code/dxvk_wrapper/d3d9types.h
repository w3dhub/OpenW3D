#ifndef D3D9TYPES_STUB_H
#define D3D9TYPES_STUB_H

// Linux stub for d3d9types.h
// Provides minimal type definitions needed by the codebase without pulling in full DXVK

#include "dxvk_wrapper_compat.h"

// Stub D3D types for Linux - these are not real Direct3D types,
// just placeholders to satisfy the type system
// Use #ifndef to avoid conflicts with dx8wrapper.h which defines some of these too
#ifndef D3DFILL_POINT
const unsigned D3DFILL_POINT = 1;
#endif

#ifndef D3DFILL_WIREFRAME
const unsigned D3DFILL_WIREFRAME = 2;
#endif

#ifndef D3DFILL_SOLID
const unsigned D3DFILL_SOLID = 3;
#endif

// D3DRS_* render states
#ifndef D3DRS_FILLMODE
const unsigned D3DRS_FILLMODE = 8;
#endif
#ifndef D3DRS_AMBIENT
const unsigned D3DRS_AMBIENT = 13;
#endif

// D3DTSS_* texture stage states
#ifndef D3DTSS_TCI_CAMERASPACENORMAL
const unsigned D3DTSS_TCI_CAMERASPACENORMAL = 0x00010000;
#endif
#ifndef D3DTSS_TCI_CAMERASPACEPOSITION
const unsigned D3DTSS_TCI_CAMERASPACEPOSITION = 0x00020000;
#endif
#ifndef D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
const unsigned D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR = 0x00030000;
#endif
#ifndef D3DTSS_TCI_PASSTHRU
const unsigned D3DTSS_TCI_PASSTHRU = 0x00000000;
#endif
#ifndef D3DTSS_TEXCOORDINDEX
const unsigned D3DTSS_TEXTURETRANSFORMFLAGS = 27;
#endif
#ifndef D3DTSS_BUMPENVMAT00
const unsigned D3DTSS_BUMPENVMAT00 = 8;
#endif
#ifndef D3DTSS_BUMPENVMAT01
const unsigned D3DTSS_BUMPENVMAT01 = 9;
#endif
#ifndef D3DTSS_BUMPENVMAT10
const unsigned D3DTSS_BUMPENVMAT10 = 10;
#endif
#ifndef D3DTSS_BUMPENVMAT11
const unsigned D3DTSS_BUMPENVMAT11 = 11;
#endif

// D3DTTFF_* texture transform flags
#ifndef D3DTTFF_COUNT2
const unsigned D3DTTFF_COUNT2 = 0x00000200;
#endif
#ifndef D3DTTFF_COUNT3
const unsigned D3DTTFF_COUNT3 = 0x00000400;
#endif
#ifndef D3DTTFF_PROJECTED
const unsigned D3DTTFF_PROJECTED = 0x00000100;
#endif

// D3DTRANSFORMSTATETYPE
#ifndef D3DTRANSFORMSTATETYPE_DEFINED
typedef unsigned D3DTRANSFORMSTATETYPE;
#define D3DTRANSFORMSTATETYPE_DEFINED
#endif

// D3DVIEWPORT9
#ifndef D3DVIEWPORT9_DEFINED
struct D3DVIEWPORT9 {
    unsigned int X;
    unsigned int Y;
    unsigned int Width;
    unsigned int Height;
    float MinZ;
    float MaxZ;
};
#define D3DVIEWPORT9_DEFINED
#endif

// D3DLOCKED_RECT
#ifndef D3DLOCKED_RECT_DEFINED
struct D3DLOCKED_RECT {
    int Pitch;
    void *pBits;
};
#define D3DLOCKED_RECT_DEFINED
#endif

// D3DSURFACE_DESC  
#ifndef D3DSURFACE_DESC_DEFINED
struct D3DSURFACE_DESC {
    unsigned int Format;
    unsigned int Type;
    unsigned int Usage;
    unsigned int Pool;
    unsigned int MultiSampleType;
    unsigned int MultiSampleQuality;
    unsigned int Width;
    unsigned int Height;
};
#define D3DSURFACE_DESC_DEFINED
#endif

// D3DFORMAT
#ifndef D3DFORMAT_DEFINED
typedef unsigned D3DFORMAT;
#define D3DFORMAT_DEFINED
#endif

// D3D pixel formats
#ifndef D3DFMT_UNKNOWN
#define D3DFMT_UNKNOWN 0
#endif

#ifndef D3DFMT_DXT1
#define D3DFMT_DXT1 0x31545844
#endif

#ifndef D3DFMT_DXT2
#define D3DFMT_DXT2 0x32545844
#endif

#ifndef D3DFMT_DXT3
#define D3DFMT_DXT3 0x33545844
#endif

#ifndef D3DFMT_DXT4
#define D3DFMT_DXT4 0x34545844
#endif

#ifndef D3DFMT_DXT5
#define D3DFMT_DXT5 0x35545844
#endif

#ifndef D3DFMT_R8G8B8
#define D3DFMT_R8G8B8 20
#endif

#ifndef D3DFMT_A8R8G8B8
#define D3DFMT_A8R8G8B8 21
#endif

#ifndef D3DFMT_X8R8G8B8
#define D3DFMT_X8R8G8B8 22
#endif

#ifndef D3DFMT_R5G6B5
#define D3DFMT_R5G6B5 23
#endif

#ifndef D3DFMT_X1R5G5B5
#define D3DFMT_X1R5G5B5 24
#endif

#ifndef D3DFMT_A1R5G5B5
#define D3DFMT_A1R5G5B5 25
#endif

#ifndef D3DFMT_A4R4G4B4
#define D3DFMT_A4R4G4B4 26
#endif

#ifndef D3DFMT_R3G3B2
#define D3DFMT_R3G3B2 27
#endif

#ifndef D3DFMT_A8
#define D3DFMT_A8 28
#endif

#ifndef D3DFMT_A8R3G3B2
#define D3DFMT_A8R3G3B2 29
#endif

#ifndef D3DFMT_X4R4G4B4
#define D3DFMT_X4R4G4B4 30
#endif

#endif // D3D9TYPES_STUB_H
