///////////////////////////////////////////////////////////////////////////
//
// File: worldfile.hh
// Author: Andrew Howard
// Date: 15 Nov 2001
// Desc: Class for handling world files
//
// $Id: worldfile.hh,v 1.1 2001-11-17 00:24:12 ahoward Exp $
//
///////////////////////////////////////////////////////////////////////////

#ifndef WORLDFILE_HH
#define WORLDFILE_HH


/* Class for loading/saving world file
   This class encapsulates hides the syntax of the world file
   and provides an 'section.item = value' style interface.
   Global settings go in section 0; every other section
   refers to a specific object.  Parent/child relationships
   are encapsulated in the form of section/subsection.
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

  // Get the number of sections.
  public: int GetSectionCount();

  // Get a section's parent section.
  // Returns -1 if there is no parent.
  public: int GetParentSection(int section);

  // Read a string
  public: char *ReadString(int section, const char *name, const char *value);

  // Write a string
  public: void WriteString(int section, const char *name, const char *value);

  // Read an integer 
  public: int ReadInt(int section, const char *name, int defaultv);

  // Write an integer
  public: void WriteInt(int section, const char *name, int value);

  // Read a float 
  public: double ReadFloat(int section, const char *name, double value);

  // Write a float
  public: void WriteFloat(int section, const char *name, double value);

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

  // Tokenize a string
  private: bool Tokenize(char *buff, int buff_len,
                         int *token_len, int token_size, char **tokens);

  // Parse a line
  private: bool Parse(int token_count, char **tokens);

  // Add a section
  private: int AddSection(int parent);

  // Add an item
  private: int AddItem(int section, const char *name, int index,
                       const char *value, bool save);

  // Get an item
  private: int GetItem(int section, const char *name, int index);

  // Private section class
  private: struct CSection
  {
    int parent;
  };

  // Private item class
  private: struct CItem
  {
    int section;
    char *name;
    int value_size;
    int value_count;
    char *values[4];
    bool save;
  };
  
  // Name of the file we loaded
  private: char *filename;

  // Section list
  private: int section_size;
  private: int section_count;
  private: CSection sections[1000];

  // Item list
  private: int item_size;
  private: int item_count;
  private: CItem items[4000];
};

#endif
