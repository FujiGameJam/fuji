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

#define PI 3.141592653589f
#define ALMOST_ZERO 0.000001f


// Data Type Definition
typedef unsigned int uint128[4];
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;

#include "Util.h"

#endif // _COMMON_H
