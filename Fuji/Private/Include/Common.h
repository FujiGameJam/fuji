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

#if defined(_LINUX)
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <stdarg.h> // For varargs
#endif

#if defined(_FUJI_UTIL)
#if defined(WIN32)
	#include <Windows.h>
#endif
#endif

// SSE optimisations for xbox and PC?
#if defined(_XBOX) || defined(_WINDOWS)
//#define _FUJI_SSE
#endif

// if SSE optimisations are enabled, include SSE intrinsics header
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

#if defined(__GNUC__)
typedef unsigned long long	uint64;
typedef long long 			int64;
#else
typedef unsigned __int64	uint64;
typedef __int64				int64;
#endif
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
#if !defined(NULL) /* In case stdlib.h hasn't been included */
#define NULL 0
#endif

// callstack profiling
#if !defined(_RETAIL)
	#define _CALLSTACK_PROFILING

	#if defined(_CALLSTACK_PROFILING) && !defined(_DEBUG)
//		#define _CALLSTACK_MONITORING
	#endif
#endif

// useful macros
#if !defined(MIN)
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif

#if !defined(MAX)
#define MAX(a, b) ((a) > (b)? (a) : (b))
#endif

#define Clamp(x,y,z) MAX((x), MIN((y),(z)))

#define ALIGN16(x) (((x)+15) & 0xFFFFFFF0)

#define UNFLAG(x, y) (x&=~y)
#define FLAG(x, y) (x|=y)

// stricmp is a Win32/XBox only function
#if (!defined(_WIN32) && !defined(_XBOX))
#define stricmp strcasecmp
#define strnicmp strncasecmp

// So is OutputDebugString
#define OutputDebugString(s) fprintf(stderr, (s))
#define LPCTSTR char *

#endif


#include "Util.h"

#if !defined(_FUJI_UTIL)
	#include "Callstack.h"
	#include "Heap.h"
#endif

#endif // _COMMON_H
