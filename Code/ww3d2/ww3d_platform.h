// ww3d_platform.h — Platform compatibility for OpenW3D
//
// On Windows, includes the real D3D9 / Windows headers.
// On other platforms, provides minimal type/enum stubs so shared ww3d2
// headers compile without the Direct3D SDK.
//
// This file is NOT a full D3D9 implementation — it only defines the
// types and enums referenced by the shared ww3d2 interface.

#ifndef WW3D_PLATFORM_H
#define WW3D_PLATFORM_H

#ifdef _WIN32
  // On Windows, use the real headers
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <d3d9.h>
  #include <d3d9types.h>
  #include <d3d9caps.h>
#else
  // ---- Basic Win32 type definitions ----
  // Some are already in bittype.h; avoid redefining those.
  // Include bittype.h first so we can check what's defined.
  #include "bittype.h"

  #ifndef TRUE
    #define TRUE 1
  #endif
  #ifndef FALSE
    #define FALSE 0
  #endif
  #ifndef MAX_PATH
    #define MAX_PATH 260
  #endif
  #ifndef CALLBACK
    #define CALLBACK
  #endif
  #ifndef WINAPI
    #define WINAPI
  #endif
  #ifndef VOID
    #define VOID void
  #endif
  #ifndef CONST
    #define CONST const
  #endif

  // Win32 handles not in bittype.h
  #ifndef _WINDEF_
    typedef void*          HWND;
    typedef void*          HINSTANCE;
    typedef void*          HANDLE;
    typedef long           LPARAM;
    typedef unsigned long  WPARAM;
  #endif

  // HRESULT — standard COM return type
  typedef long HRESULT;
  #ifndef S_OK
    #define S_OK ((HRESULT)0L)
  #endif
  #ifndef E_FAIL
    #define E_FAIL ((HRESULT)0x80004005L)
  #endif

  // GUID — needed by D3DADAPTER_IDENTIFIER9
  typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
  } GUID;

  // Forward-declare D3D interface types (pointer-only usage on non-Windows)
  struct IDirect3D9;
  struct IDirect3DDevice9;
  struct IDirect3DTexture9;
  struct IDirect3DSurface9;
  struct IDirect3DVertexBuffer9;
  struct IDirect3DIndexBuffer9;
  struct IDirect3DBaseTexture9;
  struct IDirect3DSwapChain9;

  // ---- D3D color ----
  typedef unsigned int D3DCOLOR;

  // ---- D3DRENDERSTATETYPE ----
  typedef enum _D3DRENDERSTATETYPE {
    D3DRS_FILLMODE              = 8,
    D3DRS_SHADEMODE             = 9,
    D3DRS_ZENABLE               = 7,
    D3DRS_CULLMODE              = 22,
    D3DRS_ALPHABLENDENABLE      = 27,
    D3DRS_SRCBLEND              = 19,
    D3DRS_DESTBLEND             = 20,
    D3DRS_FOGENABLE             = 28,
    D3DRS_FOGCOLOR              = 34,
    D3DRS_FOGSTART              = 36,
    D3DRS_FOGEND                = 37,
    D3DRS_FOGDENSITY            = 38,
    D3DRS_ZFUNC                 = 23,
    D3DRS_ZWRITEENABLE          = 14,
    D3DRS_LIGHTING              = 137,
    D3DRS_SPECULARENABLE         = 29,
    D3DRS_AMBIENT               = 123,
    D3DRS_AMBIENTMATERIALSOURCE = 142,
    D3DRS_DIFFUSEMATERIALSOURCE= 143,
    D3DRS_SPECULARMATERIALSOURCE=144,
    D3DRS_EMISSIVEMATERIALSOURCE=145,
    D3DRS_NORMALIZENORMALS      = 232,
    D3DRS_LOCALVIEWER           = 142,
    D3DRS_COLORVERTEX            = 141,
    D3DRS_FOGVERTEXMODE          = 140,
    D3DRS_FOGTABLEMODE           = 35,
    D3DRS_CLIPPLANEENABLE        = 152,
    D3DRS_STENCILENABLE          = 155,
    D3DRS_STENCILFAIL            = 156,
    D3DRS_STENCILZFAIL           = 157,
    D3DRS_STENCILPASS            = 158,
    D3DRS_STENCILFUNC            = 151,
    D3DRS_STENCILREF              = 154,
    D3DRS_STENCILMASK             = 153,
    D3DRS_STENCILWRITEMASK       = 159,
    D3DRS_ALPHATESTENABLE        = 15,
    D3DRS_ALPHAREF                = 24,
    D3DRS_ALPHAFUNC               = 25,
    D3DRS_DITHERENABLE            = 26,
    D3DRS_TEXTUREFACTOR           = 60,
    D3DRS_MULTISAMPLEANTIALIAS   = 161,
    D3DRS_MULTISAMPLEMASK        = 162,
    D3DRS_ANTIALIASEDLINEENABLE   = 163,
    D3DRS_LASTPIXEL               = 16,
    D3DRS_SCISSORTESTENABLE       = 174,
    D3DRS_SLOPESCALEDEPTHBIAS     = 182,
    D3DRS_DEPTHBIAS               = 183,
    D3DRS_SEPARATEALPHABLENDENABLE= 206,
    D3DRS_SRCBLENDALPHA           = 207,
    D3DRS_DESTBLENDALPHA          = 208,
    D3DRS_BLENDOPALPHA           = 209,
    D3DRS_BLENDOP                = 171,
    D3DRS_POINTSIZE              = 154,
    D3DRS_POINTSIZEMIN           = 155,
    D3DRS_POINTSIZEMAX           = 156,
    D3DRS_POINTSCALEENABLE       = 157,
    D3DRS_POINTSPRITEENABLE      = 160,
    D3DRS_WRAP0                  = 128,
    D3DRS_WRAP1                  = 129,
    D3DRS_WRAP2                  = 130,
    D3DRS_WRAP3                  = 131,
    D3DRS_WRAP4                  = 132,
    D3DRS_WRAP5                  = 133,
    D3DRS_WRAP6                  = 134,
    D3DRS_WRAP7                  = 135,
    D3DRS_COLORWRITEENABLE       = 168,
    D3DRS_TWEENFACTOR            = 184,
  } D3DRENDERSTATETYPE;

  // ---- D3DCULL ----
  typedef enum _D3DCULL {
    D3DCULL_NONE         = 1,
    D3DCULL_CW           = 2,
    D3DCULL_CCW          = 3,
  } D3DCULL;

  // ---- D3DFILLMODE ----
  typedef enum _D3DFILL_MODE {
    D3DFILL_POINT        = 1,
    D3DFILL_WIREFRAME    = 2,
    D3DFILL_SOLID        = 3,
  } D3DFILLMODE;

  // ---- D3DSHADEMODE ----
  typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT        = 1,
    D3DSHADE_GOURAUD     = 2,
    D3DSHADE_PHONG       = 3,
  } D3DSHADEMODE;

  // ---- D3DBLEND ----
  typedef enum _D3DBLEND {
    D3DBLEND_ZERO               = 1,
    D3DBLEND_ONE                = 2,
    D3DBLEND_SRCCOLOR           = 3,
    D3DBLEND_INVSRCCOLOR        = 4,
    D3DBLEND_SRCALPHA           = 5,
    D3DBLEND_INVSRCALPHA        = 6,
    D3DBLEND_DESTALPHA           = 7,
    D3DBLEND_INVDESTALPHA        = 8,
    D3DBLEND_DESTCOLOR           = 9,
    D3DBLEND_INVDESTCOLOR        = 10,
    D3DBLEND_SRCALPHASAT         = 11,
    D3DBLEND_BOTHSRCALPHA        = 12,
    D3DBLEND_BOTHINVSRCALPHA     = 13,
    D3DBLEND_BLENDFACTOR         = 14,
    D3DBLEND_INVBLENDFACTOR      = 15,
  } D3DBLEND;

  // ---- D3DCMPFUNC ----
  typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER          = 1,
    D3DCMP_LESS           = 2,
    D3DCMP_EQUAL          = 3,
    D3DCMP_GREATEREQUAL   = 7,
    D3DCMP_ALWAYS         = 8,
  } D3DCMPFUNC;

  // ---- D3DFOGMODE ----
  typedef enum _D3DFOGMODE {
    D3DFOG_NONE          = 0,
    D3DFOG_EXP            = 1,
    D3DFOG_EXP2           = 2,
    D3DFOG_LINEAR         = 3,
  } D3DFOGMODE;

  // ---- D3DZBUFFERTYPE ----
  typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE           = 0,
    D3DZB_TRUE            = 1,
    D3DZB_USEW            = 2,
  } D3DZBUFFERTYPE;

  // ---- MAKEFOURCC ----
  #ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3) \
      ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | \
       ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24))
  #endif

  // ---- D3DFORMAT ----
  typedef enum _D3DFORMAT {
    D3DFMT_UNKNOWN              = 0,
    D3DFMT_R8G8B8               = 20,
    D3DFMT_A8R8G8B8             = 21,
    D3DFMT_X8R8G8B8             = 22,
    D3DFMT_R5G6B5               = 23,
    D3DFMT_X1R5G5B5             = 24,
    D3DFMT_A1R5G5B5             = 25,
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8                    = 28,
    D3DFMT_A8R3G3B2              = 29,
    D3DFMT_X4R4G4B4              = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_A8B8G8R8              = 32,
    D3DFMT_X8B8G8R8              = 33,
    D3DFMT_G16R16               = 34,
    D3DFMT_A2R10G10B10          = 35,
    D3DFMT_A16B16G16R16         = 36,
    D3DFMT_D16                  = 80,
    D3DFMT_D32                  = 71,
    D3DFMT_D15S1                = 73,
    D3DFMT_D24S8                = 75,
    D3DFMT_D24X8                = 77,
    D3DFMT_D16_LOCKABLE         = 70,
    D3DFMT_D32F_LOCKABLE        = 82,
    D3DFMT_D24FS8               = 83,
    D3DFMT_INDEX16              = 101,
    D3DFMT_INDEX32              = 102,
    D3DFMT_L8                   = 50,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,
    D3DFMT_DXT1                 = MAKEFOURCC('D','X','T','1'),
    D3DFMT_DXT2                 = MAKEFOURCC('D','X','T','2'),
    D3DFMT_DXT3                 = MAKEFOURCC('D','X','T','3'),
    D3DFMT_DXT4                 = MAKEFOURCC('D','X','T','4'),
    D3DFMT_DXT5                 = MAKEFOURCC('D','X','T','5'),
    D3DFMT_X8L8V8U8             = MAKEFOURCC('X','L','V','U'),
    D3DFMT_P8                   = 41,
    D3DFMT_A8P8                 = 40,
    D3DFMT_V8U8                 = MAKEFOURCC('V','U',' ',' '),  // D3DFMT_V8U8
    D3DFMT_L6V5U5               = MAKEFOURCC('L','V','U','5'),  // D3DFMT_L6V5U5
    D3DFMT_Q8W8V8U8             = MAKEFOURCC('Q','W','V','U'),
    D3DFMT_V16U16               = MAKEFOURCC('V','1','6','U'),  // D3DFMT_V16U16
    D3DFMT_A2W10V10U10          = MAKEFOURCC('A','2','W','1'),   // bumpmap format
    D3DFMT_R8G8_B8G8            = MAKEFOURCC('R','G','B','G'),
    D3DFMT_G8R8_G8B8            = MAKEFOURCC('G','R','G','B'),
    D3DFMT_UYVY                 = MAKEFOURCC('U','Y','V','Y'),
    D3DFMT_YUY2                 = MAKEFOURCC('Y','U','Y','2'),
    D3DFMT_D24X4S4              = MAKEFOURCC('D','2','4','S'),
  } D3DFORMAT;

  // ---- D3DPRIMITIVETYPE ----
  typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST             = 1,
    D3DPT_LINELIST              = 2,
    D3DPT_LINESTRIP             = 3,
    D3DPT_TRIANGLELIST          = 4,
    D3DPT_TRIANGLESTRIP         = 5,
    D3DPT_TRIANGLEFAN           = 6,
  } D3DPRIMITIVETYPE;

  // ---- D3DFVF flags ----
  #ifndef D3DFVF_XYZ
    #define D3DFVF_XYZ              0x002
    #define D3DFVF_XYZRHW           0x004
    #define D3DFVF_XYZB1            0x006
    #define D3DFVF_XYZB2            0x008
    #define D3DFVF_XYZB3            0x00a
    #define D3DFVF_NORMAL           0x010
    #define D3DFVF_DIFFUSE          0x040
    #define D3DFVF_SPECULAR         0x080
    #define D3DFVF_TEX0             0x000
    #define D3DFVF_TEX1             0x100
    #define D3DFVF_TEX2             0x200
    #define D3DFVF_TEX3             0x300
    #define D3DFVF_TEX4             0x400
    #define D3DFVF_TEX5             0x500
    #define D3DFVF_TEX6             0x600
    #define D3DFVF_TEX7             0x700
    #define D3DFVF_TEX8             0x800
    #define D3DFVF_LASTBATCH_0      0x10000
    #define D3DFVF_POSITION_MASK    0x00E
    #define D3DFVF_POSITIONTX      0x10000
  #endif

  // ---- D3D max texture coordinates ----
  #ifndef D3DDP_MAXTEXCOORD
    #define D3DDP_MAXTEXCOORD 8
  #endif

  // ---- D3DDEVTYPE ----
  typedef enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL         = 1,
    D3DDEVTYPE_REF         = 2,
    D3DDEVTYPE_SW          = 3,
  } D3DDEVTYPE;

  // ---- D3DTEXA (texture argument flags) ----
  #ifndef D3DTA_SELECTMASK
    #define D3DTA_SELECTMASK      0x0000000f
    #define D3DTA_DIFFUSE         0x00000000
    #define D3DTA_CURRENT         0x00000001
    #define D3DTA_TEXTURE         0x00000002
    #define D3DTA_TFACTOR         0x00000003
    #define D3DTA_SPECULAR        0x00000004
    #define D3DTA_TEMP            0x00000005
    #define D3DTA_CONSTANT       0x00000006
    #define D3DTA_COMPLEMENT      0x00000010
    #define D3DTA_ALPHAREPLICATE  0x00000020
  #endif

  // ---- D3DTEXTURESTAGESTATETYPE ----
  typedef enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_COLOROP          = 1,
    D3DTSS_COLORARG1        = 2,
    D3DTSS_COLORARG2        = 3,
    D3DTSS_ALPHAOP           = 4,
    D3DTSS_ALPHAARG1         = 5,
    D3DTSS_ALPHAARG2         = 6,
    D3DTSS_BUMPENVMAT00     = 7,
    D3DTSS_BUMPENVMAT01     = 8,
    D3DTSS_BUMPENVMAT10     = 9,
    D3DTSS_BUMPENVMAT11     = 10,
    D3DTSS_BUMPENVLSCALE    = 11,
    D3DTSS_BUMPENVLOFFSET   = 12,
    D3DTSS_TEXCOORDINDEX    = 13,
    D3DTSS_COLORARG0        = 24,
    D3DTSS_ALPHAARG0        = 25,
    D3DTSS_RESULTARG         = 26,
    D3DTSS_TEXTURETRANSFORMFLAGS = 20,
    D3DTSS_BUMPENV_MAT00    = 7,
    D3DTSS_BUMPENV_MAT01    = 8,
    D3DTSS_BUMPENV_MAT10    = 9,
    D3DTSS_BUMPENV_MAT11    = 10,
  } D3DTEXTURESTAGESTATETYPE;

  // ---- D3DTEXTURETRANSFORMFLAGS ----
  #ifndef D3DTTFF_COUNT1
    #define D3DTTFF_COUNT1    1
    #define D3DTTFF_COUNT2    2
    #define D3DTTFF_COUNT3    3
    #define D3DTTFF_COUNT4    4
    #define D3DTTFF_PROJECTED 256
  #endif

  // ---- D3DTSS_TCI texture coordinate index flags ----
  #ifndef D3DTSS_TCI_PASSTHRU
    #define D3DTSS_TCI_PASSTHRU                        0x00000000
    #define D3DTSS_TCI_CAMERASPACENORMAL               0x00010000
    #define D3DTSS_TCI_CAMERASPACEPOSITION              0x00020000
    #define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR      0x00030000
    #define D3DTSS_TCI_SPHEREMAP                        0x00040000
  #endif

  // ---- FLOAT (Win32 type) and F2DW macro ----
  #ifndef FLOAT
    #define FLOAT float
  #endif
  #ifndef F2DW
    // Type-punning macro: convert float bits to DWORD
    // On Linux, use a static inline to avoid strict-aliasing issues
    // The #ifndef guard means this is only defined once per TU
    static inline DWORD _F2DW_impl(FLOAT f) { DWORD d; __builtin_memcpy(&d, &f, sizeof(d)); return d; }
    #define F2DW(f) _F2DW_impl(f)
  #endif

  // ---- D3DTEXTUREOP ----
  typedef enum _D3DTEXTUREOP {
    D3DTOP_DISABLE           = 1,
    D3DTOP_SELECTARG1        = 2,
    D3DTOP_SELECTARG2        = 3,
    D3DTOP_MODULATE          = 4,
    D3DTOP_MODULATE2X        = 5,
    D3DTOP_MODULATE4X        = 6,
    D3DTOP_ADD               = 7,
    D3DTOP_ADDSIGNED         = 8,
    D3DTOP_ADDSIGNED2X       = 9,
    D3DTOP_SUBTRACT          = 10,
    D3DTOP_ADDSMOOTH         = 11,
    D3DTOP_BLENDDIFFUSEALPHA = 12,
    D3DTOP_BLENDTEXTUREALPHA = 13,
    D3DTOP_BLENDTEXTUREALPHAPM= 14,
    D3DTOP_BLENDCURRENTALPHA = 15,
    D3DTOP_PREMODULATE       = 16,
    D3DTOP_MODULATEALPHA_ADDCOLOR = 17,
    D3DTOP_MODULATECOLOR_ADDALPHA  = 18,
    D3DTOP_MODULATEINVALPHA_ADDCOLOR = 19,
  } D3DTEXTUREOP;

  // ---- D3DTEXTUREFILTERTYPE ----
  typedef enum _D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE            = 0,
    D3DTEXF_POINT           = 1,
    D3DTEXF_LINEAR          = 2,
    D3DTEXF_ANISOTROPIC     = 3,
  } D3DTEXTUREFILTERTYPE;

  // ---- D3DTEXTUREADDRESS ----
  typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP        = 1,
    D3DTADDRESS_MIRROR       = 2,
    D3DTADDRESS_CLAMP        = 3,
    D3DTADDRESS_BORDER       = 4,
    D3DTADDRESS_MIRRORONCE   = 5,
  } D3DTEXTUREADDRESS;

  // ---- D3DTRANSFORMSTATETYPE ----
  typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTS_VIEW          = 2,
    D3DTS_PROJECTION    = 3,
    D3DTS_TEXTURE0      = 16,
    D3DTS_TEXTURE1      = 17,
    D3DTS_TEXTURE2      = 18,
    D3DTS_TEXTURE3      = 19,
    D3DTS_TEXTURE4      = 20,
    D3DTS_TEXTURE5      = 21,
    D3DTS_TEXTURE6      = 22,
    D3DTS_TEXTURE7      = 23,
    D3DTS_WORLD         = 256,
    D3DTS_WORLD1        = 257,
    D3DTS_WORLD2        = 258,
    D3DTS_WORLD3        = 259,
    D3DTS_FORCE_DWORD   = 0x7fffffff,
  } D3DTRANSFORMSTATETYPE;

  // ---- D3DSAMPLERSTATETYPE ----
  typedef enum _D3DSAMPLERSTATETYPE {
    D3DSAMP_ADDRESSU       = 1,
    D3DSAMP_ADDRESSV       = 2,
    D3DSAMP_ADDRESSW       = 3,
    D3DSAMP_BORDERCOLOR    = 4,
    D3DSAMP_MAGFILTER      = 5,
    D3DSAMP_MINFILTER      = 6,
    D3DSAMP_MIPFILTER      = 7,
    D3DSAMP_MIPMAPLODBIAS  = 8,
    D3DSAMP_MAXMIPLEVEL    = 9,
    D3DSAMP_MAXANISOTROPY  = 10,
    D3DSAMP_SRGBTEXTURE    = 11,
    D3DSAMP_ELEMENTINDEX   = 12,
  } D3DSAMPLERSTATETYPE;

  // ---- D3DLIGHTTYPE ----
  typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
  } D3DLIGHTTYPE;

  // ---- D3DLIGHT9 ----
  typedef struct _D3DLIGHT9 {
    D3DLIGHTTYPE Type;     // need D3DLIGHTTYPE
    D3DCOLOR      Diffuse;
    D3DCOLOR      Specular;
    D3DCOLOR      Ambient;
    float         Position[3];
    float         Direction[3];
    float         Range;
    float         Falloff;
    float         Attenuation0;
    float         Attenuation1;
    float         Attenuation2;
    float         Theta;
    float         Phi;
  } D3DLIGHT9;

  // ---- D3DMATERIAL9 ----
  typedef struct _D3DMATERIAL9 {
    D3DCOLOR Diffuse;
    D3DCOLOR Ambient;
    D3DCOLOR Specular;
    D3DCOLOR Emissive;
    float    Power;
  } D3DMATERIAL9;

  // ---- D3DCAPS9 ----
  typedef struct _D3DCAPS9 {
    unsigned int Caps;
    unsigned int Caps2;
    unsigned int Caps3;
    unsigned int PresentationIntervals;
    unsigned int CursorCaps;
    unsigned int DevCaps;
    unsigned int PrimitiveMiscCaps;
    unsigned int RasterCaps;
    unsigned int ZCmpCaps;
    unsigned int SrcBlendCaps;
    unsigned int DestBlendCaps;
    unsigned int AlphaCmpCaps;
    unsigned int ShadeCaps;
    unsigned int TextureCaps;
    unsigned int TextureFilterCaps;
    unsigned int CubeTextureFilterCaps;
    unsigned int VolumeTextureFilterCaps;
    unsigned int TextureAddressCaps;
    unsigned int VolumeTextureAddressCaps;
    unsigned int LineCaps;
    unsigned int MaxTextureWidth;
    unsigned int MaxTextureHeight;
    unsigned int MaxVolumeExtent;
    unsigned int MaxTextureRepeat;
    unsigned int MaxTextureAspectRatio;
    unsigned int MaxAnisotropy;
    float MaxVertexW;
    float GuardBandLeft;
    float GuardBandTop;
    float GuardBandRight;
    float GuardBandBottom;
    float ExtentsAdjust;
    unsigned int StencilCaps;
    unsigned int FVFCaps;
    unsigned int TextureOpCaps;
    unsigned int MaxTextureBlendStages;
    unsigned int MaxSimultaneousTextures;
    unsigned int VertexProcessingCaps;
    unsigned int MaxActiveLights;
    unsigned int MaxUserClipPlanes;
    unsigned int MaxVertexBlendMatrices;
    unsigned int MaxVertexBlendMatrixIndex;
    float MaxPointSize;
    unsigned int MaxPrimitiveCount;
    unsigned int MaxVertexIndex;
    unsigned int MaxStreams;
    unsigned int MaxStreamStride;
    unsigned int VertexShaderVersion;
    unsigned int MaxVertexShaderConst;
    unsigned int PixelShaderVersion;
    float PixelShader1xMaxValue;
  } D3DCAPS9;

  // ---- D3DPRESENT_PARAMETERS ----
  typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD    = 1,
    D3DSWAPEFFECT_FLIP       = 2,
    D3DSWAPEFFECT_COPY       = 3,
  } D3DSWAPEFFECT;

  typedef enum _D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE      = 0,
    D3DMULTISAMPLE_NONMASKABLE = 1,
  } D3DMULTISAMPLE_TYPE;

  typedef struct _D3DPRESENT_PARAMETERS {
    UINT BackBufferWidth;
    UINT BackBufferHeight;
    UINT BackBufferCount;
    D3DFORMAT BackBufferFormat;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    unsigned int MultiSampleQuality;
    D3DSWAPEFFECT SwapEffect;
    HWND hDeviceWindow;
    BOOL Windowed;
    BOOL EnableAutoDepthStencil;
    D3DFORMAT AutoDepthStencilFormat;
    unsigned int Flags;
    UINT FullScreen_RefreshRateInHz;
    UINT PresentationInterval;
  } D3DPRESENT_PARAMETERS;

  // ---- D3DDISPLAYMODE ----
  typedef struct _D3DDISPLAYMODE {
    UINT Width;
    UINT Height;
    UINT RefreshRate;
    D3DFORMAT Format;
  } D3DDISPLAYMODE;

  // ---- D3DADAPTER_IDENTIFIER9 ----
  typedef struct _D3DADAPTER_IDENTIFIER9 {
    char Driver[512];
    char Description[512];
    char DeviceName[32];
    unsigned long DriverVersionLowPart;
    unsigned long DriverVersionHighPart;
    unsigned long VendorId;
    unsigned long DeviceId;
    unsigned long SubSysId;
    unsigned long Revision;
    GUID DeviceIdentifier;
    unsigned long WHQLLevel;
  } D3DADAPTER_IDENTIFIER9;

  // ---- D3DVIEWPORT9 ----
  typedef struct _D3DVIEWPORT9 {
    UINT X;
    UINT Y;
    UINT Width;
    UINT Height;
    float MinZ;
    float MaxZ;
  } D3DVIEWPORT9;

  // ---- D3DPOOL ----
  typedef enum _D3DPOOL {
    D3DPOOL_DEFAULT      = 0,
    D3DPOOL_MANAGED      = 1,
    D3DPOOL_SYSTEMMEM    = 2,
  } D3DPOOL;

  // ---- D3DLOCK flags ----
  #ifndef D3DLOCK_READONLY
    #define D3DLOCK_READONLY       0x00000010
    #define D3DLOCK_DISCARD         0x00002000
    #define D3DLOCK_NOOVERWRITE     0x00001000
    #define D3DLOCK_NOSYSLOCK       0x00000800
    #define D3DLOCK_NO_DIRTY_UPDATE 0x00008000
  #endif

  // ---- Direct3D return codes ----
  #ifndef D3D_OK
    #define D3D_OK                  0
  #endif
  #ifndef D3DERR_INVALIDCALL
    #define D3DERR_INVALIDCALL      MAKEFOURCC('I','N','V','C')
    #define D3DERR_DEVICELOST       MAKEFOURCC('D','E','V','L')
  #endif

// ---- Platform path utilities ----
  #ifndef _MAX_FNAME
    #define _MAX_FNAME 256
  #endif
  #ifndef _MAX_EXT
    #define _MAX_EXT 256
  #endif
  #ifndef _MAX_DRIVE
    #define _MAX_DRIVE 3
  #endif
  #ifndef _MAX_DIR
    #define _MAX_DIR 256
  #endif

  // ---- RECT and POINT (Win32 types used in dx8wrapper.h) ----
  typedef struct tagRECT {
    long left;
    long top;
    long right;
    long bottom;
  } RECT;

  typedef struct tagPOINT {
    long x;
    long y;
  } POINT;

  // ---- D3DMATRIX ----
  typedef struct _D3DMATRIX {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
  } D3DMATRIX;

#endif // _WIN32

#endif // WW3D_PLATFORM_H