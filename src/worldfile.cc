///////////////////////////////////////////////////////////////////////////
//
// File: worldfile.cc
// Author: Andrew Howard
// Date: 15 Nov 2001
// Desc: A property handling class
//
// $Id: worldfile.cc,v 1.1 2001-11-17 00:24:12 ahoward Exp $
//
///////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stage_types.hh"
#include "worldfile.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
CWorldFile::CWorldFile()
{
  this->filename = NULL;
  
  // Create list to store section info
  this->section_count = 0;
  this->section_size = sizeof(this->sections) / sizeof(this->sections[0]);
  memset(this->sections, 0, sizeof(this->sections));
  
  // Add an initial section to store global settings
  AddSection(-1);

  // Create list to store item info
  this->item_count = 0;
  this->item_size = sizeof(this->items) / sizeof(this->items[0]);
  memset(this->items, 0, sizeof(this->items));
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorldFile::~CWorldFile()
{
  // Delete values stored in the item list
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    free(pitem->name);
    for (int index = 0; index < pitem->value_count; index++)
    {
      if (pitem->values[index])
        free(pitem->values[index]);
    }
  }

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

  // Open the file
  FILE *file = fopen(this->filename, "r");
  if (!file)
  {
    PRINT_ERR2("unable to open world file %s : %s",
               this->filename, strerror(errno));
    return false;
  }

  // Create a buffer to store the file
  int buff_len = 0;
  int buff_size = 0x10000L;
  char *buff = new char[buff_size];

  // Read the whole thing in;
  // what the hell, memory is cheap!
  fread(buff, buff_size, 1, file);
  if (ferror(file))
  {
    PRINT_ERR2("error reading world file %s : %s",
               this->filename, strerror(errno));
    fclose(file);
    return false;
  }
  else if (!feof(file))
  {
    PRINT_ERR2("world file %s is too large (%dKb)",
               this->filename, buff_len);
    fclose(file);
    return false;
  }
  buff_len = ftell(file);
  fclose(file);

  // Create a list to put the tokens into
  int token_count = 0;
  int token_size = 4096;
  char **tokens = new char*[token_size];

  // Tokenize the buffer
  if (!Tokenize(buff, buff_len, &token_count, token_size, tokens))
  {
    delete[] tokens;
    delete[] buff;
    return false;
  }

  // Parse the assembled tokens
  if (!Parse(token_count, tokens))
  {
    delete[] tokens;
    delete[] buff;
    return false;
  }

  // Clean-up
  delete[] tokens;
  delete[] buff;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save world to file
bool CWorldFile::Save(const char *filename)
{
  if (!filename)
    filename = this->filename;

  // Open file
  FILE *file = fopen("testx.world", "w+");
  if (!file)
  {
    PRINT_ERR2("unable to open world file %s : %s",
               this->filename, strerror(errno));
    return false;
  }

  // Write items to file
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;

    if (pitem->section == 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "set %s %s\n", pitem->name, pitem->values[0]);
    }
    else if (strcmp(pitem->name, "type") == 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "create %s ", pitem->values[0]);
    }
    else
    {
      if (pitem->value_count == 1)
        fprintf(file, "%s %s ", pitem->name, pitem->values[0]);
      else
      {
        fprintf(file, "(");
        for (int i = 0; i < pitem->value_count; i++)
        {
          if (i > 0)
            fprintf(file, " ");
          fprintf(file, "%s %s", pitem->name, pitem->values[i]);
        }
        fprintf(file, ")");
      }
    }
  }

  fclose(file);
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Tokenize a line
// Tokens are delimited by spaces.
// Tuples are delimted by parentheses.
// Eg "foo bar (1 2 3)" yields tokens "foo", "bar", "(" "1" "2" "3" ")".
bool CWorldFile::Tokenize(char *buff, int buff_len,
                          int *token_count, int token_size, char **tokens)

{
  int linecount = 1;

  assert(*token_count < token_size);
  tokens[*token_count] = buff;

  for (int i = 0; i < buff_len; i++)
  {
    assert(*token_count < token_size);

    if (strchr(" \t\r", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = buff + i;
    }
    else if (strchr("\n", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = "\n";
      (*token_count)++;
      tokens[*token_count] = buff + i;
    }
    else if (strchr("#", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = "#";
      (*token_count)++;
      tokens[*token_count] = buff + i;
    }
    else if (strchr("(", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = "(";
      (*token_count)++;
      tokens[*token_count] = buff + i;
    }
    else if (strchr(")", buff[i]))
    {
      buff[i] = 0;
      linecount++;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = ")";
      (*token_count)++;
      tokens[*token_count] = buff + i;
    }
    else if (tokens[*token_count][0] == 0)
      tokens[*token_count] = buff + i;
  }

  /* Dont remove!
  // Dump a token list for debugging
  for (int i = 0; i < *token_count; i++)
  {
    if (tokens[i][0] == '\n')
      printf("[\\n]\n");
    else
      printf("[%s] ", tokens[i]);
  }
  printf("\n");
  */

  return token_count;
}


///////////////////////////////////////////////////////////////////////////
// Parse tokens
bool CWorldFile::Parse(int token_count, char **tokens)
{
  int linecount = 1;
  int parent = 0;
  int section = 0;

  // Loop through the tokens
  for (int i = 0; i < token_count;)
  {
    // End-of-line
    // Increase the line counter
    if (tokens[i][0] == '\n')
    {
      linecount++;
      i++;
    }

    // Comments
    // Skip comments by consuming all takens up to but not including
    // the next end-of-line.
    else if (tokens[i][0] == '#')
    {
      while (i < token_count && tokens[i][0] != '\n')
        i++;
    }

    // Open block tokens
    // Make the current section the parent.
    else if (tokens[i][0] == '{')
    {
      parent = section;
      section = -1;
      i++;
    }

    // Close block tokens.
    // Make current sections parent the parent
    else if (strcmp(tokens[i], "}") == 0)
    {
      if (section > 0)
      {
        section = parent;
        parent = GetParentSection(section);
        i++;
      }
      else
      {
        PRINT_ERR2("world file %s:%d - extra '}'", this->filename, linecount);
        return false;
      }
    }

    // "set" command
    // set <name> <value>
    // Does not support tuples at the moment.
    else if (strcmp(tokens[i], "set") == 0 && i + 2 < token_count)
    {
      char *name = tokens[i + 1];
      char *value = tokens[i + 2];

      // TODO: Should do some syntax checking here.
      
      AddItem(0, name, 0, value, true);
      i += 3;
    }

    // "create" command
    // create <objecttype> [<name> <value> <name> <value> ...]
    else if (strcmp(tokens[i], "create") == 0 && i + 1 < token_count)
    {
      // Create a new section
      section = AddSection(parent);

      // Add a item describing the object type
      char *name = "type";
      char *value = tokens[i + 1];

      // TODO: Should do some syntax checking here.
      
      AddItem(section, name, 0, value, true);
      i += 2;
    }

    // Assume everything else is a <name value> tuple
    else if (i + 1 < token_count)
    {
      char *name = tokens[i + 0];
      char *value = tokens[i + 1];
      i += 2;

      // If the value not a tuple,
      // Just add a simple item.
      if (strcmp(value, "(") != 0)
      {
        if (strcmp(value, ")") == 0)
        {
          PRINT_ERR2("world file %s:%d - extra ')'",
                     this->filename, linecount);
          return false;
        }
        if (strcmp(value, "\n") == 0 || strcmp(value, "#") == 0)
        {
          PRINT_ERR2("world file %s:%d - missing value",
                     this->filename, linecount);
          return false;
        }
        AddItem(section, name, 0, value, true);
      }

      // Otherwise add a tuple
      // This means reading up to and including the close paren.
      else
      {
        int index = 0;
        while (i < token_count)
        {
          value = tokens[i++];
          if (strcmp(value, ")") == 0)
            break;
          if (strcmp(value, "(") == 0)
          {
            PRINT_ERR2("world file %s:%d - nested '(' not allowed",
                       this->filename, linecount);
            return false;
          }
          if (strcmp(value, "\n") == 0 || strcmp(value, "#") == 0)
          {
            PRINT_ERR2("world file %s:%d - missing ')'",
                       this->filename, linecount);
            return false;
          }
          AddItem(section, name, index++, value, true); 
        }
      }
    }

    // Error
    else
    {
      PRINT_ERR2("world file %s:%d - syntax error", this->filename, linecount);
      return false;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Add a section
int CWorldFile::AddSection(int parent)
{
  assert(this->section_count < this->section_size);

  // Add into section list
  int section = this->section_count++;
  CSection *psection = this->sections + section;
  psection->parent = parent;

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
int CWorldFile::GetParentSection(int section)
{
  if (section < 0 || section >= this->section_count)
    return -1;
  return this->sections[section].parent;
}


///////////////////////////////////////////////////////////////////////////
// Add an item
int CWorldFile::AddItem(int section, const char *name, int index,
                        const char *value, bool save)
{
  int item = GetItem(section, name, index);

  // If there is no such item in the list,
  // add the item now (re-sizing the list if necessary).
  if (item < 0)
  {
    assert(this->item_count < this->item_size);
    item = this->item_count++;
    CItem *pitem = this->items + item;
    pitem->section = section;
    pitem->name = strdup(name);
    pitem->value_count = 0;
    pitem->value_size = sizeof(pitem->values) / sizeof(pitem->values[0]);
    pitem->save = save;
  }

  // Now set the value at the matching index,
  CItem *pitem = this->items + item;
  assert(index < pitem->value_size);
  if (index >= pitem->value_count)
    pitem->value_count = index + 1;
  if (pitem->values[index])
    free(pitem->values[index]);
  pitem->values[index] = strdup(value);

  return item;
}


///////////////////////////////////////////////////////////////////////////
// Get an item 
int CWorldFile::GetItem(int section, const char *name, int index)
{
  // First see if we can find an instance of this item
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    if (pitem->section == section && strcmp(pitem->name, name) == 0)
      if (index < pitem->value_count && pitem->values[index] != NULL)
        return item;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Read a string
char *CWorldFile::ReadString(int section, const char *name, const char *defaultv)
{
  // See if there is a matching item it the list
  for (int i = 0; i < this->item_count; i++)
  {
    CItem *pitem = this->items + i;
    if (pitem->section == section && strcmp(pitem->name, name) == 0)
    {
      assert(pitem->value_count > 0);
      return pitem->values[0];
    }  
  }
   
  // If there isnt a matching item, add one
  int item = AddItem(section, name, 0, defaultv, false);
  return this->items[item].values[0];
}


///////////////////////////////////////////////////////////////////////////
// Read an int
int CWorldFile::ReadInt(int section, const char *name, int defaultv)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%d", defaultv);
  char *value = ReadString(section, name, default_str);
  return atoi(value);
}


///////////////////////////////////////////////////////////////////////////
// Read a float
double CWorldFile::ReadFloat(int section, const char *name, double defaultv)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", defaultv);
  char *value = ReadString(section, name, default_str);
  return atof(value);
}


///////////////////////////////////////////////////////////////////////////
// Read a string from a tuple
const char *CWorldFile::ReadTupleString(int section, const char *name,
                                  int index, const char *value)
{
  int item = GetItem(section, name, index);
  if (item < 0)
    return value;
  return this->items[item].values[index];
}


///////////////////////////////////////////////////////////////////////////
// Write a string to a tuple
void CWorldFile::WriteTupleString(int section, const char *name,
                                  int index, const char *value)
{
  // Get existing item or add one if it doesnt exist
  int item = AddItem(section, name, index, value, true);
  assert(item >= 0);
  CItem *pitem = this->items + item;

  // Set new value
  if (pitem->values[index])
    free(pitem->values[index]);
  pitem->values[index] = strdup(value);
}


///////////////////////////////////////////////////////////////////////////
// Read a float from a tuple
double CWorldFile::ReadTupleFloat(int section, const char *name,
                                  int index, double value)
{
  int item = GetItem(section, name, index);
  if (item < 0)
    return value;
  return atof(ReadTupleString(section, name, index, ""));
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
