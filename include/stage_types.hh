///////////////////////////////////////////////////////////////////////////
//
// File: stage_types.hh
// Author: Andrew Howard
// Date: 12 Mar 2001
// Desc: Types and macros for stage
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/stage_types.hh,v $
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

#ifndef STAGE_TYPES_HH
#define STAGE_TYPES_HH

#include <stddef.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
//
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef M_PI
	#define M_PI        3.14159265358979323846
#endif

// Convert radians to degrees
//
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
//
#define DTOR(d) ((d) * M_PI / 180)

#define ASSERT(m) assert(m)

// Message macros
//
#define PRINT_MSG(m) printf("stage msg : %s : "m"\n", __FUNCTION__)
#define PRINT_MSG1(m, a) printf("stage msg : %s : "m"\n", __FUNCTION__, a)
#define PRINT_MSG2(m, a, b) printf("stage msg : %s : "m"\n", __FUNCTION__, a, b)

#endif
