///////////////////////////////////////////////////////////////////////////
//
// File: worldfile.cc
// Author: Andrew Howard
// Date: 15 Nov 2001
// Desc: A property handling class
//
// $Id: worldfile.cc,v 1.6 2002-01-29 03:25:29 inspectorg Exp $
//
///////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <libgen.h> // for dirname(3)
#include <limits.h> // for PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG

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
  
  // Create list to store item info
  this->item_count = 0;
  this->item_size = sizeof(this->items) / sizeof(this->items[0]);
  memset(this->items, 0, sizeof(this->items));

  // Set defaults units
  this->unit_length = 1.0;
  this->unit_angle = M_PI / 180;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CWorldFile::~CWorldFile()
{
  // Delete values stored in the item list
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    if (pitem->name)
      free(pitem->name);
    if (pitem->values)
      delete pitem->values;
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

  // if the extension is 'm4', then run it through m4 first
  //printf("checking %s for .m4 extension\n", this->filename);
  if(strlen(this->filename) > 3 && 
     !strncmp(this->filename+(strlen(this->filename)-3),".m4",3))
  {
    //puts("it is m4");
    char* newfilename = strdup(this->filename);
    assert(newfilename);
    newfilename[strlen(newfilename)-3] = '\0';

    char* execstr = 
      (char*)(malloc(strlen(this->filename)+strlen(newfilename)+8));
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

  // Create a buffer to store the file
  int buff_len = 0;
  int buff_size = 0x1000000;
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
    PRINT_ERR2("world file %s is too large (> %dKB)",
               this->filename, buff_size/1024);
    fclose(file);
    return false;
  }
  buff_len = ftell(file);
  fclose(file);

  // Create a list to put the tokens into
  int token_count = 0;
  int token_size = 65536;
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

  // Debugging
  //DumpItems();
  
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
  //DumpItems();
  
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

  // Write items to file
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    CSection *psection = this->sections + pitem->section;

    // Comments
    if (strcmp(pitem->name, "#") == 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "%*s", psection->indent * 2, "");
      fprintf(file, "#%s", pitem->values[0]);
    }

    // EOL
    else if (strcmp(pitem->name, "\n") == 0)
    {
      // Do nothing
    }

    // Begin
    else if (strcmp(pitem->name, "begin") == 0)
    {
      assert(pitem->value_count == 1);
      fprintf(file, "%*s", (psection->indent - 1) * 2, "");
      fprintf(file, "%s ", pitem->name);
      fprintf(file, "%s", pitem->values[0]);
    }

    // End
    else if (strcmp(pitem->name, "end") == 0)
    {
      assert(pitem->value_count == 0);
      fprintf(file, "%*s", (psection->indent - 1) * 2, "");
      fprintf(file, "%s ", pitem->name);
    }
    
    // Add key-value-pairs
    else
    {
      fprintf(file, "%*s", psection->indent * 2, "");
      fprintf(file, "%s ", pitem->name);

      if (pitem->value_count == 1)
        fprintf(file, "%s ", pitem->values[0]);
      else if (pitem->value_count > 1)
      {
        fprintf(file, "(");
        for (int i = 0; i < pitem->value_count; i++)
        {
          if (i > 0)
            fprintf(file, " ");
          fprintf(file, "%s", pitem->values[i]);
        }
        fprintf(file, ") ");
      }
    }

    // Put in eol
    fprintf(file, "\n");
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
    if (pitem->property && !pitem->used)
    {
      unused = true;
      PRINT_WARN3("worldfile %s:%d : property [%s] is defined but not used",
                  this->filename, pitem->line, pitem->name);
    }
  }
  return unused;
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
      if (!comment)
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
    }
    else if (strchr("(", buff[i]))
    {
      if (!comment)
      {
        buff[i] = 0;
        if (tokens[*token_count][0] != 0)
          (*token_count)++;
        tokens[*token_count] = "(";
        (*token_count)++;
        tokens[*token_count] = buff + i;
      }
    }
    else if (strchr(")", buff[i]))
    {
      if (!comment)
      {
        buff[i] = 0;
        linecount++;
        if (tokens[*token_count][0] != 0)
          (*token_count)++;
        tokens[*token_count] = ")";
        (*token_count)++;
        tokens[*token_count] = buff + i;
      }
    }
    else if (tokens[*token_count][0] == 0)
      tokens[*token_count] = buff + i;
  }

  if (tokens[*token_count][0] != 0)
    (*token_count)++;
  
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
// A useful macro for dumping parser errors
#define PARSE_ERR(z) \
{\
  PRINT_ERR2("world file %s:%d : " z, this->filename, linecount);\
  return false;\
}


///////////////////////////////////////////////////////////////////////////
// Parse tokens
bool CWorldFile::Parse(int token_count, char **tokens)
{
  int linecount = 1;
  int lineitems = 0;
  int item = -1;
  int indent = 0;
  int section = AddSection(-1, indent);
    
  for (int i = 0; i < token_count;)
  {
    // End-of-line
    // We preserve blank lines,
    // and check that there is only one item on each line.
    if (tokens[i][0] == '\n')
    {
      if (lineitems == 0)
        item = AddItem(linecount, section, tokens[i], false);
      else if (lineitems > 1)
        PARSE_ERR("garbage at end of line");
      linecount++;
      lineitems = 0;
      i++;
    }

    // Comment
    else if (tokens[i][0] == '#')
    {
      lineitems++;
      item = AddItem(linecount, section, tokens[i++], false);
      if (i < token_count)
        SetItemValue(item, 0, tokens[i++]);
    }

    // Begin-block
    else if (strcmp(tokens[i], "begin") == 0)
    {
      lineitems++;
      indent++;
      section = AddSection(section, indent);
      item = AddItem(linecount, section, tokens[i++], false);
      if (i >= token_count)
        PARSE_ERR("missing type specifier");
      char *type = tokens[i++];
      SetItemValue(item, 0, type);
    }

    // End-block
    else if (strcmp(tokens[i], "end") == 0)
    {
      lineitems++;
      indent--;
      item = AddItem(linecount, section, tokens[i++], false);
      section = GetSectionParent(section);
      if (section < 0)
        PARSE_ERR("misplaced 'end'");
    }

    // Everything else is a <name> <value> pair
    else
    {
      lineitems++;
      char *name = tokens[i++];
      item = AddItem(linecount, section, name, true);

      if (i >= token_count)
        PARSE_ERR("incomplete statement");
      char *value = tokens[i++];

      // If the value not a tuple,
      // Just add a simple item.
      if (strcmp(value, "(") != 0)
      {
        if (strcmp(value, ")") == 0)
          PARSE_ERR("misplaced ')'");
        if (strcmp(value, "\n") == 0 || strcmp(value, "#") == 0)
          PARSE_ERR("incomplete statement");
        SetItemValue(item, 0, value);
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
            PARSE_ERR("nested '(' is not allowed");
          if (strcmp(value, "\n") == 0 || strcmp(value, "#") == 0)
            PARSE_ERR("incomplete statement");
          SetItemValue(item, index++, value);
        }
      }     
    }
  }
    
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Add a section
int CWorldFile::AddSection(int parent, int indent)
{
  assert(this->section_count < this->section_size);

  // Add into section list
  int section = this->section_count++;
  CSection *psection = this->sections + section;
  psection->parent = parent;
  psection->indent = indent;

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
  return ReadString(section, "begin", "");
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
// Add an item
int CWorldFile::AddItem(int line, int section, const char *name, bool property)
{
  assert(this->item_count < this->item_size);
  int item = this->item_count;

  CItem *pitem = this->items + item;
  memset(pitem, 0, sizeof(CItem));
  pitem->line = line;
  pitem->property = property;
  pitem->used = false;
  pitem->section = section;
  pitem->name = strdup(name);
  pitem->value_count = 0;
  pitem->value_size = 0;
  pitem->values = NULL;

  this->item_count++;

  return item;
}


///////////////////////////////////////////////////////////////////////////
// Insert an item
int CWorldFile::InsertItem(int section, const char *name)
{
  assert(this->item_count < this->item_size);
  
  // Find the right place in the list to add the item
  int item = -1;
  for (int i = 0; i < this->item_count; i++)
  {
    CItem *pitem = this->items + i;
    if (pitem->section == section)
      item = i;
  } 
  if (item < 0)
    item = this->item_count;
  
  // Move stuff up to make room for the new item
  if (item < this->item_count)
    memmove(this->items + item + 1, this->items + item,
            (this->item_count - item) * sizeof(this->items[0]));

  CItem *pitem = this->items + item;
  memset(pitem, 0, sizeof(CItem));
  pitem->line = -1;
  pitem->property = true;
  pitem->used = true;
  pitem->section = section;
  pitem->name = strdup(name);
  pitem->value_count = 0;
  pitem->value_size = 0;
  pitem->values = NULL;

  this->item_count++;

  return item;
}


///////////////////////////////////////////////////////////////////////////
// Get an item 
int CWorldFile::GetItem(int section, const char *name)
{
  // First see if we can find an instance of this item
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;
    if (pitem->section == section && strcmp(pitem->name, name) == 0)
        return item;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Set the value of an item
void CWorldFile::SetItemValue(int item, int index, const char *value)
{
  assert(item >= 0);
  CItem *pitem = this->items + item;

  // Expand the array if it's too small
  if (index >= pitem->value_size)
  {
    int tmpsize = pitem->value_size + 2;
    char **tmp = new char*[tmpsize];
    memset(tmp, 0, tmpsize * sizeof(pitem->values[0]));
    if (pitem->values)
    {
      memmove(tmp, pitem->values, pitem->value_count * sizeof(pitem->values[0]));
      delete[] pitem->values;
    }
    pitem->values = tmp;
    pitem->value_size = tmpsize;
  }

  // Set the relevant value
  assert(index < pitem->value_size);
  if (index >= pitem->value_count)
    pitem->value_count = index + 1;
  if (pitem->values[index])
    free(pitem->values[index]);
  pitem->values[index] = strdup(value);  
}


///////////////////////////////////////////////////////////////////////////
// Get the value of an item 
const char *CWorldFile::GetItemValue(int item, int index)
{
  assert(item >= 0);
  CItem *pitem = this->items + item;
  assert(index < pitem->value_size);
  pitem->used = true;
  return pitem->values[index];
}


///////////////////////////////////////////////////////////////////////////
// Dump the item list for debugging
void CWorldFile::DumpItems()
{
  for (int item = 0; item < this->item_count; item++)
  {
    CItem *pitem = this->items + item;

    printf("[%d][%d]", pitem->section, GetSectionParent(pitem->section));
    if (pitem->name[0] == '\n')
      printf("[\\n][");
    else
      printf("[%s][", pitem->name);

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
  int item = GetItem(section, name);
  if (item < 0)
    return value;
  return GetItemValue(item, 0);
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
  if (item < 0)
    item = InsertItem(section, name);
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





