///////////////////////////////////////////////////////////////////////////
//
// File: world_load.cc
// Author: Andrew Howard
// Date: 14 May 2001
// Desc: Load/save routines for the world can be found here
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world_load.cc,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.4 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#include "stage_types.hh"
#include "world.hh"
#include "entityfactory.hh"


///////////////////////////////////////////////////////////////////////////
// Tokenize a string (naked utility function)
//
static int Tokenize(char *buffer, int bufflen, char **argv, int maxargs)
{
    // Parse the line into tokens
    // Tokens are delimited by spaces
    //
    int argc = 0;
    char *token = buffer;
    while (true)
    {
        assert(argc < maxargs);
        argv[argc] = strsep(&token, " \t\n");
        if (token == NULL)
            break;
        if (argv[argc][0] != 0)
            argc++;
    }
    return argc;
}


///////////////////////////////////////////////////////////////////////////
// Load the world
//
bool CWorld::Load(const char *filename)
{
    PRINT_MSG1("loading world file [%s]", filename);

    // Keep this file name: we will need it again when we save.
    //
    strcpy(m_filename, filename);
    
    // Open the file for reading
    //
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("unable to open world file %s; ignoring\n", (char*) filename);
        return false;
    }

    int linecount = 0;
    CEntity *parent_object = NULL;
    
    while (true)
    {
        // Get a line
        //
        char line[1024];
        if (fgets(line, sizeof(line), file) == NULL)
            break;
        linecount++;

        // Make a copy of the line an tokenize it
        //
        char tokens[1024];
        strcpy(tokens, line);
        char *argv[32];
        int argc = Tokenize(tokens, sizeof(tokens), argv, ARRAYSIZE(argv));

        // Skip blank lines
        //
        if (argc == 0)
            continue;
        
        // Ignore comment lines
        //
        if (argv[0][0] == '#')
            continue;

        // Look for open block tokens
        //
        if (strcmp(argv[0], "{") == 0)
        {
            if (m_object_count > 0)
                parent_object = m_object[m_object_count - 1]->m_default_object;
            else
                printf("line %d : misplaced '{'\n", (int) linecount);
        }

        // Look for close block tokens
        //
        else if (strcmp(argv[0], "}") == 0)
        {
            if (parent_object != NULL)
                parent_object = parent_object->m_parent_object;
            else
                printf("line %d : extra '}'\n", (int) linecount);
        }

        // Parse "set" command
        // set <variable> = <value>
        //
        else if (argc == 4 && strcmp(argv[0], "set") == 0 && strcmp(argv[2], "=") == 0)
        {
            if (strcmp(argv[1], "environment_file") == 0)
                strcpy(m_env_file, argv[3]);
            else if (strcmp(argv[1], "pixels_per_meter") == 0)
                ppm = atof(argv[3]);
            else
                printf("line %d : variable %s is not defined\n",
                       (int) linecount, (char*) argv[1]);  
        }

        // Parse "create" command
        // create <type> ...
        //
        else if (argc >= 2 && strcmp(argv[0], "create") == 0)
        {
            // Create the object
            //
            CEntity *object = ::CreateObject(argv[1], this, parent_object);
            if (object != NULL)
            {
                // Set some properties we will need later
                // We need the line number so we can to work out how to save this object.
                //
                object->m_line = linecount;
                object->m_column = strspn(line, " \t");
                strcpy(object->m_type, argv[1]);
                
                // Let the object load itself
                //
                if (!object->Load(argc, argv))
                {
                    fclose(file);
                    return false;
                }

                // Add to list of objects
                //
                AddObject(object);
            }
            else
                printf("line %d : object type %s is not defined\n",
                       (int) linecount, (char*) argv[1]);
        }
    }
    
    fclose(file);
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Save objects to a file
//
bool CWorld::Save(const char *filename)
{
    PRINT_MSG1("saving world file [%s]", filename);

    // Generate a temporary file name
    //
    char tempname[64];
    strcpy(tempname, m_filename);
    strcat(tempname, ".tmp");
    
    // Open the original file for reading
    //
    FILE *file = fopen(m_filename, "r");
    if (file == NULL)
    {
        printf("unable to find file %s; aborting save\n", (char*) filename);
        return false;
    }

    // Open the temp file for writing
    //
    FILE *tempfile = fopen(tempname, "w+");
    if (tempfile == NULL)
    {
        printf("unable to create file %s; aborting save\n", (char*) tempname);
        return false;
    }

    // Do a line-by-line copy of the original file to the temp file
    //
    int linecount = 0;
    while (true)
    {
        // Get a line from the orginal file
        //
        char line[1024];
        if (fgets(line, sizeof(line), file) == NULL)
            break;
        linecount++;

        // See if this line corresponds to an object;
        // if it does, let the object modify it.
        //
        for (int i = 0; i < m_object_count; i++)
        {
            CEntity *object = m_object[i];
            if (object->m_line == linecount)
            {
                // Create a token list for this object
                // WARNING -- no bounds check on argc
                //
                int argc = 0;
                char *argv[1024];
                argv[argc++] = strdup("create");
                argv[argc++] = strdup(object->m_type);
                object->Save(argc, argv);

                // Insert leading whitespace
                //
                line[0] = 0;
                for (int j = 0; j < object->m_column; j++)
                    strcat(line, " ");
                
                // Concatenate tokens into a string
                //                
                for (int j = 0; j < argc; j++)
                {
                    strcat(line, argv[j]);
                    free(argv[j]);
                    strcat(line, " ");
                }

                // Append trailing linefeed and check for overflow
                //
                strcat(line, "\n");
                assert(strlen(line) < sizeof(line));
                
                //object->Save(line, sizeof(line));
            }
        }

        // Write the line to the temp file
        //
        fputs(line, tempfile);
    }

    fclose(file);
    fclose(tempfile);

    // Now rename the temp file to the desired value
    //
    rename(tempname, filename);
    return true;
}

