#if !defined(_COMMON_H)
#define _COMMON_H

#if defined(_XBOX)
	#define DEBUG_KEYBOARD
	#include <xtl.h>
#endif

#if defined(_WINDOWS)
	#include <Windows.h>
	#include <d3d9.h>
	#include <d3dx9.h>
#endif

#if defined(_FUJI_UTIL)
	#include <Windows.h>
#endif

// SSE optimisations for xbox and PC?
#if defined(_XBOX) || defined(_WINDOWS)
//#define _FUJI_SSE
#endif

// if SSE optimisations enabled, include SSE intrinsics header
#if defined(_FUJI_SSE)
#include "xmmintrin.h"
#endif

// _ALIGN16 define
#if defined(_FUJI_SSE)
#define _ALIGN16 _MM_ALIGN16
#else
#define _ALIGN16
#endif

// Data Type Definition
#if defined(_FUJI_SSE)
typedef __m128				uint128;
#else
typedef unsigned int		uint128[4];
#endif

typedef unsigned __int64	uint64;
typedef __int64				int64;
typedef unsigned int		uint32;
typedef int					int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uint8;
typedef char				int8;

// defined values
#if !defined(PI)
#define PI 3.141592653589f
#endif
#define ALMOST_ZERO 0.000001f

// callstack profiling
#if !defined(_RETAIL)
	#define _CALLSTACK_PROFILING

	#if defined(_CALLSTACK_PROFILING) && !defined(_DEBUG)
		#define _CALLSTACK_MONITORING
	#endif
#endif

// useful macros
#define Clamp(x,y,z) max((x), min((y),(z)))

#define ALIGN16(x) (((x)+15) & 0xFFFFFFF0)

#define UNFLAG(x, y) (x&=~y)
#define FLAG(x, y) (x|=y)

#include "Util.h"

#if !defined(_FUJI_UTIL)
	#include "Callstack.h"
#endif


#if !defined(_FUJI_UTIL)
	#include "Heap.h"
#endif

#endif // _COMMON_H
