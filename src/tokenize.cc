///////////////////////////////////////////////////////////////////////////
//
// File: tokenize.cc
// Author: Andrew Howard
// Date: 13 Mar 2000
// Desc: Functions for manipulating strings
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/tokenize.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.2 $
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokenize.hh"


///////////////////////////////////////////////////////////////////////////
// Tokenize a string
//
int Tokenize(char *buffer, int bufflen, char **argv, int maxargs)
{
    // Parse the line into tokens
    // Tokens are delimited by spaces
    //
    int argc = 0;
    char *token = buffer;
    while (true)
    {
        assert(argc < maxargs);
        argv[argc] = strsep(&token, " \t\n");
        if (token == NULL)
            break;
        if (argv[argc][0] != 0)
            argc++;
    }

    /*
    // Debugging -- dump the tokens
    //
    for (int i = 0; i < argc; i++)
        printf("[%s]", (char*) argv[i]);
    printf("\n");
    */
    
    return argc;
}
