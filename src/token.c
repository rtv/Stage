
#include "token.h"
//#include 

// TODO - proper destruction for these things to avoid memory leaks

stg_token_t* stg_token_next( stg_token_t* tokens )
{
  return tokens->next;
}

// print <token> formatted for a human reader, with a string <prefix>
void stg_token_print( char* prefix,  stg_token_t* token )
{
  printf( "%s %s \t%s\t%d\n", 
	  prefix, 
	  "fix me", //stg_token_type_string(token->type), 
	  token->token,
	  token->line );
}

// print a token array suitable for human reader
void stg_tokens_print( stg_token_t* tokens )
{
  puts( "tokens:" );

  int count = 0;
  while( tokens )
    {
      printf( "%d ", count++ );
      stg_token_print( "", tokens );
      tokens = stg_token_next( tokens );
    }
}

void stg_tokens_free( stg_token_t* tokens )
{
  while( tokens ) 
    {
      stg_token_t* doomed = tokens;
      tokens = stg_token_next( tokens );
      free( doomed );
    }
}

// create a new token structure from the arguments
stg_token_t* stg_token_create( char* token, stg_token_type_t type, int line )
{
  stg_token_t* infant = (stg_token_t*)calloc(sizeof(stg_token_t),1);

  if( infant == NULL )
    {
      printf( "failed to allocate structure for token \"%s\" line %d.\n",
	      token, line );
      return NULL;
    }

  infant->token = calloc( 1, strlen(token)+1 );
  if( infant->token == NULL )
    {
      printf( "failed to allocate string for token \"%s\" line %d.\n",
	      token, line );
      return NULL;
    }

  strcpy( infant->token, token );
  infant->type = type;
  infant->line = line;

  //stg_token_print( "created token", infant );

  return infant;
}


// add a token to a list
stg_token_t* stg_token_append( stg_token_t* head, 
			       char* token, stg_token_type_t type, int line )
{
  if( token == NULL )
    {
      printf( "warning: attempting to add NULL token. Ignoring." );
      return 0;
    }

  stg_token_t* tail = stg_token_create( token, type, line );

  //stg_token_print( "appending token", tail );

  if( head == NULL ) // if the list is empty
    head = tail; 
  else
    {
      // zip to the end of the list  and append the new tail
      stg_token_t* tit;
      for( tit = head; tit->next; tit = tit->next ) 
	{}

      tit->next = tail;
    }

  return head; // return the new head of the list
}
