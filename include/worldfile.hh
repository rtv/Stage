///////////////////////////////////////////////////////////////////////////
//
// File: worldfile.hh
// Author: Andrew Howard
// Date: 15 Nov 2001
// Desc: Class for handling world files
//
// $Id: worldfile.hh,v 1.5 2002-01-16 20:22:07 gerkey Exp $
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

  // Read an angle (includes unit conversion)
  public: double ReadAngle(int section, const char *name, double value);

  // Read a boolean
  public: bool ReadBool(int section, const char *name, bool value);
  
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
  
  // Tokenize a string
  private: bool Tokenize(char *buff, int buff_len,
                         int *token_len, int token_size, char **tokens);

  // Parse a line
  private: bool Parse(int token_count, char **tokens);

  // Add a section
  private: int AddSection(int parent, int indent);

  // Add an item
  private: int AddItem(int line, int section, const char *name, bool property);

  // Insert an item
  private: int InsertItem(int section, const char *name);

  // Get an item
  private: int GetItem(int section, const char *name);

  // Set the value of an item
  private: void SetItemValue(int item, int index, const char *value);

  // Get the value of an item
  private: const char *GetItemValue(int item, int index);

  // Dump the item list for debugging
  private: void DumpItems();
  
  // Private section class
  private: struct CSection
  {
    int parent;
    int indent;
  };

  // Private item class
  private: struct CItem
  {
    int line;
    int section;
    bool property;
    bool used;
    char *name;
    int value_size;
    int value_count;
    char **values;
  };
  
  // Name of the file we loaded
  private: char *filename;
  // Name of the file we loaded
public: char *Filename( void ) { return( filename ); };

  // Section list
  private: int section_size;
  private: int section_count;
  private: CSection sections[1000];

  // Item list
  private: int item_size;
  private: int item_count;
  private: CItem items[4000];

  // Conversion units
  private: double unit_length;
  private: double unit_angle;
};

#endif
