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
 * Author: Andrew Howard
 * Date: 15 Nov 2001
 * CVS info: $Id: worldfile.cc,v 1.12 2002-06-07 23:53:06 inspectorg Exp $
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h> // for dirname(3)
#include <limits.h> // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define DEBUG

#include "stage_types.hh"
#include "worldfile.hh"


///////////////////////////////////////////////////////////////////////////
// Useful macros for dumping parser errors
#define TOKEN_ERR(z, l) \
  PRINT_ERR2("%s:%d : " z, this->filename, l)
#define PARSE_ERR(z, l) \
  PRINT_ERR2("%s:%d : " z, this->filename, l)


///////////////////////////////////////////////////////////////////////////
// Default constructor
CWorldFile::CWorldFile() 
{
  this->filename = NULL;

  this->token_size = 0;
  this->token_count = 0;
  this->tokens = NULL;

  this->section_count = 0;
  this->section_size = 0;
  this->sections = NULL;

  this->item_count = 0;
  this->item_size = 0;
  this->items = NULL;

  // Set defaults units
  this->unit_length = 1.0;
  this->unit_angle = M_PI / 180;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorldFile::~CWorldFile()
{
  ClearItems();
  ClearSections();
  ClearTokens();

  if (this->filename)
    free(this->filename);
}


///////////////////////////////////////////////////////////////////////////
// Load world from file
bool CWorldFile::Load(const char *filename)
{
  // Shouldnt call load more than once,
  // so this should be null.
  assert(this->filename == NULL);
  this->filename = strdup(filename);

  // if the extension is 'm4', then run it through m4 first
  //printf("checking %s for .m4 extension\n", this->filename);
  if(strlen(this->filename) > 3 && 
     !strncmp(this->filename+(strlen(this->filename)-3),".m4",3))
  {
    //puts("it is m4");
    char* newfilename = strdup(this->filename);
    assert(newfilename);
    newfilename[strlen(newfilename)-3] = '\0';

    char* execstr = (char*)(malloc(strlen(this->filename)+strlen(newfilename)+8));
    assert(execstr);

    sprintf(execstr,"m4 -E %s > %s", this->filename, newfilename);

    //printf("calling system() with \"%s\"\n", execstr);
    if(system(execstr) < 0)
    {
      PRINT_ERR1("unable to invoke m4 on world file %s", this->filename);
      return false;
    }

    strcpy(this->filename, newfilename);
  }
  //puts("it is not m4");

  // Open the file
  FILE *file = fopen(this->filename, "r");
  if (!file)
  {
    PRINT_ERR2("unable to open world file %s : %s",
               this->filename, strerror(errno));
    return false;
  }

  // Read tokens from the file
  if (!LoadTokens(file))
  {
    //DumpTokens();
    return false;
  }

  // Parse the tokens to identify sections
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
    DumpSections();
    DumpItems();
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
bool CWorldFile::Save(const char *filename)
{
  // Debugging
  DumpItems();
  
  // If no filename is supplied, use default
  if (!filename)
    filename = this->filename;

  // Open file
  FILE *file = fopen(filename, "w+");
  if (!file)
  {
    PRINT_ERR2("unable to open world file %s : %s",
               filename, strerror(errno));
    return false;
  }

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
bool CWorldFile::WarnUnused()
{
  bool unused = false;
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    if (!pitem->used)
    {
      unused = true;
      PRINT_WARN3("worldfile %s:%d : property [%s] is defined but not used",
                  this->filename, pitem->line, pitem->name);
    }
  }
  return unused;
}


///////////////////////////////////////////////////////////////////////////
// Load tokens from a file.
bool CWorldFile::LoadTokens(FILE *file)
{
  int ch;
  int line;
  char token[256];

  ClearTokens();
  
  line = 1;

  while (true)
  {
    ch = fgetc(file);
    if (ch == EOF)
      break;
    
    if ((char) ch == '#')
    {
      ungetc(ch, file);
      if (!LoadTokenComment(file, &line))
        return false;
    }
    else if (isalpha(ch))
    {
      ungetc(ch, file);
      if (!LoadTokenWord(file, &line))
        return false;
    }
    else if (strchr("+-.0123456789", ch))
    {
      ungetc(ch, file);
      if (!LoadTokenNum(file, &line))
        return false;
    }
    else if (isblank(ch))
    {
      ungetc(ch, file);
      if (!LoadTokenSpace(file, &line))
        return false;
    }
    else if (ch == '"')
    {
      ungetc(ch, file);
      if (!LoadTokenString(file, &line))
        return false;
    }
    else if (strchr("(", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenSection, token);
    }
    else if (strchr(")", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseSection, token);
    }
    else if (strchr("[", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenTuple, token);
    }
    else if (strchr("]", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseTuple, token);
    }
    else if (ch == '\n')
    {
      line++;
      AddToken(TokenEOL, "\n");
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
bool CWorldFile::LoadTokenComment(FILE *file, int *line)
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
      AddToken(TokenComment, token);
      return true;
    }
    else if (ch == '\n')
    {
      ungetc(ch, file);
      AddToken(TokenComment, token);
      return true;
    }
    else
      token[len++] = ch;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a word token
bool CWorldFile::LoadTokenWord(FILE *file, int *line)
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
      AddToken(TokenWord, token);
      return true;
    }
    else if (isalpha(ch) || isdigit(ch) || strchr("_[]", ch))
    {
      token[len++] = ch;
    }
    else
    {
      AddToken(TokenWord, token);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a number token
bool CWorldFile::LoadTokenNum(FILE *file, int *line)
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
      AddToken(TokenNum, token);
      return true;
    }
    else if (strchr("+-.0123456789", ch))
    {
      token[len++] = ch;
    }
    else
    {
      AddToken(TokenNum, token);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a string token
bool CWorldFile::LoadTokenString(FILE *file, int *line)
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

    if (ch == EOF || ch == '\n')
    {
      TOKEN_ERR("unterminated string constant", line);
      return false;
    }
    else if (ch == '"')
    {
      AddToken(TokenString, token);
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
bool CWorldFile::LoadTokenSpace(FILE *file, int *line)
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
      AddToken(TokenSpace, token);
      return true;
    }
    else if (isblank(ch))
    {
      token[len++] = ch;
    }
    else
    {
      AddToken(TokenSpace, token);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Save tokens to a file.
bool CWorldFile::SaveTokens(FILE *file)
{
  int i;
  CToken *token;
  
  for (i = 0; i < this->token_count; i++)
  {
    token = this->tokens + i;

    if (token->type == TokenString)
      fprintf(file, "\"%s\"", token->value);  
    else
      fprintf(file, "%s", token->value);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Clear the token list
void CWorldFile::ClearTokens()
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
bool CWorldFile::AddToken(int type, const char *value)
{
  if (this->token_count >= this->token_size)
  {
    this->token_size += 1000;
    this->tokens = (CToken*) realloc(this->tokens, this->token_size * sizeof(this->tokens[0]));
  }

  this->tokens[this->token_count].type = type;
  this->tokens[this->token_count].value = strdup(value);
  this->token_count++;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Set a token value in the token list
bool CWorldFile::SetTokenValue(int index, const char *value)
{
  assert(index >= 0 && index < this->token_count);

  free(this->tokens[index].value);
  this->tokens[index].value = strdup(value);
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Get the value of a token
const char *CWorldFile::GetTokenValue(int index)
{
  assert(index >= 0 && index < this->token_count);

  return this->tokens[index].value;
}


///////////////////////////////////////////////////////////////////////////
// Dump the token list (for debugging).
void CWorldFile::DumpTokens()
{
  int line;

  line = 1;
  printf("\n## begin tokens\n");
  printf("## %4d : ", line);
  for (int i = 0; i < this->token_count; i++)
  {
    if (this->tokens[i].value[0] == '\n')
      printf("[\\n]\n## %4d : ", ++line);
    else
      printf("[%s] ", this->tokens[i].value);
  }
  printf("\n");
  printf("## end tokens\n");
}


///////////////////////////////////////////////////////////////////////////
// Parse tokens into sections and items.
bool CWorldFile::ParseTokens()
{
  int i;
  int line;
  CToken *token;

  ClearSections();
  ClearItems();
  
  // Add in the "global" section.
  AddSection(-1, "");
  
  line = 1;
  
  for (i = 0; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenWord:
        if (strcmp(token->value, "define") == 0)
        {
          if (!ParseTokenMacro(0, &i, &line))
            return false;
        }
        else
        {
          if (!ParseTokenWord(0, &i, &line))
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
// Parse something starting with a word; could be a section or an item.
bool CWorldFile::ParseTokenWord(int section, int *index, int *line)
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
      case TokenOpenSection:
        return ParseTokenSection(section, index, line);
      case TokenNum:
      case TokenString:
      case TokenOpenTuple:
        return ParseTokenItem(section, index, line);
      default:
        PARSE_ERR("syntax error 2", *line);
        return false;
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Parse a macro definition
bool CWorldFile::ParseTokenMacro(int section, int *index, int *line)
{
  int i;
  CToken *token;

  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Parse a section from the token list.
bool CWorldFile::ParseTokenSection(int section, int *index, int *line)
{
  int i;
  int name;
  CToken *token;

  name = *index;
  
  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenOpenSection:
        section = AddSection(section, GetTokenValue(name));
        break;
      case TokenWord:
        if (!ParseTokenWord(section, &i, line))
          return false;
        break;
      case TokenCloseSection:
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
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse an item from the token list.
bool CWorldFile::ParseTokenItem(int section, int *index, int *line)
{
  int i, item;
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
        item = AddItem(section, GetTokenValue(name), *line);
        AddItemValue(item, 0, i);
        *index = i;
        return true;
      case TokenString:
        item = AddItem(section, GetTokenValue(name), *line);
        AddItemValue(item, 0, i);
        *index = i;
        return true;
      case TokenOpenTuple:
        item = AddItem(section, GetTokenValue(name), *line);
        if (!ParseTokenTuple(section, item, &i, line))
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
bool CWorldFile::ParseTokenTuple(int section, int item, int *index, int *line)
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
        AddItemValue(item, count++, i);
        *index = i;
        break;
      case TokenString:
        AddItemValue(item, count++, i);
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
// Clear the section list
void CWorldFile::ClearSections()
{
  free(this->sections);
  this->sections = NULL;
  this->section_size = 0;
  this->section_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a section
int CWorldFile::AddSection(int parent, const char *name)
{
  if (this->section_count >= this->section_size)
  {
    this->section_size += 100;
    this->sections = (CSection*)
      realloc(this->sections, this->section_size * sizeof(this->sections[0]));
  }

  int section = this->section_count;
  this->sections[section].parent = parent;
  this->sections[section].name = name;
  this->section_count++;
  
  return section;
}


///////////////////////////////////////////////////////////////////////////
// Get the number of sections
int CWorldFile::GetSectionCount()
{
  return this->section_count;
}


///////////////////////////////////////////////////////////////////////////
// Get a section's parent section
int CWorldFile::GetSectionParent(int section)
{
  if (section < 0 || section >= this->section_count)
    return -1;
  return this->sections[section].parent;
}


///////////////////////////////////////////////////////////////////////////
// Get a section (returns the section type value)
const char *CWorldFile::GetSectionType(int section)
{
  if (section < 0 || section >= this->section_count)
    return NULL;

  return this->sections[section].name;
}


///////////////////////////////////////////////////////////////////////////
// Lookup a section number by type name
// Returns -1 if there is section with this type
int CWorldFile::LookupSection(const char *type)
{
  for (int section = 0; section < GetSectionCount(); section++)
  {
    if (strcmp(GetSectionType(section), type) == 0)
      return section;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Dump the section list for debugging
void CWorldFile::DumpSections()
{
  printf("\n## begin sections\n");
  for (int i = 0; i < this->section_count; i++)
  {
    CSection *section = this->sections + i;

    printf("## [%d][%d]", i, section->parent);
    printf("[%s]\n", section->name);
  }
  printf("## end sections\n");
}


///////////////////////////////////////////////////////////////////////////
// Clear the item list
void CWorldFile::ClearItems()
{
  int i;
  CItem *item;

  for (i = 0; i < this->item_count; i++)
  {
    item = this->items + i;
    free(item->values);
  }
  free(this->items);
  this->items = NULL;
  this->item_size = 0;
  this->item_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add an item
int CWorldFile::AddItem(int section, const char *name, int line)
{
  if (this->item_count >= this->item_size)
  {
    this->item_size += 100;
    this->items = (CItem*)
      realloc(this->items, this->item_size * sizeof(this->items[0]));
  }

  int i = this->item_count;

  CItem *item = this->items + i;
  memset(item, 0, sizeof(CItem));
  item->section = section;
  item->name = name;
  item->value_count = 0;
  item->values = NULL;
  item->line = line;
  item->used = false;

  this->item_count++;

  return i;
}


///////////////////////////////////////////////////////////////////////////
// Add an item value
void CWorldFile::AddItemValue(int item, int index, int value_token)
{
  assert(item >= 0);
  CItem *pitem = this->items + item;

  // Expand the array if it's too small
  if (index >= pitem->value_count)
  {
    pitem->value_count = index + 1;
    pitem->values = (int*) realloc(pitem->values, pitem->value_count * sizeof(int));
  }

  // Set the relevant value
  pitem->values[index] = value_token;
}


///////////////////////////////////////////////////////////////////////////
// Get an item 
int CWorldFile::GetItem(int section, const char *name)
{
  // Find first instance of item
  for (int i = 0; i < this->item_count; i++)
  {
    CItem *item = this->items + i;
    if (item->section != section)
      continue;
    if (strcmp(item->name, name) == 0)
      return i;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Set the value of an item
void CWorldFile::SetItemValue(int item, int index, const char *value)
{
  assert(item >= 0 && item < this->item_count);
  CItem *pitem = this->items + item;
  assert(index >= 0 && index < pitem->value_count);

  // Set the relevant value
  SetTokenValue(pitem->values[index], value);
}


///////////////////////////////////////////////////////////////////////////
// Get the value of an item 
const char *CWorldFile::GetItemValue(int item, int index)
{
  assert(item >= 0);
  CItem *pitem = this->items + item;
  assert(index < pitem->value_count);
  pitem->used = true;
  return GetTokenValue(pitem->values[index]);
}


///////////////////////////////////////////////////////////////////////////
// Dump the item list for debugging
void CWorldFile::DumpItems()
{
  printf("\n## begin items\n");
  for (int i = 0; i < this->item_count; i++)
  {
    CItem *item = this->items + i;
    CSection *section = this->sections + item->section;
    
    printf("## [%d]", item->section);
    printf("[%s]", section->name);
    printf("[%s]", item->name);
    for (int j = 0; j < item->value_count; j++)
      printf("[%s]", GetTokenValue(item->values[j]));
    printf("\n");
  }
  printf("## end items\n");
}


///////////////////////////////////////////////////////////////////////////
// Read a string
const char *CWorldFile::ReadString(int section, const char *name, const char *value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return GetItemValue(item, 0);
}


///////////////////////////////////////////////////////////////////////////
// Write a string
void CWorldFile::WriteString(int section, const char *name, const char *value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return;
  SetItemValue(item, 0, value);  
}


///////////////////////////////////////////////////////////////////////////
// Read an int
int CWorldFile::ReadInt(int section, const char *name, int value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atoi(GetItemValue(item, 0));
}


///////////////////////////////////////////////////////////////////////////
// Read a float
double CWorldFile::ReadFloat(int section, const char *name, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, 0));
}


///////////////////////////////////////////////////////////////////////////
// Read a length (includes unit conversion)
double CWorldFile::ReadLength(int section, const char *name, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, 0)) * this->unit_length;
}


///////////////////////////////////////////////////////////////////////////
// Write a length (includes units conversion)
void CWorldFile::WriteLength(int section, const char *name, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteString(section, name, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle (includes unit conversion)
double CWorldFile::ReadAngle(int section, const char *name, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, 0)) * this->unit_angle;
}


///////////////////////////////////////////////////////////////////////////
// Read a boolean
bool CWorldFile::ReadBool(int section, const char *name, bool value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  const char *v = GetItemValue(item, 0);
  if (strcmp(v, "true") == 0)
    return true;
  else if (strcmp(v, "false") == 0)
    return false;
  CItem *pitem = this->items + item;
  PRINT_WARN3("worldfile %s:%d : '%s' is not a valid boolean value; assuming 'false'",
              this->filename, pitem->line, v);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read a file name
// Always returns an absolute path.
// If the filename is entered as a relative path, we prepend
// the world files path to it.
// Known bug: will leak memory everytime it is called (but its not called often,
// so I cant be bothered fixing it).
const char *CWorldFile::ReadFilename(int section, const char *name, const char *value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  const char *filename = GetItemValue(item, 0);
  
  if( filename[0] == '/' || filename[0] == '~' )
    return filename;

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


///////////////////////////////////////////////////////////////////////////
// Read a string from a tuple
const char *CWorldFile::ReadTupleString(int section, const char *name,
                                        int index, const char *value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return GetItemValue(item, index);
}


///////////////////////////////////////////////////////////////////////////
// Write a string to a tuple
void CWorldFile::WriteTupleString(int section, const char *name,
                                  int index, const char *value)
{
  int item = GetItem(section, name);
  /* TODO
  if (item < 0)
    item = InsertItem(section, name);
  */
  SetItemValue(item, index, value);  
}


///////////////////////////////////////////////////////////////////////////
// Read a float from a tuple
double CWorldFile::ReadTupleFloat(int section, const char *name,
                                  int index, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, index));
}


///////////////////////////////////////////////////////////////////////////
// Write a float to a tuple
void CWorldFile::WriteTupleFloat(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value);
  WriteTupleString(section, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read a length from a tuple (includes unit conversion)
double CWorldFile::ReadTupleLength(int section, const char *name,
                                   int index, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, index)) * this->unit_length;
}


///////////////////////////////////////////////////////////////////////////
// Write a length to a tuple (includes unit conversion)
void CWorldFile::WriteTupleLength(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteTupleString(section, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle from a tuple (includes unit conversion)
double CWorldFile::ReadTupleAngle(int section, const char *name,
                                  int index, double value)
{
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return atof(GetItemValue(item, index)) * this->unit_angle;
}


///////////////////////////////////////////////////////////////////////////
// Write an angle to a tuple (includes unit conversion)
void CWorldFile::WriteTupleAngle(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_angle);
  WriteTupleString(section, name, index, default_str);
}





