#include <stdio.h>
#include <stdarg.h>

/********* General *********/

/**
 * A variable-argument function that Zoo routines should call to report
 * errors that wouldn't stop the simulation (use stg_err for ones that
 * would).  The parameters work just like printf.
 */
int
zoo_err( const char *fmt, ... )
{
	va_list va;
	char fmt2[1024];
	int r;

	sprintf(fmt2, "Zoo: %s", fmt);
	va_start(va, fmt);
	r = vfprintf(stderr, fmt2, va);
	va_end(va);

	return r;
}
