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
 * CVS info: $Id: worldfile.hh,v 1.10 2002-06-07 01:53:34 inspectorg Exp $
 */

#ifndef WORLDFILE_HH
#define WORLDFILE_HH


/* Class for loading/saving world file
 *  This class encapsulates hides the syntax of the world file
 *  and provides an 'section.item = value' style interface.
 *  Global settings go in section 0; every other section
 *  refers to a specific object.  Parent/child relationships
 *  are encapsulated in the form of section/subsection.
 */
class CWorldFile
{
  // Standard constructors/destructors
  public: CWorldFile();
  public: ~CWorldFile();

  // Load world from file
  public: bool Load(const char *filename);

  // Save world back into file
  // Set filename to NULL to save back into the original file
  public: bool Save(const char *filename);

  // Check for unused properties and print warnings
  public: bool WarnUnused();

  // Get the number of sections.
  public: int GetSectionCount();

  // Get a section (returns the section type value)
  public: const char *GetSectionType(int section);

  // Lookup a section number by type name
  // Returns -1 if there is section with this type
  public: int LookupSection(const char *type);
  
  // Get a section's parent section.
  // Returns -1 if there is no parent.
  public: int GetSectionParent(int section);

  // Read a string
  public: const char *ReadString(int section, const char *name, const char *value);

  // Write a string
  public: void WriteString(int section, const char *name, const char *value);

  // Read an integer 
  public: int ReadInt(int section, const char *name, int value);

  // Write an integer
  public: void WriteInt(int section, const char *name, int value);

  // Read a float 
  public: double ReadFloat(int section, const char *name, double value);

  // Write a float
  public: void WriteFloat(int section, const char *name, double value);

  // Read a length (includes unit conversion)
  public: double ReadLength(int section, const char *name, double value);

  // Write a length (includes units conversion)
  public: void WriteLength(int section, const char *name, double value);
  
  // Read an angle (includes unit conversion)
  public: double ReadAngle(int section, const char *name, double value);

  // Read a boolean
  public: bool ReadBool(int section, const char *name, bool value);

  // Read a file name.  Always returns an absolute path.  If the
  // filename is entered as a relative path, we prepend the world
  // files path to it.
  public: const char *ReadFilename(int section, const char *name, const char *value);
  
  // Read a string from a tuple
  public: const char *ReadTupleString(int section, const char *name,
                                      int index, const char *value);
  
  // Write a string to a tuple
  public: void WriteTupleString(int section, const char *name,
                                int index, const char *value);
  
  // Read a float from a tuple
  public: double ReadTupleFloat(int section, const char *name,
                                int index, double value);

  // Write a float to a tuple
  public: void WriteTupleFloat(int section, const char *name,
                               int index, double value);

  // Read a length from a tuple (includes units conversion)
  public: double ReadTupleLength(int section, const char *name,
                                 int index, double value);

  // Write a to a tuple length (includes units conversion)
  public: void WriteTupleLength(int section, const char *name,
                                int index, double value);

  // Read an angle form a tuple (includes units conversion)
  public: double ReadTupleAngle(int section, const char *name,
                                int index, double value);

  // Write an angle to a tuple (includes units conversion)
  public: void WriteTupleAngle(int section, const char *name,
                               int index, double value);

  
  // Load tokens from a file.
  private: bool LoadTokens(FILE *file);

  // Read in a comment token
  private: bool LoadTokenComment(FILE *file, int *line);

  // Read in a word token
  private: bool LoadTokenWord(FILE *file, int *line);

  // Read in a number token
  private: bool LoadTokenNum(FILE *file, int *line);

  // Read in a string token
  private: bool LoadTokenString(FILE *file, int *line);

  // Read in a whitespace token
  private: bool LoadTokenSpace(FILE *file, int *line);

  // Save tokens to a file.
  private: bool SaveTokens(FILE *file);

  // Clear the token list
  private: void ClearTokens();

  // Add a token to the token list
  private: bool AddToken(int type, const char *value);

  // Set a token in the token list
  private: bool SetTokenValue(int index, const char *value);

  // Get the value of a token
  private: const char *GetTokenValue(int index);

  // Dump the token list (for debugging).
  private: void DumpTokens();

  // Parse a line
  private: bool ParseTokens();

  // Parse an word (could be a section or an item) from the token list.
  private: bool ParseTokenWord(int section, int *index, int *line);

  // Parse a section from the token list.
  private: bool ParseTokenSection(int section, int *index, int *line);

  // Parse an item from the token list.
  private: bool ParseTokenItem(int section, int *index, int *line);

  // Parse a tuple.
  private: bool ParseTokenTuple(int section, int item, int *index, int *line);

  // Clear the section list
  private: void ClearSections();

  // Add a section
  private: int AddSection(int parent, const char *name);

  // Dump the section list for debugging
  private: void DumpSections();

  // Clear the item list
  private: void ClearItems();

  // Add an item
  private: int AddItem(int section, const char *name, int line);

  // Add an item value.
  private: void AddItemValue(int item, int index, int value_token);
  
  // Get an item
  private: int GetItem(int section, const char *name);

  // Set the value of an item.
  private: void SetItemValue(int item, int index, const char *value);

  // Get the value of an item.
  private: const char *GetItemValue(int item, int index);

  // Dump the item list for debugging
  private: void DumpItems();

  // Token types.
  private: enum
  {
    TokenComment,
    TokenWord, TokenNum, TokenString,
    TokenOpenSection, TokenCloseSection,
    TokenOpenTuple, TokenCloseTuple,
    TokenSpace, TokenEOL
  };

  // Token structure.
  private: struct CToken
  {
    // Token type (enumerated value).
    int type;

    // Token value
    char *value;
  };

  // A list of tokens loaded from the file.
  // Modified values are written back into the token list.
  private: int token_size, token_count;
  private: CToken *tokens;

  // Private section class
  private: struct CSection
  {
    // Parent section
    int parent;

    // Name of section
    const char *name;
  };

  // Section list
  private: int section_size;
  private: int section_count;
  private: CSection *sections;

  // Private item class
  private: struct CItem
  {
    // Index of section this item belongs to
    int section;

    // Name of item
    const char *name;
    
    // A list of token indexes
    int value_count;
    int *values;

    // Line this item came from
    int line;

    // Flag set if item has been used
    bool used;
  };
  
  // Item list
  private: int item_size;
  private: int item_count;
  private: CItem *items;

  // Name of the file we loaded
  private: char *filename;

  // Conversion units
  private: double unit_length;
  private: double unit_angle;
};

#endif
