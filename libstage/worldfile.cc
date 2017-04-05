/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001-2011 Richard Vaughan, Andrew Howard and Brian Gerkey.
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
 * CVS info: $Id$
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h> // for PATH_MAX
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define DEBUG

#include "replace.h" // for dirname(3)
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// the isblank() macro is not standard - it's a GNU extension
// and it doesn't work for me, so here's an implementation - rtv
#ifndef isblank
#define isblank(a) (a == ' ' || a == '\t')
#endif

///////////////////////////////////////////////////////////////////////////
// Useful macros for dumping parser errors
#define TOKEN_ERR(z, l) PRINT_ERR2("%s:%d : " z, this->filename.c_str(), l)
#define PARSE_ERR(z, l) PRINT_ERR2("%s:%d : " z, this->filename.c_str(), l)

///////////////////////////////////////////////////////////////////////////
// Default constructor
Worldfile::Worldfile()
    : tokens(), macros(), entities(), properties(), filename(), unit_length(1.0),
      unit_angle(M_PI / 180.0)
{
}

///////////////////////////////////////////////////////////////////////////
// Destructor
Worldfile::~Worldfile()
{
  ClearProperties();
  ClearMacros();
  ClearEntities();
  ClearTokens();
}

FILE *Worldfile::FileOpen(const std::string &filename, const char *method)
{
  FILE *fp = fopen(filename.c_str(), method);
  // if this opens, then we will go with it:
  if (fp) {
    PRINT_DEBUG1("Loading: %s", filename.c_str());
    return fp;
  }
  // else, search other places, and set this->filename
  // accordingly if found:
  char *stagepath = getenv("STAGEPATH");
  char *token = strtok(stagepath, ":");
  char *fullpath = new char[PATH_MAX];
  char *tmp = strdup(filename.c_str());
  const char *base = basename(tmp);
  while (token != NULL) {
    // for each part of the path, try it:
    memset(fullpath, 0, PATH_MAX);
    strcat(fullpath, token);
    strcat(fullpath, "/");
    strcat(fullpath, base);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    fp = fopen(fullpath, method);
    if (fp) {
      this->filename = std::string(fullpath);
      PRINT_DEBUG1("Loading: %s", filename.c_str());
      free(tmp);
      return fp;
    }
    token = strtok(NULL, ":");
  }
  if (tmp)
    free(tmp);
  if (fullpath)
    delete[] fullpath;
  return NULL;
}

///////////////////////////////////////////////////////////////////////////
// Load world file from stream
bool Worldfile::Load(std::istream &world_content, const std::string &filename)
{
  this->filename = filename; // required to resolve paths to relative-path based includes

  ClearTokens();

  // Read tokens from the stream
  if (!LoadTokens(world_content, 0))
    return false;

  return LoadCommon();
}

// Load world from file
bool Worldfile::Load(const std::string &filename)
{
  this->filename = filename;

  // Open the file
  FILE *file = FileOpen(this->filename, "r");
  if (!file) {
    PRINT_ERR2("unable to open world file %s : %s", this->filename.c_str(), strerror(errno));
    return false;
  }

  ClearTokens();

  // Read tokens from the file
  if (!LoadTokens(file, 0)) {
    // DumpTokens();
    fclose(file);
    return false;
  }

  fclose(file);
  return LoadCommon();
}

bool Worldfile::LoadCommon()
{
  // Parse the tokens to identify entities
  if (!ParseTokens()) {
    // DumpTokens();
    return false;
  }

  // Dump contents and exit if this file is meant for debugging only.
  if (ReadInt(0, "test", 0) != 0) {
    PRINT_ERR("this is a test file; quitting");
    DumpTokens();
    DumpMacros();
    DumpEntities();
    DumpProperties();
    return false;
  }

  // Work out what the length units are
  const std::string &unitl = ReadString(0, "unit_length", "m");
  if (unitl == "m")
    this->unit_length = 1.0;
  else if (unitl == "cm")
    this->unit_length = 0.01;
  else if (unitl == "mm")
    this->unit_length = 0.001;

  // Work out what the angle units are
  const std::string &unita = ReadString(0, "unit_angle", "degrees");
  if (unita == "degrees")
    this->unit_angle = M_PI / 180;
  else if (unita == "radians")
    this->unit_angle = 1;

  // printf( "f: %s\n", this->filename.c_str() );

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Save world to file
bool Worldfile::Save(const std::string &filename)
{
  // Debugging
  // DumpProperties();

  // Open file
  FILE *file = fopen(filename.c_str(), "w+");
  // FILE *file = FileOpen(filename, "w+");
  if (!file) {
    PRINT_ERR2("unable to save world file %s : %s", filename.c_str(), strerror(errno));
    return false;
  }

  // TODO - make a backup before saving the file

  // Write the current set of tokens to the file
  if (!SaveTokens(file)) {
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
  bool unused = false;

  FOR_EACH (it, properties) {
    if (!it->second->used) {
      PRINT_WARN3("worldfile %s:%d : property [%s] is defined but not used", this->filename.c_str(),
                  it->second->line, it->second->name.c_str());
      unused = true;
    }
  }

  return unused;
}

///////////////////////////////////////////////////////////////////////////
// Load tokens from a stream.
bool Worldfile::LoadTokens(std::istream &content, int include)
{
  int line;
  char token[256];

  line = 1;

  while (true) {
    int ch = content.get();
    if (ch == EOF)
      break;

    if ((char)ch == '#') {
      content.putback(ch);
      if (!LoadTokenComment(content, &line, include))
        return false;
    } else if (isalpha(ch)) {
      content.putback(ch);
      if (!LoadTokenWord(content, &line, include))
        return false;
    } else if (strchr("+-.0123456789", ch)) {
      content.putback(ch);
      if (!LoadTokenNum(content, &line, include))
        return false;
    } else if (isblank(ch)) {
      content.putback(ch);
      if (!LoadTokenSpace(content, &line, include))
        return false;
    } else if (ch == '"') {
      content.putback(ch);
      if (!LoadTokenString(content, &line, include))
        return false;
    } else if (strchr("(", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenEntity, token, include);
    } else if (strchr(")", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseEntity, token, include);
    } else if (strchr("[", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenTuple, token, include);
    } else if (strchr("]", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseTuple, token, include);
    } else if (0x0d == ch) {
      ch = content.get();
      if (0x0a != ch)
        content.putback(ch);
      line++;
      AddToken(TokenEOL, "\n", include);
    } else if (0x0a == ch) {
      ch = content.get();
      if (0x0d != ch)
        content.putback(ch);
      line++;
      AddToken(TokenEOL, "\n", include);
    } else {
      TOKEN_ERR("syntax error", line);
      return false;
    }
  }

  return true;
}

// Load tokens from a file.
bool Worldfile::LoadTokens(FILE *file, int include)
{
  int line;
  char token[256];

  line = 1;

  while (true) {
    int ch = fgetc(file);
    if (ch == EOF)
      break;

    if ((char)ch == '#') {
      ungetc(ch, file);
      if (!LoadTokenComment(file, &line, include))
        return false;
    } else if (isalpha(ch)) {
      ungetc(ch, file);
      if (!LoadTokenWord(file, &line, include))
        return false;
    } else if (strchr("+-.0123456789", ch)) {
      ungetc(ch, file);
      if (!LoadTokenNum(file, &line, include))
        return false;
    } else if (isblank(ch)) {
      ungetc(ch, file);
      if (!LoadTokenSpace(file, &line, include))
        return false;
    } else if (ch == '"') {
      ungetc(ch, file);
      if (!LoadTokenString(file, &line, include))
        return false;
    } else if (strchr("(", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenEntity, token, include);
    } else if (strchr(")", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseEntity, token, include);
    } else if (strchr("[", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenTuple, token, include);
    } else if (strchr("]", ch)) {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseTuple, token, include);
    } else if (0x0d == ch) {
      ch = fgetc(file);
      if (0x0a != ch)
        ungetc(ch, file);
      line++;
      AddToken(TokenEOL, "\n", include);
    } else if (0x0a == ch) {
      ch = fgetc(file);
      if (0x0d != ch)
        ungetc(ch, file);
      line++;
      AddToken(TokenEOL, "\n", include);
    } else {
      TOKEN_ERR("syntax error", line);
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Read in a comment token
bool Worldfile::LoadTokenComment(FILE *file, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = fgetc(file);

    if (ch == EOF) {
      AddToken(TokenComment, token, include);
      return true;
    } else if (0x0a == ch || 0x0d == ch) {
      ungetc(ch, file);
      AddToken(TokenComment, token, include);
      return true;
    } else
      token[len++] = ch;
  }
  return true;
}

bool Worldfile::LoadTokenComment(std::istream &content, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = content.get();

    if (ch == EOF) {
      AddToken(TokenComment, token, include);
      return true;
    } else if (0x0a == ch || 0x0d == ch) {
      content.putback(ch);
      AddToken(TokenComment, token, include);
      return true;
    } else
      token[len++] = ch;
  }
  return true;
}
///////////////////////////////////////////////////////////////////////////
// Read in a word token
bool Worldfile::LoadTokenWord(FILE *file, int *line, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = fgetc(file);

    if (ch == EOF) {
      AddToken(TokenWord, token, include);
      return true;
    } else if (isalpha(ch) || isdigit(ch) || strchr(".-_[]", ch)) {
      token[len++] = ch;
    } else {
      if (strcmp(token, "include") == 0) {
        ungetc(ch, file);
        AddToken(TokenWord, token, include);
        if (!LoadTokenInclude(file, line, include))
          return false;
      } else {
        ungetc(ch, file);
        AddToken(TokenWord, token, include);
      }
      return true;
    }
  }
  assert(false);
  return false;
}

bool Worldfile::LoadTokenWord(std::istream &content, int *line, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = content.get();

    if (ch == EOF) {
      AddToken(TokenWord, token, include);
      return true;
    } else if (isalpha(ch) || isdigit(ch) || strchr(".-_[]", ch)) {
      token[len++] = ch;
    } else {
      if (strcmp(token, "include") == 0) {
        content.putback(ch);
        AddToken(TokenWord, token, include);
        if (!LoadTokenInclude(content, line, include))
          return false;
      } else {
        content.putback(ch);
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
  const char *filename;
  char *fullpath;

  int ch = fgetc(file);

  if (ch == EOF) {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  } else if (!isblank(ch)) {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  ungetc(ch, file);
  if (!LoadTokenSpace(file, line, include))
    return false;

  ch = fgetc(file);

  if (ch == EOF) {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  } else if (ch != '"') {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  ungetc(ch, file);
  if (!LoadTokenString(file, line, include))
    return false;

  // This is the basic filename
  filename = GetTokenValue(this->tokens.size() - 1);

  // Now do some manipulation.  If its a relative path,
  // we append the path of the world file.
  if (filename[0] == '/' || filename[0] == '~') {
    fullpath = strdup(filename);
  } else if (this->filename[0] == '/' || this->filename[0] == '~') {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    fullpath = new char[PATH_MAX];
    memset(fullpath, 0, PATH_MAX);
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  } else {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    fullpath = new char[PATH_MAX];
    char *dummy = getcwd(fullpath, PATH_MAX);
    if (!dummy) {
      PRINT_ERR2("unable to get cwd %d: %s", errno, strerror(errno));
      if (tmp)
        free(tmp);
      delete[] fullpath;
      return false;
    }
    strcat(fullpath, "/");
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  }

  printf("[Include %s]", filename);
  fflush(stdout);

  // Open the include file
  FILE *infile = FileOpen(fullpath, "r");
  if (!infile) {
    PRINT_ERR2("unable to open include file %s : %s", fullpath, strerror(errno));
    // free(fullpath);
    delete[] fullpath;
    return false;
  }

  // Terminate the include line
  AddToken(TokenEOL, "\n", include);

  // DumpTokens();

  // Read tokens from the file
  if (!LoadTokens(infile, include + 1)) {
    fclose(infile);
    // DumpTokens();
    // free(fullpath);
    delete[] fullpath;
    return false;
  }

  // done with the include file
  fclose(infile);

  // consume the rest of the include line XX a bit of a hack - assumes
  // that an include is the last thing on a line
  while (ch != '\n')
    ch = fgetc(file);

  delete[] fullpath;
  return true;
}

bool Worldfile::LoadTokenInclude(std::istream &content, int *line, int include)
{
  const char *filename;
  char *fullpath;

  int ch = content.get();

  if (ch == EOF) {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  } else if (!isblank(ch)) {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  content.putback(ch);
  if (!LoadTokenSpace(content, line, include))
    return false;

  ch = content.get();

  if (ch == EOF) {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  } else if (ch != '"') {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  content.putback(ch);
  if (!LoadTokenString(content, line, include))
    return false;

  // This is the basic filename
  filename = GetTokenValue(this->tokens.size() - 1);

  // Now do some manipulation.  If its a relative path,
  // we append the path of the world file.
  if (filename[0] == '/' || filename[0] == '~') {
    fullpath = strdup(filename);
  } else if (this->filename[0] == '/' || this->filename[0] == '~') {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    fullpath = new char[PATH_MAX];
    memset(fullpath, 0, PATH_MAX);
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  } else {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    fullpath = new char[PATH_MAX];
    char *dummy = getcwd(fullpath, PATH_MAX);
    if (!dummy) {
      PRINT_ERR2("unable to get cwd %d: %s", errno, strerror(errno));
      if (tmp)
        free(tmp);
      delete[] fullpath;
      return false;
    }
    strcat(fullpath, "/");
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  }

  printf("[Include %s]", filename);
  fflush(stdout);

  // Open the include file
  FILE *infile = FileOpen(fullpath, "r");
  if (!infile) {
    PRINT_ERR2("unable to open include file %s : %s", fullpath, strerror(errno));
    // free(fullpath);
    delete[] fullpath;
    return false;
  }

  // Terminate the include line
  AddToken(TokenEOL, "\n", include);

  // DumpTokens();

  // Read tokens from the file
  if (!LoadTokens(infile, include + 1)) {
    fclose(infile);
    // DumpTokens();
    // free(fullpath);
    delete[] fullpath;
    return false;
  }

  // done with the include file
  fclose(infile);

  // consume the rest of the include line XX a bit of a hack - assumes
  // that an include is the last thing on a line
  while (ch != '\n')
    ch = content.get();

  delete[] fullpath;
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Read in a number token
bool Worldfile::LoadTokenNum(FILE *file, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = fgetc(file);

    if (ch == EOF) {
      AddToken(TokenNum, token, include);
      return true;
    } else if (strchr("+-.0123456789", ch)) {
      token[len++] = ch;
    } else {
      AddToken(TokenNum, token, include);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}
bool Worldfile::LoadTokenNum(std::istream &content, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = content.get();

    if (ch == EOF) {
      AddToken(TokenNum, token, include);
      return true;
    } else if (strchr("+-.0123456789", ch)) {
      token[len++] = ch;
    } else {
      AddToken(TokenNum, token, include);
      content.putback(ch);
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
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  int ch = fgetc(file);

  while (true) {
    ch = fgetc(file);

    if (EOF == ch || 0x0a == ch || 0x0d == ch) {
      TOKEN_ERR("unterminated string constant", *line);
      return false;
    } else if (ch == '"') {
      AddToken(TokenString, token, include);
      return true;
    } else {
      token[len++] = ch;
    }
  }
  assert(false);
  return false;
}
bool Worldfile::LoadTokenString(std::istream &content, int *line, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  int ch = content.get();

  while (true) {
    ch = content.get();

    if (EOF == ch || 0x0a == ch || 0x0d == ch) {
      TOKEN_ERR("unterminated string constant", *line);
      return false;
    } else if (ch == '"') {
      AddToken(TokenString, token, include);
      return true;
    } else {
      token[len++] = ch;
    }
  }
  assert(false);
  return false;
}
///////////////////////////////////////////////////////////////////////////
// Read in a whitespace token
bool Worldfile::LoadTokenSpace(FILE *file, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = fgetc(file);

    if (ch == EOF) {
      AddToken(TokenSpace, token, include);
      return true;
    } else if (isblank(ch)) {
      token[len++] = ch;
    } else {
      AddToken(TokenSpace, token, include);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}
bool Worldfile::LoadTokenSpace(std::istream &content, int *, int include)
{
  char token[256];

  int len = 0;
  memset(token, 0, sizeof(token));

  while (true) {
    int ch = content.get();

    if (ch == EOF) {
      AddToken(TokenSpace, token, include);
      return true;
    } else if (isblank(ch)) {
      token[len++] = ch;
    } else {
      AddToken(TokenSpace, token, include);
      content.putback(ch);
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
  for (unsigned int i = 0; i < this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    if (token->include > 0)
      continue;
    if (token->type == TokenString)
      fprintf(file, "\"%s\"", token->value.c_str());
    else
      fprintf(file, "%s", token->value.c_str());
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Clear the token list
void Worldfile::ClearTokens()
{
  tokens.clear();
}

///////////////////////////////////////////////////////////////////////////
// Add a token to the token list
bool Worldfile::AddToken(int type, const char *value, int include)
{
  tokens.push_back(CToken(include, type, value));
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Set a token value in the token list
bool Worldfile::SetTokenValue(int index, const char *value)
{
  assert(index >= 0 && index < (int)this->tokens.size());
  tokens[index].value = value;
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Get the value of a token
const char *Worldfile::GetTokenValue(int index)
{
  assert(index >= 0 && index < (int)this->tokens.size());
  return this->tokens[index].value.c_str();
}

///////////////////////////////////////////////////////////////////////////
// Dump the token list (for debugging).
void Worldfile::DumpTokens()
{
  int line;

  line = 1;
  printf("\n## begin tokens\n");
  printf("## %4d : ", line);

  FOR_EACH (it, tokens)
  // for (int i = 0; i < this->token_count; i++)
  {
    if (it->value[0] == '\n')
      printf("[\\n]\n## %4d : %02d ", ++line, it->include);
    else
      printf("[%s] ", it->value.c_str());
  }
  printf("\n");
  printf("## end tokens\n");
}

///////////////////////////////////////////////////////////////////////////
// Parse tokens into entities and properties.
bool Worldfile::ParseTokens()
{
  int entity;
  int line;

  ClearEntities();
  ClearProperties();

  // Add in the "global" entity.
  entity = AddEntity(-1, "");
  line = 1;

  for (int i = 0; i < (int)this->tokens.size(); i++) {
    CToken *token = &this->tokens[0] + i;

    switch (token->type) {
    case TokenWord:
      if (token->value == "include") {
        if (!ParseTokenInclude(&i, &line))
          return false;
      } else if (token->value == "define") {
        if (!ParseTokenDefine(&i, &line))
          return false;
      } else {
        if (!ParseTokenWord(entity, &i, &line))
          return false;
      }
      break;
    case TokenComment: break;
    case TokenSpace: break;
    case TokenEOL: line++; break;
    default: PARSE_ERR("syntax error 1", line); return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Parse an include statement
bool Worldfile::ParseTokenInclude(int *index, int *line)
{
  for (int i = *index + 1; i < (int)this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    switch (token->type) {
    case TokenString: break;
    case TokenSpace: break;
    case TokenEOL:
      *index = i;
      (*line)++;
      return true;
    default: PARSE_ERR("syntax error in include statement", *line); return false;
    }
  }
  PARSE_ERR("incomplete include statement", *line);
  return false;
}

///////////////////////////////////////////////////////////////////////////
// Parse a macro definition
bool Worldfile::ParseTokenDefine(int *index, int *line)
{
  const char *macroname, *entityname;
  int starttoken;

  macroname = NULL;
  entityname = NULL;
  starttoken = -1;

  for (int i = *index + 1, count = 0; i < (int)this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    switch (token->type) {
    case TokenWord:
      if (count == 0) {
        if (macroname == NULL)
          macroname = GetTokenValue(i);
        else if (entityname == NULL) {
          entityname = GetTokenValue(i);
          starttoken = i;
        } else {
          PARSE_ERR("extra tokens in macro definition", *line);
          return false;
        }
      } else {
        if (macroname == NULL) {
          PARSE_ERR("missing name in macro definition", *line);
          return false;
        }
        if (entityname == NULL) {
          PARSE_ERR("missing name in macro definition", *line);
          return false;
        }
      }
      break;
    case TokenOpenEntity: count++; break;
    case TokenCloseEntity:
      count--;
      if (count == 0) {
        AddMacro(macroname, entityname, *line, starttoken, i);
        *index = i;
        return true;
      }
      if (count < 0) {
        PARSE_ERR("misplaced ')'", *line);
        return false;
      }
      break;
    default: break;
    }
  }
  PARSE_ERR("missing ')'", *line);
  return false;
}

///////////////////////////////////////////////////////////////////////////
// Parse something starting with a word; could be a entity or an property.
bool Worldfile::ParseTokenWord(int entity, int *index, int *line)
{
  for (int i = *index + 1; i < (int)this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    switch (token->type) {
    case TokenComment: break;
    case TokenSpace: break;
    case TokenEOL: (*line)++; break;
    case TokenOpenEntity: return ParseTokenEntity(entity, index, line);
    case TokenNum:
    case TokenString:
    case TokenOpenTuple: return ParseTokenProperty(entity, index, line);
    default: PARSE_ERR("syntax error 2", *line); return false;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////
// Parse a entity from the token list.
bool Worldfile::ParseTokenEntity(int entity, int *index, int *line)
{
  int name;

  name = *index;
  CMacro *macro = LookupMacro(GetTokenValue(name));

  // If the entity name is a macro...
  if (macro) {
    int nentity = this->entities.size();
    int mindex = macro->starttoken;
    int mline = macro->line;
    if (!ParseTokenEntity(entity, &mindex, &mline))
      return false;
    entity = nentity;

    for (int i = *index + 1; i < (int)this->tokens.size(); i++) {
      CToken *token = &this->tokens[i];

      switch (token->type) {
      case TokenOpenEntity: break;
      case TokenWord:
        if (!ParseTokenWord(entity, &i, line))
          return false;
        break;
      case TokenCloseEntity: *index = i; return true;
      case TokenComment: break;
      case TokenSpace: break;
      case TokenEOL: (*line)++; break;
      default: PARSE_ERR("syntax error 3", *line); return false;
      }
    }
    PARSE_ERR("missing ')'", *line);
  }

  // If the entity name is not a macro...
  else {
    for (int i = *index + 1; i < (int)this->tokens.size(); i++) {
      CToken *token = &this->tokens[i];

      switch (token->type) {
      case TokenOpenEntity: entity = AddEntity(entity, GetTokenValue(name)); break;
      case TokenWord:
        if (!ParseTokenWord(entity, &i, line))
          return false;
        break;
      case TokenCloseEntity: *index = i; return true;
      case TokenComment: break;
      case TokenSpace: break;
      case TokenEOL: (*line)++; break;
      default: PARSE_ERR("syntax error 3", *line); return false;
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
  CProperty *property(NULL);
  int name(*index);

  for (int i = *index + 1; i < (int)this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    switch (token->type) {
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
      if (!ParseTokenTuple(property, &i, line))
        return false;
      *index = i;
      return true;
    case TokenSpace: break;
    default: PARSE_ERR("syntax error 4", *line); return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Parse a tuple.
bool Worldfile::ParseTokenTuple(CProperty *property, int *index, int *line)
{
  for (unsigned int i = *index + 1, count = 0; i < this->tokens.size(); i++) {
    CToken *token = &this->tokens[i];

    switch (token->type) {
    case TokenNum:
      AddPropertyValue(property, count++, i);
      *index = i;
      break;
    case TokenString:
      AddPropertyValue(property, count++, i);
      *index = i;
      break;
    case TokenCloseTuple: *index = i; return true;
    case TokenSpace: break;
    default: PARSE_ERR("syntax error 5", *line); return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// Clear the macro list
void Worldfile::ClearMacros()
{
  macros.clear();
}

///////////////////////////////////////////////////////////////////////////
// Add a macro
void Worldfile::AddMacro(const char *macroname, const char *entityname, int line, int starttoken,
                         int endtoken)
{
  macros.insert(std::pair<std::string, CMacro>(
      macroname, CMacro(macroname, entityname, line, starttoken, endtoken)));
}

///////////////////////////////////////////////////////////////////////////
// Lookup a macro by name
// Returns -1 if there is no macro with this name.
Worldfile::CMacro *Worldfile::LookupMacro(const char *macroname)
{
  std::map<std::string, CMacro>::iterator it = macros.find(macroname);

  if (it == macros.end())
    return NULL;
  else
    return &it->second;
}

///////////////////////////////////////////////////////////////////////////
// Dump the macro list for debugging
void Worldfile::DumpMacros()
{
  printf("\n## begin macros\n");

  FOR_EACH (it, macros)
  // for (int i = 0; i < this->macro_count; i++)
  {
    CMacro *macro = &(it->second); // this->macros + i;

    printf("## [%s][%s]", macro->macroname.c_str(), macro->entityname.c_str());
    for (int j = macro->starttoken; j <= macro->endtoken; j++) {
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
  // free(this->entities);
  // this->entities = NULL;
  // this->entity_size = 0;
  // this->entity_count = 0;

  this->entities.clear();
}

///////////////////////////////////////////////////////////////////////////
// Add a entity
int Worldfile::AddEntity(int parent, const char *type)
{
  // if (this->entity_count >= this->entity_size)
  //   {
  //     this->entity_size += 100;
  //     this->entities = (CEntity*)
  // realloc(this->entities, this->entity_size * sizeof(this->entities[0]));
  //   }

  // int entity = this->entity_count;
  // this->entities[entity].parent = parent;
  // this->entities[entity].type = type;
  // this->entity_count++;

  this->entities.push_back(CEntity(parent, type));
  return (entities.size() - 1); // index of the new entity
}

///////////////////////////////////////////////////////////////////////////
// Get the number of entities
int Worldfile::GetEntityCount()
{
  return this->entities.size();
}

///////////////////////////////////////////////////////////////////////////
// Get a entity's parent entity
int Worldfile::GetEntityParent(int entity)
{
  if (entity < 0 || entity >= (int)this->entities.size())
    return -1;
  return this->entities[entity].parent;
}

///////////////////////////////////////////////////////////////////////////
// Get a entity (returns the entity type value)
const char *Worldfile::GetEntityType(int entity)
{
  if (entity < 0 || entity >= (int)this->entities.size())
    return NULL;
  return this->entities[entity].type.c_str();
}

///////////////////////////////////////////////////////////////////////////
// Lookup a entity number by type name
// Returns -1 if there is no entity with this type
int Worldfile::LookupEntity(const char *type)
{
  for (int entity = 0; entity < GetEntityCount(); entity++) {
    if (strcmp(GetEntityType(entity), type) == 0)
      return entity;
  }
  return -1;
}

void PrintProp(const char *key, CProperty *prop)
{
  if (prop)
    printf("Print key %s prop ent %d name %s\n", key, prop->entity, prop->name.c_str());
}

///////////////////////////////////////////////////////////////////////////
// Dump the entity list for debugging
void Worldfile::DumpEntities()
{
  printf("\n## begin entities\n");

  FOR_EACH (it, properties)
    PrintProp(it->first.c_str(), it->second);

  printf("## end entities\n");
}

///////////////////////////////////////////////////////////////////////////
// Clear the property list
void Worldfile::ClearProperties()
{
  FOR_EACH (it, properties)
    delete it->second;
  properties.clear();
}

///////////////////////////////////////////////////////////////////////////
// Add an property
CProperty *Worldfile::AddProperty(int entity, const char *name, int line)
{
  char key[128];
  snprintf(key, 127, "%d%s", entity, name);

  CProperty *property = new CProperty(entity, name, line);

  properties[key] = property;

  return property;
}

///////////////////////////////////////////////////////////////////////////
// Add an property value
void Worldfile::AddPropertyValue(CProperty *property, int index, int value_token)
{
  assert(property);

  // Set the relevant value

  if (index >= (int)property->values.size())
    property->values.resize(index + 1);

  property->values[index] = value_token;
}

///////////////////////////////////////////////////////////////////////////
// Get an property
CProperty *Worldfile::GetProperty(int entity, const char *name)
{
  char key[128];
  snprintf(key, 127, "%d%s", entity, name);

  // printf( "looking up key %s for entity %d name %s\n", key, entity, name );

  static char cache_key[128] = { 0 };
  static CProperty *cache_property = NULL;

  if (strncmp(key, cache_key, 128) != 0) // different to last time
  {
    strncpy(cache_key, key, 128); // remember for next time

    std::map<std::string, CProperty *>::iterator it = properties.find(key);
    if (it == properties.end()) // not found
      cache_property = NULL;
    else
      cache_property = it->second;
  }
  // else
  // printf( "cache hit with %s\n", cache_key );

  return cache_property;
}

bool Worldfile::PropertyExists(int section, const char *token)
{
  return (bool)GetProperty(section, token);
}

///////////////////////////////////////////////////////////////////////////
// Set the value of an property
void Worldfile::SetPropertyValue(CProperty *property, int index, const char *value)
{
  assert(property);
  // printf( "property %s index %d values %d \n",
  //	  property->name.c_str(), index, (int)property->values.size() );

  assert(index >= 0 && index < (int)property->values.size());
  // Set the relevant value
  SetTokenValue(property->values[index], value);
}

///////////////////////////////////////////////////////////////////////////
// Get the value of an property
const char *Worldfile::GetPropertyValue(CProperty *property, int index)
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
const std::string Worldfile::ReadString(int entity, const char *name, const std::string &value)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return value;
  return GetPropertyValue(property, 0);
}

///////////////////////////////////////////////////////////////////////////
// Write a string
void Worldfile::WriteString(int entity, const char *name, const std::string &value)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return;
  SetPropertyValue(property, 0, value.c_str());
}

///////////////////////////////////////////////////////////////////////////
// Read an int
int Worldfile::ReadInt(int entity, const char *name, int value)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
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
  // compact zeros make the file more readable
  if (fabs(value) < 0.001) // nearly 0
    WriteString(entity, name, "0");
  else {
    char default_str[64];
    snprintf(default_str, sizeof(default_str), "%.3f", value);
    WriteString(entity, name, default_str);
  }
}

///////////////////////////////////////////////////////////////////////////
// Read a float
double Worldfile::ReadFloat(int entity, const char *name, double value)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return value;
  return atof(GetPropertyValue(property, 0));
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
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return value;
  const char *filename = GetPropertyValue(property, 0);

  if (filename[0] == '/' || filename[0] == '~')
    return filename;

  else if (this->filename[0] == '/' || this->filename[0] == '~') {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    char *fullpath = new char[PATH_MAX];
    memset(fullpath, 0, PATH_MAX);
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    if (tmp)
      free(tmp);
    return fullpath;
  } else {
    // Prepend the path
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename.c_str());
    char *fullpath = new char[PATH_MAX];
    char *dummy = getcwd(fullpath, PATH_MAX);
    if (!dummy) {
      PRINT_ERR2("unable to get cwd %d: %s", errno, strerror(errno));
      if (fullpath)
        delete[] fullpath;
      if (tmp)
        free(tmp);
      return value;
    }

    strcat(fullpath, "/");
    strcat(fullpath, dirname(tmp));
    strcat(fullpath, "/");
    strcat(fullpath, filename);
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);

    return fullpath;
  }
}

///////////////////////////////////////////////////////////////////////////
// Read a series of floats from a tuple (experimental)
int Worldfile::ReadTuple(const int entity, const char *name, const unsigned int first,
                         const unsigned int count, const char *format, ...)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return 0;

  if (property->values.size() < first + count) {
    PRINT_ERR4("Worldfile: reading tuple \"%s\" index %u to %u - tuple has "
               "length %u\n",
               name, first, first + count - 1, (unsigned int)property->values.size());
    exit(-1);
  }

  if (strlen(format) != count) {
    PRINT_ERR2("format string length %u does not match argument count %u",
               (unsigned int)strlen(format), count);
    exit(-1);
  }

  va_list args;
  va_start(args, format);

  for (unsigned int i = 0; i < count; i++) {
    const char *val = GetPropertyValue(property, first + i);

    switch (format[i]) {
    case 'i': // signed integer
      *va_arg(args, int *) = atoi(val);
      break;

    case 'u': // unsigned integer
      *va_arg(args, unsigned int *) = (unsigned int)atoi(val);
      break;

    case 'f': // float
      *va_arg(args, double *) = atof(val);
      break;

    case 'l': // length
      *va_arg(args, double *) = atof(val) * unit_length;
      break;

    case 'a': // angle
      *va_arg(args, double *) = atof(val) * unit_angle;
      break;

    case 's': // string
      *va_arg(args, char **) = strdup(val);
      break;

    default:
      PRINT_ERR3("Unknown format character %c in string %s loading %s", format[i], format, name);
    }
  }

  va_end(args);

  return count;
}

///////////////////////////////////////////////////////////////////////////
// Write a series of floats to a tuple (experimental)
void Worldfile::WriteTuple(const int entity, const char *name, const unsigned int first,
                           const unsigned int count, const char *format, ...)
{
  CProperty *property = GetProperty(entity, name);
  if (property == NULL)
    return;

  if (property->values.size() < first + count) {
    PRINT_ERR4("Worldfile: reading tuple \"%s\" index %u to %d - tuple has "
               "length %u\n",
               name, first, int(first + count) - 1,
               static_cast<unsigned int>(property->values.size()));
    exit(-1);
  }

  if (strlen(format) != count) {
    PRINT_ERR2("format string length %u does not match argument count %u",
               (unsigned int)strlen(format), count);
    exit(-1);
  }

  char buf[2048];
  buf[0] = 0;

  va_list args;
  va_start(args, format);

  for (unsigned int i = 0; i < count; i++) {
    switch (format[i]) {
    case 'i': // integer
      snprintf(buf, sizeof(buf), "%d", va_arg(args, int));
      break;

    case 'u': // unsigned integer
      snprintf(buf, sizeof(buf), "%u", va_arg(args, unsigned int));
      break;

    case 'f': // float
      snprintf(buf, sizeof(buf), "%.3f", va_arg(args, double));
      break;

    case 'l': // length
      snprintf(buf, sizeof(buf), "%.3f", va_arg(args, double) / unit_length);
      break;

    case 'a': // angle
      snprintf(buf, sizeof(buf), "%.3f", va_arg(args, double) / unit_angle);
      break;

    case 's': // string
      strncpy(buf, va_arg(args, char *), sizeof(buf));
      buf[sizeof(buf) - 1] = 0; // force zero terminator
      break;

    default:
      PRINT_ERR3("Unknown format character %c in string %s loading %s", format[i], format, name);
      exit(-1);
    }

    // printf( "writing %s %d %s\n", name, first+i, buf );
    SetPropertyValue(property, first + i, buf);
  }

  va_end(args);
}
