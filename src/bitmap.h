
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "stage.h"

#ifdef __cplusplus
 extern "C" {
#endif 

// the external interface to Stage's bitmap functions

// load <filename>, an image format understood by gdk-pixbuf, and
// return a set of rectangles that approximate the image. Caller must
// free the array of rectangles.
int stg_load_image( const char* filename, stg_rotrect_t** rects, int* rect_count );


#ifdef __cplusplus
 }
#endif 

