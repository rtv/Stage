///////////////////////////////////////////////////////////////////////////
//
// File: world_load.cc
// Author: Andrew Howard
// Date: 14 May 2001
// Desc: Load/save routines for the world can be found here
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/world_load.cc,v $
//  $Author: gerkey $
//  $Revision: 1.9 $
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
#include "truthserver.hh"

#undef DEBUG


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
#ifdef DEBUG
    PRINT_MSG1("loading world file [%s]", filename);
#endif
   
    // TODO: if we have any object's already, kill 'em, clear grids, etc.  

    FILE *file = NULL;
    
    // if the filename ends in ".m4", then run it through m4
    // and parse the result
    if(!strcmp(filename+strlen(filename)-3, ".m4"))
    {
      char bare_filename[256];
      char system_command_string[512];
      int retval;

      strncpy(bare_filename,filename,strlen(filename)-3);
      bare_filename[strlen(filename)-3] = '\0';
      sprintf(system_command_string, "m4 -E %s > %s",
              filename, bare_filename);

      PRINT_MSG1("running m4: %s",system_command_string);
      retval = system(system_command_string);
      //if(retval == 127 || retval == -1)
      if(retval)
      {
        PRINT_MSG("Error while running m4 on world file");
        return false;
      }

      if(!(file = fopen(bare_filename, "r")))
      {
        printf("unable to open world file %s; ignoring\n", 
               (char*)bare_filename);
        return false;
      }
     
      // Keep this file name: we will need it again when we save.
      //
      // 
      strcpy(m_filename, bare_filename);
    }
    else
    {
      // normal world file; just open it
      if(!(file = fopen(filename, "r")))
      {
        printf("unable to open world file %s; ignoring\n", (char*) filename);
        return false;
      }
      // Keep this file name: we will need it again when we save.
      //
      // 
      strcpy(m_filename, filename);
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
            {
              strcpy(m_env_file, argv[3]);

              printf( "[%s LOADING]", m_env_file );
              fflush( stdout );

              // Initialise the world grids
              if( !InitGrids(m_env_file) )
                printf( "Stage warning: "
                        "error loading environment bitmap %s\n", 
                        m_env_file );

              printf( "\b\b\b\b\b\b\b\b\b" ); // erase load indicator
              printf( "         " ); // erase load indicator
              printf( "\b\b\b\b\b\b\b\b\b]" ); // erase load indicator
              fflush( stdout );
            }
            else if (strcmp(argv[1], "auth_key") == 0)
            {
              strncpy(m_auth_key, argv[3],sizeof(m_auth_key));
              m_auth_key[sizeof(m_auth_key)-1] = '\0';
            }
            else if (strcmp(argv[1], "pixels_per_meter") == 0)
              ppm = atof(argv[3]);
            else if (strcmp(argv[1], "laser_res") == 0)
              m_laser_res = DTOR(atof(argv[3]));
            else
              printf("line %d : variable %s is not defined\n",
                     (int) linecount, (char*) argv[1]);  
        }

        // Parse "enable" command
        // switch on optional sevices
        //
        else if (argc >= 2 && strcmp(argv[0], "enable") == 0)
        {
	  if( strcmp( argv[1], "truth_server" ) == 0 )
	    {
	      // kick off the truth server thread
	      pthread_t tid_dummy;
	      pthread_create(&tid_dummy, NULL, &TruthServer, (void *)NULL );  

	      //printf( "[Truth %d]", TRUTH_SERVER_PORT );
	      //fflush ( stdout );

	    }
	  else if( strcmp( argv[1], "environment_server" ) == 0 )
	    {
	      // kick off the environment server thread
	      pthread_t tid_dummy;
	      pthread_create(&tid_dummy, NULL, &EnvServer, (void *)NULL );  
	    }
            else
                printf("%s line %d : service %s is not defined\n",
                       filename, (int) linecount, (char*) argv[1]);
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








