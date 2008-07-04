/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: A class for reading in the world file.
 * Authors: Andrew Howard <ahoward@usc.edu>
 *          Richard Vaughan (rtv) <vaughan@sfu.ca>
 *          Douglas S. Blank <dblank@brynmawr.edu>
 *
 * Date: 15 Nov 2001
 * CVS info: $Id: worldfile.cc,v 1.4 2008-04-01 23:57:41 rtv Exp $
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h> // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

//#define DEBUG

#include "replace.h" // for dirname(3)
#include "stage_internal.hh"
//#include "worldfile.hh"

// the isblank() macro is not standard - it's a GNU extension
// and it doesn't work for me, so here's an implementation - rtv
#ifndef isblank
#define isblank(a) (a == ' ' || a == '\t')
#endif

///////////////////////////////////////////////////////////////////////////
// Useful macros for dumping parser errors
#define TOKEN_ERR(z, l)				\
  PRINT_ERR2("%s:%d : " z, this->filename, l)
#define PARSE_ERR(z, l)				\
  PRINT_ERR2("%s:%d : " z, this->filename, l)



// guint PropertyHash( const CProperty* prop )
// {
//   char key[128];
//   snprintf( key, 127, "%d%s", prop->entity, prop->name );
//   prop->key = strdup( key );

//   return g_str_hash( prop->key );
// }



///////////////////////////////////////////////////////////////////////////
// Default constructor
Worldfile::Worldfile() 
{
  this->filename = NULL;

  this->token_size = 0;
  this->token_count = 0;
  this->tokens = NULL;

  this->macro_count = 0;
  this->macro_size = 0;
  this->macros = NULL;

  this->entity_count = 0;
  this->entity_size = 0;
  this->entities = NULL;

  this->property_count = 0;
  //this->property_size = 0;
  //this->properties = NULL;

  // Set defaults units
  this->unit_length = 1.0;
  this->unit_angle = M_PI / 180;

  this->nametable = g_hash_table_new( g_str_hash, g_str_equal );
}


///////////////////////////////////////////////////////////////////////////
// Destructor
Worldfile::~Worldfile()
{
  ClearProperties();
  ClearMacros();
  ClearEntities();
  ClearTokens();

  g_hash_table_destroy( this->nametable );

  if (this->filename)
    free(this->filename);
}

FILE *Worldfile::FileOpen(const char *filename, const char* method)
{
  FILE *fp = fopen(filename, method);
  // if this opens, then we will go with it:
  if (fp) {
    PRINT_DEBUG1( "Loading: %s", filename);
    return fp;
  }
  // else, search other places, and set this->filename
  // accordingly if found:
  char *stagepath = getenv("STAGEPATH");
  char *token = strtok(stagepath, ":");
  char *fullpath = (char*) malloc(PATH_MAX);
  char *tmp = strdup(filename);
  char *base = basename(tmp);
  while (token != NULL) {
    // for each part of the path, try it:
    memset( fullpath, 0, PATH_MAX);
    strcat( fullpath, token);
    strcat( fullpath, "/" );
    strcat( fullpath, base);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    fp = fopen(fullpath, method);
    if (fp) {
      this->filename = fullpath;
      PRINT_DEBUG1( "Loading: %s", filename);
      free(tmp);
      return fp;
    }
    token = strtok(NULL, ":");
  }
  free(tmp);
  return NULL;
}


///////////////////////////////////////////////////////////////////////////
// Load world from file
bool Worldfile::Load(const char *filename)
{
  // Shouldnt call load more than once,
  // so this should be null.
  
  if(this->filename == NULL)
    {
      this->filename = strdup(filename);
    }
  
  
  // Open the file
  //FILE *file = fopen(this->filename, "r");
  FILE *file = FileOpen(this->filename, "r");
  if (!file)
    {
      PRINT_ERR2("unable to open world file %s : %s",
		 this->filename, strerror(errno));
      return false;
    }
  
  ClearTokens();
  
  // Read tokens from the file
  if (!LoadTokens(file, 0))
    {
      //DumpTokens();
      return false;
    }
  
  // Parse the tokens to identify entities
  if (!ParseTokens())
    {
      //DumpTokens();
      return false;
    }
  
  // Dump contents and exit if this file is meant for debugging only.
  if (ReadInt(0, "test", 0) != 0)
    {
      PRINT_ERR("this is a test file; quitting");
      DumpTokens();
      DumpMacros();
      DumpEntities();
      DumpProperties();
      return false;
    }
  
  // Work out what the length units are
  const char *unit = ReadString(0, "unit_length", "m");
  if (strcmp(unit, "m") == 0)
    this->unit_length = 1.0;
  else if (strcmp(unit, "cm") == 0)
    this->unit_length = 0.01;
  else if (strcmp(unit, "mm") == 0)
    this->unit_length = 0.001;
  
  // Work out what the angle units are
  unit = ReadString(0, "unit_angle", "degrees");
  if (strcmp(unit, "degrees") == 0)
    this->unit_angle = M_PI / 180;
  else if (strcmp(unit, "radians") == 0)
    this->unit_angle = 1;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save world to file
bool Worldfile::Save(const char *filename)
{
  // Debugging
  //DumpProperties();

  // If no filename is supplied, use default
  if (!filename)
    filename = this->filename;

  // Open file
  FILE *file = fopen(filename, "w+");
  //FILE *file = FileOpen(filename, "w+");
  if (!file)
    {
      PRINT_ERR2("unable to open world file %s : %s",
		 filename, strerror(errno));
      return false;
    }

  // TODO - make a backup before saving the file

  // Write the current set of tokens to the file
  if (!SaveTokens(file))
    {
      fclose(file);
      return false;
    }

  fclose(file);
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Check for unused properties and print warnings
bool Worldfile::WarnUnused()
{
  //   bool unused = false;
  //   for (int i = 0; i < this->property_count; i++)
  //   {
  //     CProperty *property = this->properties + i;
  //     if (!property->used)
  //     {
  //       unused = true;
  //       PRINT_WARN3("worldfile %s:%d : property [%s] is defined but not used",
  //                   this->filename, property->line, property->name);
  //     }
  //   }
  //   return unused;
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Load tokens from a file.
bool Worldfile::LoadTokens(FILE *file, int include)
{
  int ch;
  int line;
  char token[256];

  line = 1;

  while (true)
    {
      ch = fgetc(file);
      if (ch == EOF)
	break;

      if ((char) ch == '#')
	{
	  ungetc(ch, file);
	  if (!LoadTokenComment(file, &line, include))
	    return false;
	}
      else if (isalpha(ch))
	{
	  ungetc(ch, file);
	  if (!LoadTokenWord(file, &line, include))
	    return false;
	}
      else if (strchr("+-.0123456789", ch))
	{
	  ungetc(ch, file);
	  if (!LoadTokenNum(file, &line, include))
	    return false;
	}
      else if (isblank(ch))
	{
	  ungetc(ch, file);
	  if (!LoadTokenSpace(file, &line, include))
	    return false;
	}
      else if (ch == '"')
	{
	  ungetc(ch, file);
	  if (!LoadTokenString(file, &line, include))
	    return false;
	}
      else if (strchr("(", ch))
	{
	  token[0] = ch;
	  token[1] = 0;
	  AddToken(TokenOpenEntity, token, include);
	}
      else if (strchr(")", ch))
	{
	  token[0] = ch;
	  token[1] = 0;
	  AddToken(TokenCloseEntity, token, include);
	}
      else if (strchr("[", ch))
	{
	  token[0] = ch;
	  token[1] = 0;
	  AddToken(TokenOpenTuple, token, include);
	}
      else if (strchr("]", ch))
	{
	  token[0] = ch;
	  token[1] = 0;
	  AddToken(TokenCloseTuple, token, include);
	}
      else if ( 0x0d == ch )
	{
	  ch = fgetc(file);
	  if ( 0x0a != ch ) 
	    ungetc(ch, file);
	  line++;
	  AddToken(TokenEOL, "\n", include);
	}
      else if ( 0x0a == ch )
	{
	  ch = fgetc(file);
	  if ( 0x0d != ch ) 
	    ungetc(ch, file);
	  line++;
	  AddToken(TokenEOL, "\n", include);
	}
      else
	{
	  TOKEN_ERR("syntax error", line);
	  return false;
	}
    }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a comment token
bool Worldfile::LoadTokenComment(FILE *file, int *line, int include)
{
  char token[256];
  int len;
  int ch;

  len = 0;
  memset(token, 0, sizeof(token));

  while (true)
    {
      ch = fgetc(file);

      if (ch == EOF)
	{
	  AddToken(TokenComment, token, include);
	  return true;
	}
      else if ( 0x0a == ch || 0x0d == ch )
	{
	  ungetc(ch, file);
	  AddToken(TokenComment, token, include);
	  return true;
	}
      else
	token[len++] = ch;
    }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a word token
bool Worldfile::LoadTokenWord(FILE *file, int *line, int include)
{
  char token[256];
  int len;
  int ch;

  len = 0;
  memset(token, 0, sizeof(token));

  while (true)
    {
      ch = fgetc(file);

      if (ch == EOF)
	{
	  AddToken(TokenWord, token, include);
	  return true;
	}
      else if (isalpha(ch) || isdigit(ch) || strchr(".-_[]", ch))
	{
	  token[len++] = ch;
	}
      else
	{
	  if (strcmp(token, "include") == 0)
	    {
	      ungetc(ch, file);
	      AddToken(TokenWord, token, include);
	      if (!LoadTokenInclude(file, line, include))
		return false;
	    }
	  else
	    {
	      ungetc(ch, file);
	      AddToken(TokenWord, token, include);
	    }
	  return true;
	}
    }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Load an include token; this will load the include file.
bool Worldfile::LoadTokenInclude(FILE *file, int *line, int include)
{
  int ch;
  const char *filename;
  char *fullpath;

  ch = fgetc(file);

  if (ch == EOF)
    {
      TOKEN_ERR("incomplete include statement", *line);
      return false;
    }
  else if (!isblank(ch))
    {
      TOKEN_ERR("syntax error in include statement", *line);
      return false;
    }

  ungetc(ch, file);
  if (!LoadTokenSpace(file, line, include))
    return false;

  ch = fgetc(file);

  if (ch == EOF)
    {
      TOKEN_ERR("incomplete include statement", *line);
      return false;
    }
  else if (ch != '"')
    {
      TOKEN_ERR("syntax error in include statement", *line);
      return false;
    }

  ungetc(ch, file);
  if (!LoadTokenString(file, line, include))
    return false;

  // This is the basic filename
  filename = GetTokenValue(this->token_count - 1);

  // Now do some manipulation.  If its a relative path,
  // we append the path of the world file.
  if (filename[0] == '/' || filename[0] == '~')
    {
      fullpath = strdup(filename);
    }
  else if (this->filename[0] == '/' || this->filename[0] == '~')
    {
      // Note that dirname() modifies the contents, so
      // we need to make a copy of the filename.
      // There's no bounds-checking, but what the heck.
      char *tmp = strdup(this->filename);
      fullpath = (char*) malloc(PATH_MAX);
      memset(fullpath, 0, PATH_MAX);
      strcat( fullpath, dirname(tmp));
      strcat( fullpath, "/" );
      strcat( fullpath, filename );
      assert(strlen(fullpath) + 1 < PATH_MAX);
      free(tmp);
    }
  else
    {
      // Note that dirname() modifies the contents, so
      // we need to make a copy of the filename.
      // There's no bounds-checking, but what the heck.
      char *tmp = strdup(this->filename);
      fullpath = (char*) malloc(PATH_MAX);
      getcwd(fullpath, PATH_MAX);
      strcat( fullpath, "/" );
      strcat( fullpath, dirname(tmp));
      strcat( fullpath, "/" );
      strcat( fullpath, filename );
      assert(strlen(fullpath) + 1 < PATH_MAX);
      free(tmp);
    }

  printf( "[Include %s]", filename );
  fflush( stdout );

  // Open the include file
  FILE *infile = FileOpen(fullpath, "r");
  if (!infile)
    {
      PRINT_ERR2("unable to open include file %s : %s",
		 fullpath, strerror(errno));
      free(fullpath);
      return false;
    }

  // Terminate the include line
  AddToken(TokenEOL, "\n", include);

  //DumpTokens();

  // Read tokens from the file
  if (!LoadTokens(infile, include + 1))
    {
      //DumpTokens();
      free(fullpath);
      return false;
    }

  // consume the rest of the include line XX a bit of a hack - assumes
  // that an include is the last thing on a line
  while ( ch != '\n' )
    ch = fgetc(file);

  free(fullpath);
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a number token
bool Worldfile::LoadTokenNum(FILE *file, int *line, int include)
{
  char token[256];
  int len;
  int ch;

  len = 0;
  memset(token, 0, sizeof(token));

  while (true)
    {
      ch = fgetc(file);

      if (ch == EOF)
	{
	  AddToken(TokenNum, token, include);
	  return true;
	}
      else if (strchr("+-.0123456789", ch))
	{
	  token[len++] = ch;
	}
      else
	{
	  AddToken(TokenNum, token, include);
	  ungetc(ch, file);
	  return true;
	}
    }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a string token
bool Worldfile::LoadTokenString(FILE *file, int *line, int include)
{
  int ch;
  int len;
  char token[256];

  len = 0;
  memset(token, 0, sizeof(token));

  ch = fgetc(file);

  while (true)
    {
      ch = fgetc(file);

      if ( EOF == ch || 0x0a == ch || 0x0d == ch )
	{
	  TOKEN_ERR("unterminated string constant", *line);
	  return false;
	}
      else if (ch == '"')
	{
	  AddToken(TokenString, token, include);
	  return true;
	}
      else
	{
	  token[len++] = ch;
	}
    }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a whitespace token
bool Worldfile::LoadTokenSpace(FILE *file, int *line, int include)
{
  int ch;
  int len;
  char token[256];

  len = 0;
  memset(token, 0, sizeof(token));

  while (true)
    {
      ch = fgetc(file);

      if (ch == EOF)
	{
	  AddToken(TokenSpace, token, include);
	  return true;
	}
      else if (isblank(ch))
	{
	  token[len++] = ch;
	}
      else
	{
	  AddToken(TokenSpace, token, include);
	  ungetc(ch, file);
	  return true;
	}
    }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Save tokens to a file.
bool Worldfile::SaveTokens(FILE *file)
{
  int i;
  CToken *token;

  for (i = 0; i < this->token_count; i++)
    {
      token = this->tokens + i;

      if (token->include > 0)
	continue;
      if (token->type == TokenString)
	fprintf(file, "\"%s\"", token->value);
      else
	fprintf(file, "%s", token->value);
    }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Clear the token list
void Worldfile::ClearTokens()
{
  int i;
  CToken *token;

  for (i = 0; i < this->token_count; i++)
    {
      token = this->tokens + i;
      free(token->value);
    }
  free(this->tokens);
  this->tokens = 0;
  this->token_size = 0;
  this->token_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a token to the token list
bool Worldfile::AddToken(int type, const char *value, int include)
{
  if (this->token_count >= this->token_size)
    {
      this->token_size += 1000;
      this->tokens = (CToken*) realloc(this->tokens, this->token_size * sizeof(this->tokens[0]));
    }

  this->tokens[this->token_count].include = include;
  this->tokens[this->token_count].type = type;
  this->tokens[this->token_count].value = strdup(value);
  this->token_count++;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Set a token value in the token list
bool Worldfile::SetTokenValue(int index, const char *value)
{
  assert(index >= 0 && index < this->token_count);

  free(this->tokens[index].value);
  this->tokens[index].value = strdup(value);

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Get the value of a token
const char *Worldfile::GetTokenValue(int index)
{
  assert(index >= 0 && index < this->token_count);

  return this->tokens[index].value;
}


///////////////////////////////////////////////////////////////////////////
// Dump the token list (for debugging).
void Worldfile::DumpTokens()
{
  int line;

  line = 1;
  printf("\n## begin tokens\n");
  printf("## %4d : ", line);
  for (int i = 0; i < this->token_count; i++)
    {
      if (this->tokens[i].value[0] == '\n')
	printf("[\\n]\n## %4d : %02d ", ++line, this->tokens[i].include);
      else
	printf("[%s] ", this->tokens[i].value);
    }
  printf("\n");
  printf("## end tokens\n");
}


///////////////////////////////////////////////////////////////////////////
// Parse tokens into entities and properties.
bool Worldfile::ParseTokens()
{
  int i;
  int entity;
  int line;
  CToken *token;

  ClearEntities();
  ClearProperties();

  // Add in the "global" entity.
  entity = AddEntity(-1, "");
  line = 1;

  for (i = 0; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenWord:
	  if (strcmp(token->value, "include") == 0)
	    {
	      if (!ParseTokenInclude(&i, &line))
		return false;
	    }
	  else if (strcmp(token->value, "define") == 0)
	    {
	      if (!ParseTokenDefine(&i, &line))
		return false;
	    }
	  else
	    {
	      if (!ParseTokenWord(entity, &i, &line))
		return false;
	    }
	  break;
	case TokenComment:
	  break;
	case TokenSpace:
	  break;
	case TokenEOL:
	  line++;
	  break;
	default:
	  PARSE_ERR("syntax error 1", line);
	  return false;
	}
    }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Parse an include statement
bool Worldfile::ParseTokenInclude(int *index, int *line)
{
  int i;
  CToken *token;

  for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenString:
	  break;
	case TokenSpace:
	  break;
	case TokenEOL:
	  *index = i;
	  (*line)++;
	  return true;
	default:
	  PARSE_ERR("syntax error in include statement", *line);
	  return false;
	}
    }
  PARSE_ERR("incomplete include statement", *line);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse a macro definition
bool Worldfile::ParseTokenDefine(int *index, int *line)
{
  int i;
  int count;
  const char *macroname, *entityname;
  int starttoken;
  CToken *token;

  count = 0;
  macroname = NULL;
  entityname = NULL;
  starttoken = -1;

  for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenWord:
	  if (count == 0)
	    {
	      if (macroname == NULL)
		macroname = GetTokenValue(i);
	      else if (entityname == NULL)
		{
		  entityname = GetTokenValue(i);
		  starttoken = i;
		}
	      else
		{
		  PARSE_ERR("extra tokens in macro definition", *line);
		  return false;
		}
	    }
	  else
	    {
	      if (macroname == NULL)
		{
		  PARSE_ERR("missing name in macro definition", *line);
		  return false;
		}
	      if (entityname == NULL)
		{
		  PARSE_ERR("missing name in macro definition", *line);
		  return false;
		}
	    }
	  break;
	case TokenOpenEntity:
	  count++;
	  break;
	case TokenCloseEntity:
	  count--;
	  if (count == 0)
	    {
	      AddMacro(macroname, entityname, *line, starttoken, i);
	      *index = i;
	      return true;
	    }
	  if (count < 0)
	    {
	      PARSE_ERR("misplaced ')'", *line);
	      return false;
	    }
	  break;
	default:
	  break;
	}
    }
  PARSE_ERR("missing ')'", *line);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse something starting with a word; could be a entity or an property.
bool Worldfile::ParseTokenWord(int entity, int *index, int *line)
{
  int i;
  CToken *token;

  for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenComment:
	  break;
	case TokenSpace:
	  break;
	case TokenEOL:
	  (*line)++;
	  break;
	case TokenOpenEntity:
	  return ParseTokenEntity(entity, index, line);
	case TokenNum:
	case TokenString:
	case TokenOpenTuple:
	  return ParseTokenProperty(entity, index, line);
	default:
	  PARSE_ERR("syntax error 2", *line);
	  return false;
	}
    }

  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse a entity from the token list.
bool Worldfile::ParseTokenEntity(int entity, int *index, int *line)
{
  int i;
  int macro;
  int name;
  CToken *token;

  name = *index;
  macro = LookupMacro(GetTokenValue(name));

  // If the entity name is a macro...
  if (macro >= 0)
    {
      // This is a bit of a hack
      int nentity = this->entity_count;
      int mindex = this->macros[macro].starttoken;
      int mline = this->macros[macro].line;
      if (!ParseTokenEntity(entity, &mindex, &mline))
	return false;
      entity = nentity;

      for (i = *index + 1; i < this->token_count; i++)
	{
	  token = this->tokens + i;

	  switch (token->type)
	    {
	    case TokenOpenEntity:
	      break;
	    case TokenWord:
	      if (!ParseTokenWord(entity, &i, line))
		return false;
	      break;
	    case TokenCloseEntity:
	      *index = i;
	      return true;
	    case TokenComment:
	      break;
	    case TokenSpace:
	      break;
	    case TokenEOL:
	      (*line)++;
	      break;
	    default:
	      PARSE_ERR("syntax error 3", *line);
	      return false;
	    }
	}
      PARSE_ERR("missing ')'", *line);
    }

  // If the entity name is not a macro...
  else
    {
      for (i = *index + 1; i < this->token_count; i++)
	{
	  token = this->tokens + i;

	  switch (token->type)
	    {
	    case TokenOpenEntity:
	      entity = AddEntity(entity, GetTokenValue(name));
	      break;
	    case TokenWord:
	      if (!ParseTokenWord(entity, &i, line))
		return false;
	      break;
	    case TokenCloseEntity:
	      *index = i;
	      return true;
	    case TokenComment:
	      break;
	    case TokenSpace:
	      break;
	    case TokenEOL:
	      (*line)++;
	      break;
	    default:
	      PARSE_ERR("syntax error 3", *line);
	      return false;
	    }
	}
      PARSE_ERR("missing ')'", *line);
    }
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse an property from the token list.
bool Worldfile::ParseTokenProperty(int entity, int *index, int *line)
{
  int i;
  CProperty* property;
  int name, value, count;
  CToken *token;

  name = *index;
  value = -1;
  count = 0;

  for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenNum:
	  property = AddProperty(entity, GetTokenValue(name), *line);
	  AddPropertyValue(property, 0, i);
	  *index = i;
	  return true;
	case TokenString:
	  property = AddProperty(entity, GetTokenValue(name), *line);
	  AddPropertyValue(property, 0, i);
	  *index = i;
	  return true;
	case TokenOpenTuple:
	  property = AddProperty(entity, GetTokenValue(name), *line);
	  if (!ParseTokenTuple( property, &i, line))
	    return false;
	  *index = i;
	  return true;
	case TokenSpace:
	  break;
	default:
	  PARSE_ERR("syntax error 4", *line);
	  return false;
	}
    }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Parse a tuple.
bool Worldfile::ParseTokenTuple( CProperty* property, int *index, int *line)
{
  int i, count;
  CToken *token;

  count = 0;

  for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
	{
	case TokenNum:
	  AddPropertyValue(property, count++, i);
	  *index = i;
	  break;
	case TokenString:
	  AddPropertyValue(property, count++, i);
	  *index = i;
	  break;
	case TokenCloseTuple:
	  *index = i;
	  return true;
	case TokenSpace:
	  break;
	default:
	  PARSE_ERR("syntax error 5", *line);
	  return false;
	}
    }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Clear the macro list
void Worldfile::ClearMacros()
{
  free(this->macros);
  this->macros = NULL;
  this->macro_size = 0;
  this->macro_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a macro
int Worldfile::AddMacro(const char *macroname, const char *entityname,
			int line, int starttoken, int endtoken)
{
  if (this->macro_count >= this->macro_size)
    {
      this->macro_size += 100;
      this->macros = (CMacro*)
	realloc(this->macros, this->macro_size * sizeof(this->macros[0]));
    }

  int macro = this->macro_count;
  this->macros[macro].macroname = macroname;
  this->macros[macro].entityname = entityname;
  this->macros[macro].line = line;
  this->macros[macro].starttoken = starttoken;
  this->macros[macro].endtoken = endtoken;
  this->macro_count++;

  return macro;
}


///////////////////////////////////////////////////////////////////////////
// Lookup a macro by name
// Returns -1 if there is no macro with this name.
int Worldfile::LookupMacro(const char *macroname)
{
  int i;
  CMacro *macro;

  for (i = 0; i < this->macro_count; i++)
    {
      macro = this->macros + i;
      if (strcmp(macro->macroname, macroname) == 0)
	return i;
    }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Dump the macro list for debugging
void Worldfile::DumpMacros()
{
  printf("\n## begin macros\n");
  for (int i = 0; i < this->macro_count; i++)
    {
      CMacro *macro = this->macros + i;

      printf("## [%s][%s]", macro->macroname, macro->entityname);
      for (int j = macro->starttoken; j <= macro->endtoken; j++)
	{
	  if (this->tokens[j].type == TokenEOL)
	    printf("[\\n]");
	  else
	    printf("[%s]", GetTokenValue(j));
	}
      printf("\n");
    }
  printf("## end macros\n");
}


///////////////////////////////////////////////////////////////////////////
// Clear the entity list
void Worldfile::ClearEntities()
{
  free(this->entities);
  this->entities = NULL;
  this->entity_size = 0;
  this->entity_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a entity
int Worldfile::AddEntity(int parent, const char *type)
{
  if (this->entity_count >= this->entity_size)
    {
      this->entity_size += 100;
      this->entities = (CEntity*)
	realloc(this->entities, this->entity_size * sizeof(this->entities[0]));
    }

  int entity = this->entity_count;
  this->entities[entity].parent = parent;
  this->entities[entity].type = type;
  this->entity_count++;

  return entity;
}


///////////////////////////////////////////////////////////////////////////
// Get the number of entities
int Worldfile::GetEntityCount()
{
  return this->entity_count;
}


///////////////////////////////////////////////////////////////////////////
// Get a entity's parent entity
int Worldfile::GetEntityParent(int entity)
{
  if (entity < 0 || entity >= this->entity_count)
    return -1;
  return this->entities[entity].parent;
}


///////////////////////////////////////////////////////////////////////////
// Get a entity (returns the entity type value)
const char *Worldfile::GetEntityType(int entity)
{
  if (entity < 0 || entity >= this->entity_count)
    return NULL;
  return this->entities[entity].type;
}


///////////////////////////////////////////////////////////////////////////
// Lookup a entity number by type name
// Returns -1 if there is no entity with this type
int Worldfile::LookupEntity(const char *type)
{
  for (int entity = 0; entity < GetEntityCount(); entity++)
    {
      if (strcmp(GetEntityType(entity), type) == 0)
	return entity;
    }
  return -1;
}


void PrintProp( char* key, CProperty* prop, void* user )
{
  if( prop )
    printf( "Print key %s prop ent %d name %s\n", key, prop->entity, prop->name );
}

///////////////////////////////////////////////////////////////////////////
// Dump the entity list for debugging
void Worldfile::DumpEntities()
{
  printf("\n## begin entities\n");

  g_hash_table_foreach( this->nametable, (GHFunc)PrintProp, NULL );

  printf("## end entities\n");
}


///////////////////////////////////////////////////////////////////////////
// Clear the property list
void Worldfile::ClearProperties()
{
  this->property_count = 0;

  if( this->nametable )
    g_hash_table_destroy( this->nametable );
  this->nametable = g_hash_table_new( g_str_hash, g_str_equal );
}


///////////////////////////////////////////////////////////////////////////
// Add an property
CProperty* Worldfile::AddProperty(int entity, const char *name, int line)
{
  //int i;
  CProperty *property = g_new0( CProperty, 1 );

  property->entity = entity;
  property->name = name;
  property->value_count = 0;
  property->values = NULL;
  property->line = line;
  property->used = false;

  char key[128];
  snprintf( key, 127, "%d%s", entity, name );
  property->key = strdup( key );

  // add this property to a hash table keyed by name for fast lookup
  g_hash_table_insert( nametable, property->key, property );

  //printf( "added key %s for prop %p entity %d name %s\n", key, property, property->entity, property->name );

  this->property_count++;

  return property;
}


///////////////////////////////////////////////////////////////////////////
// Add an property value
void Worldfile::AddPropertyValue( CProperty* property, int index, int value_token)
{
  assert(property);

  // Expand the array if it's too small
  if (index >= property->value_count)
    {
      property->value_count = index + 1;
      property->values = (int*) realloc(property->values, property->value_count * sizeof(int));
    }

  // Set the relevant value
  property->values[index] = value_token;
}



///////////////////////////////////////////////////////////////////////////
// Get an property 
CProperty* Worldfile::GetProperty(int entity, const char *name)
{
  char key[128];
  snprintf( key, 127, "%d%s", entity, name );

  //  printf( "looking up key %s for entity %d name %s\n", key, entity, name );

  // g_hash_table_foreach( this->nametable, (GHFunc)PrintProp, NULL );

  CProperty* prop = (CProperty*)g_hash_table_lookup( this->nametable, key );

  //  if( prop )
  //     printf( "found entity %d name %s\n", prop->entity, prop->name );
  //   else
  //     printf( "key %s not found\n", key );

  return prop;
}


bool Worldfile::PropertyExists( int section, const char* token )
{
  return( this->GetProperty( section, token ) ? true : false );
}


///////////////////////////////////////////////////////////////////////////
// Set the value of an property
void Worldfile::SetPropertyValue( CProperty* property, int index, const char *value)
{
  assert( property );
  //  printf( "property %s index %d value_count %d \n",
  //  property->key, index, property->value_count );

  assert(index >= 0 && index < property->value_count);

  // Set the relevant value
  SetTokenValue( property->values[index], value);
}


///////////////////////////////////////////////////////////////////////////
// Get the value of an property 
const char *Worldfile::GetPropertyValue(CProperty* property, int index)
{
  assert(property);
  property->used = true;
  return GetTokenValue(property->values[index]);
}


///////////////////////////////////////////////////////////////////////////
// Dump the property list for debugging
void Worldfile::DumpProperties()
{
  printf("\n## begin properties\n");
  //   for (int i = 0; i < this->property_count; i++)
  //   {
  //     CProperty *property = this->properties + i;
  //     CEntity *entity = this->entities + property->entity;

  //     printf("## [%d]", property->entity);
  //     printf("[%s]", entity->type);
  //     printf("[%s]", property->name);
  //     for (int j = 0; j < property->value_count; j++)
  //       printf("[%s]", GetTokenValue(property->values[j]));
  //     printf("\n");
  //   }
  printf("## end properties\n");
}


///////////////////////////////////////////////////////////////////////////
// Read a string
const char *Worldfile::ReadString(int entity, const char *name, const char *value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return GetPropertyValue(property, 0);
}


///////////////////////////////////////////////////////////////////////////
// Write a string
void Worldfile::WriteString(int entity, const char *name, const char *value)
{
  CProperty* property = GetProperty(entity, name);
  if( property == NULL )
    return;
  SetPropertyValue(property, 0, value);
}


///////////////////////////////////////////////////////////////////////////
// Read an int
int Worldfile::ReadInt(int entity, const char *name, int value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atoi(GetPropertyValue(property, 0));
}


///////////////////////////////////////////////////////////////////////////
// Write an int
void Worldfile::WriteInt(int entity, const char *name, int value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%d", value);
  WriteString(entity, name, default_str);
}

///////////////////////////////////////////////////////////////////////////
// Write a float
void Worldfile::WriteFloat(int entity, const char *name, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value);
  WriteString(entity, name, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read a float
double Worldfile::ReadFloat(int entity, const char *name, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atof(GetPropertyValue(property, 0));
}


///////////////////////////////////////////////////////////////////////////
// Read a length (includes unit conversion)
double Worldfile::ReadLength(int entity, const char *name, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atof(GetPropertyValue(property, 0)) * this->unit_length;
}

///////////////////////////////////////////////////////////////////////////
// Write a length (includes units conversion)
void Worldfile::WriteLength(int entity, const char *name, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteString(entity, name, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle (includes unit conversion)
double Worldfile::ReadAngle(int entity, const char *name, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atof(GetPropertyValue(property, 0)) * this->unit_angle;
}

/* REMOVE?
///////////////////////////////////////////////////////////////////////////
// Read a boolean
bool Worldfile::ReadBool(int entity, const char *name, bool value)
{
//return (bool) ReadInt(entity, name, value);
CProperty* property = GetProperty(entity, name);
if (property < 0)
return value;
const char *v = GetPropertyValue(property, 0);
if (strcmp(v, "true") == 0 || strcmp(v, "yes") == 0)
return true;
else if (strcmp(v, "false") == 0 || strcmp(v, "no") == 0)
return false;
CProperty *pproperty = this->properties + property;
PRINT_WARN3("worldfile %s:%d : '%s' is not a valid boolean value; assuming 'false'",
this->filename, pproperty->line, v);
return false;
}
*/

 ///////////////////////////////////////////////////////////////////////////
 // Read a color (included text -> RGB conversion).
 // We look up the color in one of the common color databases.
uint32_t Worldfile::ReadColor(int entity, const char *name, uint32_t value)
{
  CProperty* property;
  const char *color;

  property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  color = GetPropertyValue(property, 0);

  // TODO: Hmmm, should do something with the default color here.
  return stg_lookup_color(color);
}


///////////////////////////////////////////////////////////////////////////
// Read a file name
// Always returns an absolute path.
// If the filename is entered as a relative path, we prepend
// the world files path to it.
// Known bug: will leak memory everytime it is called (but its not called often,
// so I cant be bothered fixing it).
const char *Worldfile::ReadFilename(int entity, const char *name, const char *value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  const char *filename = GetPropertyValue(property, 0);

  if( filename[0] == '/' || filename[0] == '~' )
    return filename;

  else if (this->filename[0] == '/' || this->filename[0] == '~')
    {
      // Note that dirname() modifies the contents, so
      // we need to make a copy of the filename.
      // There's no bounds-checking, but what the heck.
      char *tmp = strdup(this->filename);
      char *fullpath = (char*) malloc(PATH_MAX);
      memset(fullpath, 0, PATH_MAX);
      strcat( fullpath, dirname(tmp));
      strcat( fullpath, "/" );
      strcat( fullpath, filename );
      assert(strlen(fullpath) + 1 < PATH_MAX);
      free(tmp);
      return fullpath;
    }
  else
    {
      // Prepend the path
      // Note that dirname() modifies the contents, so
      // we need to make a copy of the filename.
      // There's no bounds-checking, but what the heck.
      char *tmp = strdup(this->filename);
      char *fullpath = (char*) malloc(PATH_MAX);
      getcwd(fullpath, PATH_MAX);
      strcat( fullpath, "/" );
      strcat( fullpath, dirname(tmp));
      strcat( fullpath, "/" );
      strcat( fullpath, filename );
      assert(strlen(fullpath) + 1 < PATH_MAX);
      free(tmp);

      return fullpath;
    }
}


///////////////////////////////////////////////////////////////////////////
// Read a string from a tuple
const char *Worldfile::ReadTupleString(int entity, const char *name,
				       int index, const char *value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL)
    return value;
  return GetPropertyValue(property, index);
}


///////////////////////////////////////////////////////////////////////////
// Write a string to a tuple
void Worldfile::WriteTupleString(int entity, const char *name,
				 int index, const char *value)
{
  CProperty* property = GetProperty(entity, name);

  if( property == NULL )
    {
      /* TODO
	 property = InsertProperty(entity, name);
	 SetPropertyValue(property, index, value);
      */
    }
  else
    SetPropertyValue(property, index, value);
}


///////////////////////////////////////////////////////////////////////////
// Read a float from a tuple
double Worldfile::ReadTupleFloat(int entity, const char *name,
				 int index, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atof(GetPropertyValue(property, index));
}


///////////////////////////////////////////////////////////////////////////
// Write a float to a tuple
void Worldfile::WriteTupleFloat(int entity, const char *name,
				int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value);
  WriteTupleString(entity, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read a length from a tuple (includes unit conversion)
double Worldfile::ReadTupleLength(int entity, const char *name,
				  int index, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL )
    return value;
  return atof(GetPropertyValue(property, index)) * this->unit_length;
}


///////////////////////////////////////////////////////////////////////////
// Write a length to a tuple (includes unit conversion)
void Worldfile::WriteTupleLength(int entity, const char *name,
				 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteTupleString(entity, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle from a tuple (includes unit conversion)
double Worldfile::ReadTupleAngle(int entity, const char *name,
				 int index, double value)
{
  CProperty* property = GetProperty(entity, name);
  if (property == NULL)
    return value;
  return atof(GetPropertyValue(property, index)) * this->unit_angle;
}


///////////////////////////////////////////////////////////////////////////
// Write an angle to a tuple (includes unit conversion)
void Worldfile::WriteTupleAngle(int entity, const char *name,
				int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_angle);
  WriteTupleString(entity, name, index, default_str);
}





