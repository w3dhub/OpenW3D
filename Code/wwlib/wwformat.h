#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define OPENW3D_PRINTF_VARARG_FUNC( fmtargnumber ) __attribute__ (( format( __printf__, fmtargnumber, fmtargnumber+1 )))
#define OPENW3D_PRINTF_VARARG_FUNCV( fmtargnumber ) __attribute__(( format( __printf__, fmtargnumber, 0 )))
#else
#define OPENW3D_PRINTF_VARARG_FUNC( fmtargnumber )
#define OPENW3D_PRINTF_VARARG_FUNCV( fmtargnumber )
#endif
