///////////////////////////////////////////////////////////////////////////
//
// File: tokenize.hh
// Author: Andrew Howard
// Date: 13 Mar 2000
// Desc: Functions for manipulating strings
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/tokenize.hh,v $
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

#ifndef TOKENIZE_HH
#define TOKENIZE_HH

// Tokenize a string (space delimited)
//
int Tokenize(char *buffer, int bufflen, char **argv, int maxargs);

#endif
