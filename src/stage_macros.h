#ifndef STAGE_MACROS_HH
#define STAGE_MACROS_HH

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#define DEBUG

///////////////////////////////////////////////////////////////////////////
// Some useful macros

// Determine size of array
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int) (sizeof(x) / sizeof(x[0]))
#endif

#define TRUE 1
#define FALSE 0

#define SUCCESS TRUE
#define FAIL FALSE

#define MILLION 1000000L

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef TWOPI
#define TWOPI 2 * M_PI
#endif

#define STAGE_SYNC 0
#define STAGE_ASYNC 1

// Convert radians to degrees
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180)

// Normalize angle to domain -pi, pi
#define NORMALIZE(z) atan2(sin(z), cos(z))

#define ASSERT(m) assert(m)

// Error macros
#define PRINT_ERR(m)         printf("\nstage error : %s : "m"\n", \
                                    __FILE__)
#define PRINT_ERR1(m, a)     printf("\nstage error : %s : "m"\n", \
                                    __FILE__, a)
#define PRINT_ERR2(m, a, b)  printf("\nstage error : %s : "m"\n", \
                                    __FILE__, a, b)

// Warning macros
#define PRINT_WARN(m)         printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_WARN1(m, a)     printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_WARN2(m, a, b)  printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_WARN3(m, a, b, c) printf("\nstage warning : %s %s "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)

// Message macros
#define PRINT_MSG(m) printf("stage msg : %s :\n  "m"\n", __FILE__)
#define PRINT_MSG1(m, a) printf("stage msg : %s :\n  "m"\n", __FILE__, a)
#define PRINT_MSG2(m, a, b) printf("stage msg : %s :\n  "m"\n", __FILE__, a, b)

// DEBUG macros
#ifdef DEBUG
#define PRINT_DEBUG(m)         printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__)
#define PRINT_DEBUG1(m, a)     printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a)
#define PRINT_DEBUG2(m, a, b)  printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b)
#define PRINT_DEBUG3(m, a, b, c) printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c)
#define PRINT_DEBUG4(m, a, b, c, d) printf("\rstage debug : %s %s\n  "m"\n", \
                                     __FILE__, __FUNCTION__, a, b, c, d)
#else
#define PRINT_DEBUG(m)
#define PRINT_DEBUG1(m, a)
#define PRINT_DEBUG2(m, a, b)
#define PRINT_DEBUG3(m, a, b, c)
#define PRINT_DEBUG4(m, a, b, c, d)
#endif

// these are lifted from ahoward's rtk2 library code
// Append an item to a linked list
#define STAGE_LIST_APPEND(head, item) \
item->prev = head;\
item->next = NULL;\
if (head == NULL)\
    head = item;\
else\
{\
    while (item->prev->next)\
       item->prev = item->prev->next;\
    item->prev->next = item;\
}

// Remove an item from a linked list
#define STAGE_LIST_REMOVE(head, item) \
if (item->prev)\
    item->prev->next = item->next;\
if (item->next)\
    item->next->prev = item->prev;\
if (item == head)\
    head = item->next;

#endif









