///////////////////////////////////////////////////////////////////////////
//
// File: worldfile.cc
// Author: Andrew Howard
// Date: 15 Nov 2001
// Desc: A property handling class
//
// $Id: worldfile.cc,v 1.2 2001-11-17 21:45:39 ahoward Exp $
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
  // Debugging
  DumpItems();
  
  // If no filename is supplied, use default
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

    if (pitem->section < 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "%s", pitem->values[0]);
    }
    else if (pitem->section == 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "set %s %s", pitem->name, pitem->values[0]);
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
        fprintf(file, "%s (", pitem->name);
        for (int i = 0; i < pitem->value_count; i++)
        {
          if (i > 0)
            fprintf(file, " ");
          fprintf(file, "%s", pitem->values[i]);
        }
        fprintf(file, ") ");
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
  bool comment = false;
  
  assert(*token_count < token_size);
  tokens[*token_count] = buff;

  for (int i = 0; i < buff_len; i++)
  {
    assert(*token_count < token_size);

    if (strchr(" \t\r", buff[i]))
    {
      if (!comment)
      {
        buff[i] = 0;
        if (tokens[*token_count][0] != 0)
          (*token_count)++;
        tokens[*token_count] = buff + i;
      }
    }
    else if (strchr("\n", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = "\n";
      (*token_count)++;
      tokens[*token_count] = buff + i;
      comment = false;
    }
    else if (strchr("#", buff[i]))
    {
      buff[i] = 0;
      if (tokens[*token_count][0] != 0)
        (*token_count)++;
      tokens[*token_count] = "#";
      (*token_count)++;
      if (i + 1 < buff_len)
        tokens[*token_count] = buff + i + 1;
      comment = true;
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

  if (tokens[*token_count][0] != 0)
    (*token_count)++;
  
  ///* Dont remove!
  // Dump a token list for debugging
  for (int i = 0; i < *token_count; i++)
  {
    if (tokens[i][0] == '\n')
      printf("[\\n]\n");
    else
      printf("[%s] ", tokens[i]);
  }
  printf("\n");
  //*/

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
    // Add a dummy item and increment the line counter
    if (tokens[i][0] == '\n')
    {
      AddDummyItem("\n");
      linecount++;
      i++;
    }

    // Comments
    // Skip comments by consuming all takens up to but not including
    // the next end-of-line.
    else if (tokens[i][0] == '#')
    {
      AddDummyItem(tokens[i++]);
      if (i < token_count)
        AddDummyItem(tokens[i++]);
    }

    // Open block tokens
    // Make the current section the parent.
    else if (tokens[i][0] == '{')
    {
      AddDummyItem("{");
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
        AddDummyItem("}");
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
      
      AddItem(0, name, 0, value);
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
      
      AddItem(section, name, 0, value);
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
        AddItem(section, name, 0, value);
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
          AddItem(section, name, index++, value); 
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

  // Debugging
  DumpItems();
  
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
int CWorldFile::AddItem(int section, const char *name,
                        int index, const char *value)
{
  // See of the item is in the list
  // (ignore the index).
  int item = GetItem(section, name, -1);

  // If there is no such item in the list,
  // add the item now.
  // Make sure we insert items in the correct section.
  if (item < 0)
  {
    assert(this->item_count < this->item_size);

    // Find the correct section
    for (item = 0; item < this->item_count; item++)
    {
      CItem *pitem = this->items + item;
      if (pitem->section > section)
        break;
    }

    // Move stuff up to make room for the new item
    if (item < this->item_count)
      memmove(this->items + item + 1, this->items + item,
              (this->item_count - item) * sizeof(this->items[0]));

    CItem *pitem = this->items + item;
    pitem->section = section;
    pitem->name = strdup(name);
    pitem->value_count = 0;
    pitem->value_size = sizeof(pitem->values) / sizeof(pitem->values[0]);

    this->item_count++;
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
// Add a dummy item
int CWorldFile::AddDummyItem(const char *value)
{
  assert(this->item_count < this->item_size);
  int item = this->item_count++;
  CItem *pitem = this->items + item;
  pitem->section = -1;
  pitem->name = "";
  pitem->value_count = 1;
  pitem->value_size = sizeof(pitem->values) / sizeof(pitem->values[0]);
  pitem->values[0] = strdup(value);
  pitem->save = true;
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
    {
      if (index < 0)
        return item;
      if (index < pitem->value_count && pitem->values[index] != NULL)
        return item;
    }
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Dump the item list for debugging
void CWorldFile::DumpItems()
{
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;

    printf("%d %s [", pitem->section, pitem->name);
    for (int index = 0; index < pitem->value_count; index++)
    {
      if (index > 0)
        printf(" ");
      if (pitem->values[index][0] == '\n')
        printf("\\n");
      else
        printf("%s", pitem->values[index]);
    }
    printf("]\n");
  }
}

///////////////////////////////////////////////////////////////////////////
// Read a string
const char *CWorldFile::ReadString(int section, const char *name, const char *value)
{
  int item = GetItem(section, name, 0);
  if (item < 0)
    return value;
  return this->items[item].values[0];
}


///////////////////////////////////////////////////////////////////////////
// Read an int
int CWorldFile::ReadInt(int section, const char *name, int value)
{
  int item = GetItem(section, name, 0);
  if (item < 0)
    return value;
  return atoi(this->items[item].values[0]);
}


///////////////////////////////////////////////////////////////////////////
// Read a float
double CWorldFile::ReadFloat(int section, const char *name, double value)
{
  int item = GetItem(section, name, 0);
  if (item < 0)
    return value;
  return atof(this->items[item].values[0]);
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
  int item = AddItem(section, name, index, value);
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
