
#ifndef _TOKEN_H
#define _TOKEN_H

#include "stage.h"

#define CFG_OPEN '('
#define CFG_CLOSE ')'
#define STR_OPEN '\"'
#define STR_CLOSE '\"'
#define TPL_OPEN '['
#define TPL_CLOSE ']'

#ifdef __cplusplus
 extern "C" {
#endif 

typedef enum {
  STG_T_NUM = 0,
  STG_T_BOOLEAN,
  STG_T_MODELPROP,
  STG_T_WORLDPROP,
  STG_T_NAME,
  STG_T_STRING,
  STG_T_KEYWORD,
  STG_T_CFG_OPEN,
  STG_T_CFG_CLOSE,
  STG_T_TPL_OPEN,
  STG_T_TPL_CLOSE,
} stg_token_type_t;


typedef struct _stg_token {
  char* token; // the text of the token 
  stg_token_type_t type;
  unsigned int line; // the line on which the token appears 
  
  struct _stg_token* next; // linked list support
  struct _stg_token* child; // tree support
  
} stg_token_t;

stg_token_t* stg_token_next( stg_token_t* tokens );
// print <token> formatted for a human reader, with a string <prefix>
void stg_token_print( char* prefix,  stg_token_t* token );
// print a token array suitable for human reader
void stg_tokens_print( stg_token_t* tokens );
void stg_tokens_free( stg_token_t* tokens );

// create a new token structure from the arguments
stg_token_t* stg_token_create( char* token, stg_token_type_t type, int line );

// add a token to a list
stg_token_t* stg_token_append( stg_token_t* head, 
			       char* token, stg_token_type_t type, int line );

const char* stg_token_type_string( stg_token_type_t type );

#endif

#ifdef __cplusplus
 }
#endif 
