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
 * Desc: C++ extensions to the Stage IO library, including the Worldfile
 * class for reading in the world file. Ideally this should be provided in 
 * the basic C library, but right now we only have a C++ implementation.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 15 Nov 2001
 * CVS info: $Id: stagecpp.hh,v 1.8 2003-10-22 07:04:55 rtv Exp $
 */

#ifndef STAGECPP_HH
#define STAGECPP_HH

#include <stage.h>

#define STG_AUTONAME_MAX 100

typedef struct 
{
  char token[STG_AUTONAME_MAX];
  int count;
} stg_token_counter_t;

// Class for loading/saving world file.  This class hides the syntax
// of the world file and provides an 'entity.property = value' style
// interface.  Global settings go in entity 0; every other entity
// refers to a specific entity.  Parent/child relationships are
// encoded in the form of entity/subentity relationships.
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

  // upload the current data into a Stage server.  on exit [models] is
  // an array mapping [model_count] device names to Stage
  // ids. 
public: int Upload( stg_client_t* cli );  
public: int SaveModels( stg_client_t* cli );
  
  // Check for unused properties and print warnings
  public: bool WarnUnused();

  // Read a string
  public: const char *ReadString(int entity, const char *name, const char *value);

  // Write a string
  public: void WriteString(int entity, const char *name, const char *value);

  // Read an integer 
  public: int ReadInt(int entity, const char *name, int value);

  // Write an integer
  public: void WriteInt(int entity, const char *name, int value);

  // Read a float 
  public: double ReadFloat(int entity, const char *name, double value);

  // Write a float
  public: void WriteFloat(int entity, const char *name, double value);

  // Read a length (includes unit conversion)
  public: double ReadLength(int entity, const char *name, double value);

  // Write a length (includes units conversion)
  public: void WriteLength(int entity, const char *name, double value);
  
  // Read an angle (includes unit conversion)
  public: double ReadAngle(int entity, const char *name, double value);

  // Read a boolean
  public: bool ReadBool(int entity, const char *name, bool value);

  // Read a color (includes text to RGB conversion)
  public: uint32_t ReadColor(int entity, const char *name, uint32_t value);

  // Read a file name.  Always returns an absolute path.  If the
  // filename is entered as a relative path, we prepend the world
  // files path to it.
  public: const char *ReadFilename(int entity, const char *name, const char *value);
  
  // Read a string from a tuple
  public: const char *ReadTupleString(int entity, const char *name,
                                      int index, const char *value);
  
  // Write a string to a tuple
  public: void WriteTupleString(int entity, const char *name,
                                int index, const char *value);
  
  // Read a float from a tuple
  public: double ReadTupleFloat(int entity, const char *name,
                                int index, double value);

  // Write a float to a tuple
  public: void WriteTupleFloat(int entity, const char *name,
                               int index, double value);

  // Read an int from a tuple
  public: int ReadTupleInt(int entity, const char *name,
			      int index, int value);
  
  // Write a float to a tuple
  public: void WriteTupleInt(int entity, const char *name,
                               int index, int value);


  // Read a length from a tuple (includes units conversion)
  public: double ReadTupleLength(int entity, const char *name,
                                 int index, double value);

  // Write a to a tuple length (includes units conversion)
  public: void WriteTupleLength(int entity, const char *name,
                                int index, double value);

  // Read an angle form a tuple (includes units conversion)
  public: double ReadTupleAngle(int entity, const char *name,
                                int index, double value);

  // Write an angle to a tuple (includes units conversion)
  public: void WriteTupleAngle(int entity, const char *name,
                               int index, double value);


  ////////////////////////////////////////////////////////////////////////////
  // Private methods used to load stuff from the world file
  
  // Load tokens from a file.
  private: bool LoadTokens(FILE *file, int include);

  // Read in a comment token
  private: bool LoadTokenComment(FILE *file, int *line, int include);

  // Read in a word token
  private: bool LoadTokenWord(FILE *file, int *line, int include);

  // Load an include token; this will load the include file.
  private: bool LoadTokenInclude(FILE *file, int *line, int include);

  // Read in a number token
  private: bool LoadTokenNum(FILE *file, int *line, int include);

  // Read in a string token
  private: bool LoadTokenString(FILE *file, int *line, int include);

  // Read in a whitespace token
  private: bool LoadTokenSpace(FILE *file, int *line, int include);

  // Save tokens to a file.
  private: bool SaveTokens(FILE *file);

  // Clear the token list
  private: void ClearTokens();

  // Add a token to the token list
  private: bool AddToken(int type, const char *value, int include);

  // Set a token in the token list
  private: bool SetTokenValue(int index, const char *value);

  // Get the value of a token
  private: const char *GetTokenValue(int index);

  // Dump the token list (for debugging).
  private: void DumpTokens();

  // Parse a line
  private: bool ParseTokens();

  // Parse an include statement
  private: bool ParseTokenInclude(int *index, int *line);

  // Parse a macro definition
  private: bool ParseTokenDefine(int *index, int *line);

  // Parse an word (could be a entity or an property) from the token list.
  private: bool ParseTokenWord(int entity, int *index, int *line);

  // Parse a entity from the token list.
  private: bool ParseTokenEntity(int entity, int *index, int *line);

  // Parse an property from the token list.
  private: bool ParseTokenProperty(int entity, int *index, int *line);

  // Parse a tuple.
  private: bool ParseTokenTuple(int entity, int property, int *index, int *line);

  // Clear the macro list
  private: void ClearMacros();

  // Add a macro
  private: int AddMacro(const char *macroname, const char *entityname,
                        int line, int starttoken, int endtoken);

  // Lookup a macro by name
  // Returns -1 if there is no macro with this name.
  private: int LookupMacro(const char *macroname);

  // Dump the macro list for debugging
  private: void DumpMacros();

  // Clear the entity list
  private: void ClearEntities();

  // Add a entity
  private: int AddEntity(int parent, const char *type);

    // Get the number of entities.
  public: int GetEntityCount();

  // Get a entity (returns the entity type value)
  public: const char *GetEntityType(int entity);

  // Lookup a entity number by type name
  // Returns -1 if there is entity with this type
  public: int LookupEntity(const char *type);
  
  // Get a entity's parent entity.
  // Returns -1 if there is no parent.
  public: int GetEntityParent(int entity);

  // Dump the entity list for debugging
  private: void DumpEntities();

  // Clear the property list
  private: void ClearProperties();

  // Add an property
  private: int AddProperty(int entity, const char *name, int line);

  // Add an property value.
  private: void AddPropertyValue(int property, int index, int value_token);
  
  // Get an property
  private: int GetProperty(int entity, const char *name);

  // Set the value of an property.
  private: void SetPropertyValue(int property, int index, const char *value);

  // Get the value of an property.
  private: const char *GetPropertyValue(int property, int index);

  // Dump the property list for debugging
  private: void DumpProperties();


  // Token types.
  private: enum
  {
    TokenComment,
    TokenWord, TokenNum, TokenString,
    TokenOpenEntity, TokenCloseEntity,
    TokenOpenTuple, TokenCloseTuple,
    TokenSpace, TokenEOL
  };

  // Token structure.
  private: struct CToken
  {
    // Non-zero if token is from an include file.
    int include;
    
    // Token type (enumerated value).
    int type;

    // Token value
    char *value;
  };

  // A list of tokens loaded from the file.
  // Modified values are written back into the token list.
  private: int token_size, token_count;
  private: CToken *tokens;

  // Private macro class
  private: struct CMacro
  {
    // Name of macro
    const char *macroname;

    // Name of entity
    const char *entityname;

    // Line the macro definition starts on.
    int line;
    
    // Range of tokens in the body of the macro definition.
    int starttoken, endtoken;
  };

  // Macro list
  private: int macro_size;
  private: int macro_count;
  private: CMacro *macros;
  
  // Private entity class
  private: struct CEntity
  {
    // Parent entity
    int parent;

    // Type of entity (i.e. position, laser, etc).
    const char *type;
  };

  // Entity list
  private: int entity_size;
  private: int entity_count;
  private: CEntity *entities;

  // Private property class
  private: struct CProperty
  {
    // Index of entity this property belongs to
    int entity;

    // Name of property
    const char *name;
    
    // A list of token indexes
    int value_count;
    int *values;

    // Line this property came from
    int line;

    // Flag set if property has been used
    bool used;
  };
  
  // Property list
  private: int property_size;
  private: int property_count;
  private: CProperty *properties;

  // Name of the file we loaded
  private: char *filename;

  // Conversion units
  private: double unit_length;
  private: double unit_angle;
};

#endif
