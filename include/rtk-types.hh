///////////////////////////////////////////////////////////////////////////
//
// File: rtk-types.hh
// Author: Andrew Howard
// Date: 14 Nov 2000
// Desc: Basic type declarations
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/rtk-types.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////


#ifndef RTK_TYPES_H
#define RTK_TYPES_H


////////////////////////////////////////////////////////////////////////////////
// Maths stuff

#ifndef M_PI
	#define M_PI        3.14159265358979323846
#endif

// Convert radians to degrees
//
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
//
#define DTOR(d) ((d) * M_PI / 180)


////////////////////////////////////////////////////////////////////////////////
// Define length-specific data types
//
#define uint8_t unsigned char
#define UINT16 unsigned short

#define LOuint8_t(w) ((uint8_t) (w & 0xFF))
#define HIuint8_t(w) ((uint8_t) ((w >> 8) & 0xFF))
#define MAKEUINT16(lo, hi) ((((UINT16) (hi)) << 8) | ((UINT16) (lo)))

#ifndef bool
	#define bool int
#endif

#ifndef TRUE
	#define TRUE true
#endif

#ifndef FALSE
	#define FALSE false
#endif


////////////////////////////////////////////////////////////////////////////////
// Array checking macros

// Macro for returning array size
//
#ifndef ARRAYSIZE
	// Note that the cast to int is used to suppress warning about
	// signed/unsigned mismatches.
	#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

// Macro for checking array bounds
//
#define ASSERT_INDEX(index, array) \
    ASSERT((size_t) (index) >= 0 && (size_t) (index) < sizeof(array) / sizeof(array[0]));


////////////////////////////////////////////////////////////////////////////////
// Misc useful stuff

#define TRACE printf
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


////////////////////////////////////////////////////////////////////////////////
// Error, msg, trace macros

#include <assert.h>

#define ASSERT(m) assert(m)
#define VERIFY(m) assert(m)

#include <stdio.h>

#define ERROR(m)  printf("Error : %s : %s\n", __PRETTY_FUNCTION__, m)
#define MSG(m)       printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__)
#define MSG1(m, a)   printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a)
#define MSG2(m, a, b) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b)
#define MSG3(m, a, b, c) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c)
#define MSG4(m, a, b, c, d) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c, d)

#if ENABLE_TRACE
    #define TRACE0(m)    printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__)
    #define TRACE1(m, a) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a)
    #define TRACE2(m, a, b) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b)
    #define TRACE3(m, a, b, c) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c)
    #define TRACE4(m, a, b, c, d) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c, d)
#else
    #define TRACE0(m)
    #define TRACE1(m, a)
    #define TRACE2(m, a, b)
    #define TRACE3(m, a, b, c)
    #define TRACE4(m, a, b, c, d)
#endif


#endif
