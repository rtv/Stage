
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define DEBUG

//#include "config.h"
#include "stage.h"
#include "stageclient.h"
#include "token.h"
#include "bitmap.h"

// the maximum size of a single token
#define TOKEN_MAX 256


static char* s_keywords[] = { "define", "include", "bitmap", NULL };
static char* s_modelprops[] = { "rects", "color", "size", "pose",  
				"file", "origin", "scale", 
				"name", "nose", "boundary", "nose", "movemask",
				"velocity", "matrix_render", "neighbor",
				"light", "grid", "shape", "fiducial_id", "port",
				NULL }; 

static char* s_worldprops[] = { "resolution", "showgrid",
				NULL }; 

const char* stg_token_type_string( stg_token_type_t type )
{
  switch( type )
    {
    case STG_T_NUM: return "NUMBER";
    case STG_T_BOOLEAN: return "BOOLEAN";
    case STG_T_MODELPROP: return "MODEL_PROPERTY";
    case STG_T_WORLDPROP: return "WORLD_PROPERTY";
    case STG_T_NAME: return "NAME";
    case STG_T_KEYWORD: return "KEYWORD";
    case STG_T_STRING: return "STRING";
    case STG_T_CFG_OPEN: return "CFG_OPEN";
    case STG_T_CFG_CLOSE: return "CFG_CLOSE";
    case STG_T_TPL_OPEN: return "TPL_OPEN";
    case STG_T_TPL_CLOSE: return "TPL_CLOSE";
    }
  return "<unknown>";
}


// a wrapper for fgetc that increments the lineno when it reads a
// newline
char stg_fgetc( FILE* f, int* lineno )
{
  char c = fgetc(f);
  if( c == '\n' ) (*lineno)++;
  return c;
}

// return 1 if the string occurs in the array, else 0.
int stg_string_in_array( char* string, char* array[] )
{
  char* astr; 
  while( (astr = *(array++)) )
    {
      //printf( "comparing %s to %s\n", string, astr );
      if( strcmp( string, astr ) == 0 )	return 1; // string found!
    }
  return 0; // string not found;
}

// create a flat list of tokens from world file <wf> 
stg_token_t* stg_tokenize( FILE* wf )
{
  // we start off with one empty token to mark the end of the array
  int token_len = 0; 
  char *token = (char*)calloc( 1, TOKEN_MAX );
  stg_token_t* tokens = NULL; // a linked list of token structures
  int line = 1;

  char c = stg_fgetc(wf,&line);

  while(1)
    {
      // consume any leading whitespace
      while( isspace(c) ) c = stg_fgetc(wf,&line);

      // check for end of file
      if( c == EOF )
	{
	  puts( "EOF" );
	  return tokens; 
	}

      // consume comments
      if( c == '#' ) while( c != '\n' && c != EOF ) c = stg_fgetc(wf,&line);

      // test for reserved single-character tokens
      if( c == TPL_OPEN )
	{
	  *token = c;
	  //printf( "tuple open: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_TPL_OPEN, line );
	  c = stg_fgetc(wf,&line);
	}

      if( c == TPL_CLOSE )
	{
	  *token = c;
	  //printf( "tuple close: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_TPL_CLOSE, line );
	  c = stg_fgetc(wf,&line);
	}

      if( c == CFG_OPEN )
	{
	  *token = c;
	  //printf( "tuple open: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_CFG_OPEN, line );
	  c = stg_fgetc(wf,&line);
	}

      if( c == CFG_CLOSE )
	{
	  *token = c;
	  //printf( "tuple open: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_CFG_CLOSE, line );
	  c = stg_fgetc(wf,&line);
	}

      // test for booleans
      if( c == 'T' || c == 'F' )
	{
	  *token = c;	  
	  tokens = stg_token_append( tokens, token, STG_T_BOOLEAN, line );
	  c = stg_fgetc(wf,&line);
	}      

      // test for numbers
      while( strchr( "+-.0123456789", c ) )
	{
	  token[token_len++] = c;
	  c = stg_fgetc(wf,&line);
	}

      if( token_len > 0 )
	{ 
	  //printf( "number: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_NUM, line );
	  memset( token, 0, TOKEN_MAX );
	  token_len = 0;
	}

      // test for strings
      if( c == '\"' )
	{
	  int string_start_line = line;

	  c = stg_fgetc(wf,&line);

	  while( c != '\"' )
	    {
	      if( c == EOF ) 
		{ 
		  printf( "syntax error line %d: "
			  "EOF before closing string quote.\n", 
			  string_start_line );
		  return NULL; // error
		}

	      if( c == '\n' )
		{
		  printf( "syntax error line %d: "
			  "newline before closing string quote. "
			  "Strings must be on a single line.\n",
			  string_start_line );
		  return NULL; // error
		}

	      token[token_len++] = c;
	      c = stg_fgetc(wf,&line);
	    }

	  c = stg_fgetc(wf,&line);
	  //printf( "string: %s\n", token );
	  tokens = stg_token_append( tokens, token, STG_T_STRING, line );
	  memset( token, 0, TOKEN_MAX );
	  token_len = 0;
	}

      // test for alphanumeric tokens (that begin with a letter) and
      // underscores - these are valid identifiers for models and
      // properties
      while( isalnum(c) || c == '_' )
	{
	  token[token_len++] = c;
	  c = stg_fgetc(wf,&line);
	}

      if( token_len > 0 )
	{ 
	  //printf( "alphanum: %s\n", token );

	  // is this a keyword?
	  if( stg_string_in_array( token, s_keywords ) )
	    tokens = stg_token_append( tokens, token, STG_T_KEYWORD, line );

	  // is this a property name?
	  else if( stg_string_in_array( token, s_modelprops ) )
	    tokens = stg_token_append( tokens, token, STG_T_MODELPROP, line  );

	  else if( stg_string_in_array( token, s_worldprops ) )
	    tokens = stg_token_append( tokens, token, STG_T_WORLDPROP, line  );

	  else // it must be a model name
	    tokens = stg_token_append( tokens, token, STG_T_NAME, line );

	  memset( token, 0, TOKEN_MAX);
	  token_len = 0;
	}
    }
}



sc_prop_t* stg_load_data( sc_prop_t* prop, stg_token_t** tokenptr )
{
  stg_token_t* tokens = *tokenptr;
  
  switch( tokens->type )
    {
    case STG_T_BOOLEAN:
      {
	int boo = atoi( tokens->token );
	printf( "appending int (bool) %d (size %d)\n ", boo, (int)sizeof(boo) );
	
	sc_prop_data_append( prop, STG_T_BOOLEAN, &boo, sizeof(boo) );
	
	tokens = stg_token_next(tokens);
      }
      break;

    case STG_T_NUM:
      {
	double num = strtod( tokens->token, NULL );

	printf( "%.3f ", num ); fflush(stdout);

	printf( "appending double %.2f (size %d)\n ", num, (int)sizeof(num) );

	sc_prop_data_append( prop, STG_T_NUM, &num, sizeof(double) );

	tokens = stg_token_next(tokens);
      }
      break;

    case STG_T_STRING:
      printf( "\"%s\" ", tokens->token ); fflush(stdout);

      sc_prop_data_append( prop, STG_T_STRING, 
			   tokens->token, 
			   strlen(tokens->token)+1 );

      //printf( "appending string \"%s\" (size %d) ", 
      //      tokens->token,
      //      strlen(tokens->token)+1 );

      tokens = stg_token_next(tokens);
      break;

    default:
      printf( "syntax error line %d: "
	      "expecting property value (number, string or tuple). "
	      "Found \"%s\" of type %s.\n", 
	      tokens->line,
	      tokens->token, 
	      stg_token_type_string(tokens->type) );
      exit( -1 );
    }

  *tokenptr = tokens; // export the new token position

  return prop;
}

stg_token_t* sc_model_load( sc_model_t* model, stg_token_t* tokens )
{
  printf( "configuring model: %d.", model->id_client );

  int ptype = 0;
  for( ptype=0; ptype<STG_PROP_COUNT; ptype++ )
    if( strcmp( tokens->token, stg_property_string(ptype) ) == 0 )
      {
	printf( "%s: ", stg_property_string(ptype) ); 
	break;
      }
  
  // create a new property
  sc_prop_t* prop = sc_prop_create();
  prop->id = ptype;
  prop->token = tokens;

  tokens = stg_token_next(tokens);

  switch( tokens->type )
    {
    case STG_T_KEYWORD:
      PRINT_WARN1( "KEYWORD: %s", tokens->token );
      break;

    case STG_T_TPL_OPEN:
      tokens = stg_token_next(tokens);
      while( tokens->type != STG_T_TPL_CLOSE )
	{       
	  prop = stg_load_data( prop, &tokens );
	  //printf( "skipping %s\n", tokens->token );
	  //tokens = stg_token_next(tokens); // consume the tuple closing brace
	}

      tokens = stg_token_next(tokens); // consume the tuple closing brace
      break;

    default:
      prop = stg_load_data( prop, &tokens );
    }

  // install the new property data in the model
  sc_model_attach_prop( model, prop );

  puts( "" );

  return tokens;
}

stg_token_t* sc_world_load_config( sc_world_t* world, stg_token_t* tokens )
{
  printf( "world: %d.", world->id_client );

  int ptype = 0;

  if( strcmp( tokens->token, s_worldprops[0] ) == 0 )
    ptype = 1;
  else if( strcmp( tokens->token, s_worldprops[1] ) == 0 )
    ptype = 2;

  // create a new property
  sc_prop_t* prop = sc_prop_create();
  prop->id = ptype;
  prop->token = tokens;

  tokens = stg_token_next(tokens);

  switch( tokens->type )
    {
    case STG_T_TPL_OPEN:
      tokens = stg_token_next(tokens);
      while( tokens->type != STG_T_TPL_CLOSE )
	{       
	  prop = stg_load_data( prop, &tokens );
	  //printf( "skipping %s\n", tokens->token );
	  tokens = stg_token_next(tokens); // consume the tuple closing brace
	}

      tokens = stg_token_next(tokens); // consume the tuple closing brace
      break;

    default:
      prop = stg_load_data( prop, &tokens );
      break;
    }

  // install the new property data in the model
  g_ptr_array_add( world->props, prop );

  puts( "" );

  return tokens;
}

stg_token_t* stg_load_keyword( stg_token_t* tokens )
{
  if( strcmp( tokens->token, "include" ) == 0 )
    {
      tokens = stg_token_next(tokens);

      if( tokens->type != STG_T_STRING )
	{
	  printf( "syntax error line %d: "
		  "\"include\" directives must be followed by a string. "
		  "Found \"%s\" of type %s.",
		  tokens->line,
		  tokens->token, 
		  stg_token_type_string(tokens->type) ); 
	  return NULL; 
	}
      
      PRINT_WARN2( "include file \"%s\" on line %d: INCLUDE IS NOT YET IMPLEMENTED", 
		   tokens->token, tokens->line );
      
      tokens = stg_token_next(tokens);
    }
  
  else if( strcmp( tokens->token, "define" ) == 0 )
    {
      PRINT_WARN2( "define macro \"%s\" on line %d: DEFINE IS NOT YET IMPLEMENTED.", 
		   tokens->token, tokens->line );
      
      tokens = stg_token_next(tokens); // skip the define token & macro name
      tokens = stg_token_next(tokens); // skip the define token & macro name
    }

  return tokens;
}


stg_token_t* sc_model_bitmap_load( sc_model_t* mod, stg_token_t* tokens )
{
  tokens = stg_token_next( tokens );
  
  PRINT_DEBUG1( "loading bitmap file \"%s\"", tokens->token );
  
  // load the bitmap and convert it to a bunch of rectangles
  stg_rotrect_t* rects;
  int num_rects = 0;
  stg_load_image( tokens->token, &rects, &num_rects );
  
  // add the rectangles to the model as a rects property
  PRINT_DEBUG1( "loaded %d rects", num_rects );
  
  // create a new property
  sc_prop_t* prop = sc_prop_create();
  prop->id = STG_PROP_RECTS;
  prop->token = tokens;
  
  // stick the rect data in the property
  sc_prop_data_append( prop, 0, rects, num_rects * sizeof(stg_rotrect_t) );
  
  free( rects );

  // install the new property data in the model
  g_ptr_array_add( mod->props, prop );

  tokens = stg_token_next( tokens );
  return tokens;
}

// recursively loads and configures models from the token array
sc_model_t*  sc_world_load_model( sc_world_t* world, sc_model_t* parent, 
				  stg_token_t** tokensptr )
{
  stg_token_t* tokens = *tokensptr;
  
  printf( "load model name %s line %d\n", tokens->token, tokens->line );
  if( tokens->type != STG_T_NAME )
    {
      printf( "syntax error line %d: "
	      "expecting a world or model name. "
	      "Found \"%s\" of type %s\n", 
	      tokens->line,
	      tokens->token,
	      stg_token_type_string( tokens->type ));
      return NULL;
    }
  
  // create a new model structure
  sc_model_t* model = sc_model_create( world, parent, tokens ) ;  
  // and add it to the world
  sc_world_addmodel( world, model );

  // now load other properties from the file
  tokens = stg_token_next(tokens);
  
  if( tokens->type != STG_T_CFG_OPEN )
    { 
      printf( "syntax error line %d: "
	      "expecting a block opening \"(\" to follow model name \"%s\". "
	      "Found \"%s\" of type %s instead.\n", 
	      tokens->line,
	      model->token->token,
	      tokens->token,
	      stg_token_type_string( tokens->type ) );
      return NULL;
    }
  
  tokens = stg_token_next(tokens);
  
  printf( "creating \"%s\" line %d\n", 
	  model->token->token, model->token->line );
  
  // parse the body of the block
  while( tokens && tokens->type != STG_T_CFG_CLOSE )
    {
      switch( tokens->type )
	{
	case STG_T_NAME: 
	  printf( "creating %s as child of %s\n", 
		  tokens->token, model->token->token );
	  // recurse here, adding the created child to the current model
	  sc_model_t* child = sc_world_load_model( world, model, &tokens );	  

	  if( child == NULL )
	    exit(0);
	  break;
	  
	case STG_T_KEYWORD:
	  if( strcmp( tokens->token, "bitmap" ) == 0 )
	    tokens = sc_model_bitmap_load( model, tokens );
	  else
	    {
	      printf( "syntax error line %d. Keyword %d can not be used in a model",
		      tokens->line, tokens->token );
	      exit(-1);
	    }
	  break;
	  
	case STG_T_MODELPROP:
	  tokens = sc_model_load( model, tokens );
	  break;
	  
	default:
	  printf( "syntax error line %d: expecting a property or model name,"
		  " or a model keyword."
		  "Found \"%s\" of type %s.\n", 
		  tokens->line,
		  tokens->token,
		  stg_token_type_string( tokens->type ) ); 
	  
	  exit(-1);
	}
    }
  
  tokens = stg_token_next(tokens); // consume the model close brace
  
  printf( "finished \"%s\"\n", model->token->token );  
  
  //if( tokens )
  //printf( "next token \"%s\"\n", tokens->token );
  
  *tokensptr = tokens; // poke the new token location into the argument
  return model; // return the newly created and configured model
  
}

// recursively loads and configures wlds from the token array
sc_world_t*  sc_load_worldblock( sc_t* cli, stg_token_t** tokensptr)
{
  stg_token_t* tokens = *tokensptr;

  // eat up any keywords before the world starts
  while( tokens && tokens->type == STG_T_KEYWORD )
    tokens = stg_load_keyword( tokens );

  printf( "load world name %s line %d\n", tokens->token, tokens->line );
  if( tokens->type != STG_T_NAME )
    {
      printf( "syntax error line %d: "
	      "expecting a world name. "
	      "Found \"%s\" of type %s\n", 
	      tokens->line,
	      tokens->token,
	      stg_token_type_string( tokens->type ));
      return NULL;
    }
   
  // create a new world
  sc_world_t* wld = sc_world_create( tokens );  
  // and add it to the client
  sc_addworld( cli, wld );
  
  // now load other properties from the file
  tokens = stg_token_next(tokens);

  if( tokens->type != STG_T_CFG_OPEN )
    { 
      printf( "syntax error line %d: "
	      "expecting a block opening \"(\" to follow wld name \"%s\". "
	      "Found \"%s\" of type %s instead.\n", 
	      tokens->line,
	      wld->token->token,
	      tokens->token,
	      stg_token_type_string( tokens->type ) );
      return NULL;
    }

  tokens = stg_token_next(tokens);

  // parse the body of the block
  while( tokens && tokens->type != STG_T_CFG_CLOSE )
    {
      switch( tokens->type )
	{
	case STG_T_NAME: 
	  printf( "creating model %s as child of %s\n", 
		  tokens->token, wld->token->token );
	  
	  sc_model_t* mod = sc_world_load_model( wld, NULL, &tokens );
	  if( mod == NULL )
	    exit(0);
	  break;

	case STG_T_WORLDPROP:
	  tokens = sc_world_load_config( wld, tokens );
	  break;

	default:
	  printf( "syntax error line %d: expecting a world property or model name. "
		  "Found \"%s\" of type %s.\n", 
		  tokens->line,
		  tokens->token,
		  stg_token_type_string( tokens->type ) ); 

	  exit(-1);
	}
    }

  tokens = stg_token_next(tokens); // consume the model close brace

  printf( "finished \"%s\"\n", wld->token->token );  

  //if( tokens )
  //printf( "next token \"%s\"\n", tokens->token );

  *tokensptr = tokens; // poke the new token location into the argument
  return wld; // return the newly created and configured model
}
