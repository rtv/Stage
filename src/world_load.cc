///////////////////////////////////////////////////////////////////////////
//
// File: world_load.cc
// Author: Andrew Howard
// Date: 14 May 2001
// Desc: Load/save routines & command line processing for the world 
//
// $Id: world_load.cc,v 1.28.2.1 2001-11-21 01:38:12 ahoward Exp $
//
///////////////////////////////////////////////////////////////////////////

#include "stage_types.hh"
#include "world.hh"
#include "entityfactory.hh"
#include "truthserver.hh"
#include <libgen.h>  // for dirname/#define DEBUG

// REMOVE extern char *world_file;

bool usage = false;
void PrintUsage(); // defined in main.cc


///////////////////////////////////////////////////////////////////////////
// Load the world
bool CWorld::Load(const char *filename)
{
  printf( "[World %s]", filename );
  fflush( stdout );

  // the default hostname is this host's name
  char current_hostname[ HOSTNAME_SIZE ];
  strcpy( current_hostname, m_hostname );
  
  // Load and parse the world file
  if (!this->worldfile.Load(filename))
    return false;

  // Read global settings (from section 0)

  // Get the number of pixels per meter
  this->ppm = this->worldfile.ReadFloat(0, "pixels_per_meter", 20.0);

  // Get the scale for fig-based environment files
  this->scale = this->worldfile.ReadFloat(0, "environment_scale", 1.0);

  // Get the name of the environment file, but defer loading until later.
  // For fig-based environment files, we need to know both the
  // ppm and the scale before loading the environment.
  const char *envfile = this->worldfile.ReadString(0, "environment_file", "");
  char base[256];
  strncpy( base, filename, sizeof(base) );
  sprintf(m_env_file, "%s/%s", dirname(base), envfile);

  // Get the authorization key to pass to player
  const char *authkey = this->worldfile.ReadString(0, "auth_key", "");
  strncpy(m_auth_key, authkey, sizeof(m_auth_key));
  m_auth_key[sizeof(m_auth_key)-1] = '\0';

  // Get the real update interval
  m_real_timestep = this->worldfile.ReadFloat(0, "real_timestep", 0.100);

  // Get the simulated update interval
  m_sim_timestep = this->worldfile.ReadFloat(0, "sim_timestep", 0.100);
  
  // Iterate through sections and create objects as needs be
  for (int section = 1; section < this->worldfile.GetSectionCount(); section++)
  {
    // Find out what type of object this is,
    // and what line it came from.
    const char *type = this->worldfile.ReadString(section, "begin", "");
    int line = this->worldfile.ReadInt(section, "line", -1);

    // Find the parent object
    CEntity *parent = NULL;
    int psection = this->worldfile.GetSectionParent(section);
    for (int i = 0; i < GetObjectCount(); i++)
    {
      CEntity *object = GetObject(i);
      if (GetObject(i)->worldfile_section == psection)
        parent = object;
    }
    
    // Create the object
    CEntity *object = CreateObject(type, parent);
    if (object != NULL)
    {      
      // Store which section it came from (so we know where to
      // save it to later).
      object->worldfile_section = section;

      // Work out whether or not its a local object
      // if the object has this host's name,
      // set the local flag to show that this computer must
      // update the object
      // NEEDS FIXING AH
      strncpy( object->m_hostname, current_hostname, HOSTNAME_SIZE );
      if(CheckHostname(object->m_hostname))
        object->m_local = true;
      else
        object->m_local = false;
      
      // Let the object load itself
      if (!object->Load(&this->worldfile, section))
        return false;

      // Add to list of objects
      AddObject(object);
    }
    else
      PRINT_ERR2("line %d : entity type %s is not defined", line, type);
  }

  // See if there was anything we didnt understand in the world file
  this->worldfile.WarnUnused();
  
  // Now load the environment file
  printf( "[%s LOADING]", m_env_file );
  fflush( stdout );
  if( !InitGrids(m_env_file) )
    PRINT_WARN1("error loading environment bitmap %s", m_env_file );
  printf( "\b\b\b\b\b\b\b\b\b" ); // erase load indicator
  printf( "         " ); // erase load indicator
  printf( "\b\b\b\b\b\b\b\b\b]" ); // erase load indicator
  fflush( stdout );
  
  return true;

  /*

  printf( "[World %s]", filename ); fflush( stdout );
   
  char m_current_hostname[ HOSTNAME_SIZE ];

  // the default hostname is this host's name
  strcpy( m_current_hostname, m_hostname );
 
    
  // TODO: if we have any objects already, kill 'em, clear grids, etc.  
    
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
    sprintf(system_command_string, "/usr/bin/m4 -E %s > %s",
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
      // Get the number of pixels per meter
      if (strcmp(argv[1], "pixels_per_meter") == 0)
        this->ppm = atof(argv[3]);
      // Get the scale of fig-based environment files
      else if (strcmp(argv[1], "environment_scale") == 0)
        this->scale = atof(argv[3]);
      // Get the name of the environment file, but
      // defer loading until later.
      // For fig-based environment files, we need to know both the
      // ppm and the scale before loading the environment.
      else if (strcmp(argv[1], "environment_file") == 0)
      {		
        char base[256];
        strncpy( base, filename, 255 );
        // prepend the base path of the world file.
        sprintf(m_env_file, "%s/%s",
                dirname(base),argv[3]);
      }
      // the authorization key; it is passed on to Player
      else if (strcmp(argv[1], "auth_key") == 0)
      {
        strncpy(m_auth_key, argv[3],sizeof(m_auth_key));
        m_auth_key[sizeof(m_auth_key)-1] = '\0';
      }
	    /////////////////////////////////////////////////////////////////
	    // STAGE TIMING VALUES - 2 different ways to set the params
      // the real update interval in milliseconds
      else if (strcmp(argv[1], "real_timestep_ms") == 0)
      {
        m_real_timestep = (int)atof(argv[3]);
      }

      // the simulated update rate in milliseconds
      else if (strcmp(argv[1], "virtual_timestep_ms") == 0)
      {
        m_sim_timestep = (int)atof(argv[3]);
      }

      // the real update frequency in Hz (alternative to the above)
      else if (strcmp(argv[1], "update_frequency") == 0)
      {
        m_real_timestep = (int)((1.0/atof(argv[3])) * 1000.0);
      }

      // the update frequency in Hz (alternative to the above)
      else if (strcmp(argv[1], "virtual_time_scaling") == 0)
      {
        m_sim_timestep = (int)(m_real_timestep * atof(argv[3]));
      }

	    //////////////////////////////////////////////////////////////
	    // DEVICE RESOLUTION PARAMETERS
      else if (strcmp(argv[1], "laser_res") == 0)
        m_laser_res = DTOR(atof(argv[3]));
      else if (strcmp(argv[1], "vision_res") == 0)
        m_vision_res = DTOR(atof(argv[3]));
      // get the unit specifier
      else if (strcmp(argv[1], "units") == 0)
      {
        // case-insensitive
        if(!strcasecmp(argv[3],"m"))
          unit_multiplier = 1.0;
        else if(!strcasecmp(argv[3],"dm"))
          unit_multiplier = 0.1;
        else if(!strcasecmp(argv[3],"cm"))
          unit_multiplier = 0.01;
        else if(!strcasecmp(argv[3],"mm"))
          unit_multiplier = 0.001;
        else
        {
          printf("\nline %d : unknown unit specifier \"%s\". "
                 "defaulting to meters.",
                 (int) linecount, (char*) argv[3]);  
          unit_multiplier = 1.0;
        }
      }
      else if (strcmp(argv[1], "angles") == 0)
      {
        // case-insensitive
        if(!strcasecmp(argv[3],"degrees"))
          angle_multiplier = M_PI/180.0;
        else if(!strcasecmp(argv[3],"radians"))
          angle_multiplier = 1.0;
        else
        {
          printf("\nline %d : unknown angle specifier \"%s\". "
                 "defaulting to degrees.",
                 (int) linecount, (char*) argv[3]);  
          angle_multiplier = M_PI/180.0;
        }
      }
	    // case-insensitive
      else if (strcasecmp(argv[1], "channel") == 0)
      {
        int channel_num;
        int offset;
        sscanf( argv[3], "%d : %n", &channel_num, &offset );

        char* color_name = argv[3] + offset;

        ColorFromString( &(channel[ channel_num ]), color_name );
        //strcpy( &(channel[ channel_num ]), color_name, strlen( color_name ) );

#ifdef DEBUG
        printf( "\nsetting channel %d to %s.", channel_num, color_name );
#endif
      }
	    // change the current hostname context
      else if (strcasecmp(argv[1], "host") == 0)
      {
        strncpy( m_current_hostname, argv[3], HOSTNAME_SIZE );
        //printf( "\nHost context: %s\n", m_current_hostname );
      }
	  }
    // Parse "enable" command
    // switch on optional sevices
    //
    else if (argc >= 2 && strcmp(argv[0], "enable") == 0)
    {
      if( strcmp( argv[1], "pose_server" ) == 0 )
		  {
		    // see if we should run on a non-standard port
		    if(argc >= 4 && !strcmp(argv[2],"port"))
		      m_pose_port = atoi(argv[3]);

		    m_run_pose_server = true;

		    printf( "[Pose %d]", m_pose_port );
		    fflush ( stdout );
		  }
      else if( strcmp( argv[1], "environment_server" ) == 0 )
		  {
		    // see if we should run on a non-standard port
		    if(argc >= 4 && !strcmp(argv[2],"port"))
		      m_env_port = atoi(argv[3]);
		    
		    m_run_environment_server = true;
		
		    printf( "[Env %d]", m_env_port );
		    fflush ( stdout );

		  }
      else if( strcmp( argv[1], "xs" ) == 0 )
		  {
		    m_run_xs = true;
		
		    printf( "[XS]" );
		    fflush ( stdout );
		  }
      else
        printf("%s line %d : service %s is not defined\n",
               filename, (int) linecount, (char*) argv[1]);
    }
        
    // Parse "disnable" command
    // disable optional sevices
    //
    else if (argc >= 2 && strcmp(argv[0], "disable") == 0)
    {
      if( strcmp( argv[1], "pose_server" ) == 0 )
		  {
		    m_run_pose_server = false;

		    printf( "[Pose disabled]" );
		    fflush ( stdout );
		  }
      else if( strcmp( argv[1], "environment_server" ) == 0 )
		  {
		    m_run_environment_server = false;
		
		    printf( "[Env disabled]" );
		    fflush ( stdout );
		  }
      else if( strcmp( argv[1], "xs" ) == 0 )
		  {
		    m_run_xs = false;
		
		    printf( "[XS disabled]" );
		    fflush ( stdout );
		  }
      else
        printf("\nStage warning: %s line %d : service %s is not defined\n",
               filename, (int) linecount, (char*) argv[1]);
    }
        

    // Parse "create" command
    // create <type> ...
    //
    else if (argc >= 2 && strcmp(argv[0], "create") == 0)
    {
      // Create the object
      //
      CEntity *object = CreateObject(argv[1], parent_object);
      if (object != NULL)
		  {
		    // Set some properties we will need later
		    // We need the line number so we can to work out how 
		    // to save this object.
		    //
		    object->m_line = linecount;
		    object->m_column = strspn(line, " \t");
		    strcpy(object->m_type, argv[1]);
                
		    strncpy( object->m_hostname, m_current_hostname, 
                 HOSTNAME_SIZE );

		    // if the object has this host's name,
		    // set the local flag to show that this computer must
		    // update the object
                    
		    if(CheckHostname(object->m_hostname))
		      object->m_local = true;
		    else
		      object->m_local = false;
		      
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
      
  // Now load the environment file
  printf( "[%s LOADING]", m_env_file );
  fflush( stdout );
  if( !InitGrids(m_env_file) )
    printf( "Stage warning: "
            "error loading environment bitmap %s\n", 
            m_env_file );
  printf( "\b\b\b\b\b\b\b\b\b" ); // erase load indicator
  printf( "         " ); // erase load indicator
  printf( "\b\b\b\b\b\b\b\b\b]" ); // erase load indicator
  fflush( stdout );
    
  fclose(file);
  return true;
  */
}


///////////////////////////////////////////////////////////////////////////
// Save objects to a file
//
bool CWorld::Save(const char *filename)
{
  // Let each object save itself
  for (int i = 0; i < GetObjectCount(); i++)
  {
    CEntity *object = GetObject(i);
    if (!object->Save(&this->worldfile, object->worldfile_section))
      return false;
  }

  // Save everything
  if (!this->worldfile.Save(filename))
    return false;

  return true;

  /*
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
  */
}


///////////////////////////////////////////////////////////////////////////
// Parse the command line
//
bool CWorld::ParseCmdline(int argc, char **argv)
{
  if( argc < 2 )
  {
    usage = true;
    exit( -1 );
  }
  
  for( int a=1; a<argc-1; a++ )
  {
    // USAGE
    if( (strcmp( argv[a], "-?" ) == 0) || 
        (strcmp( argv[a], "--help") == 0) )
    {
      PrintUsage();
      return false;
    }
    // LOGGING
    else if( strcmp( argv[a], "-l" ) == 0 )
    {
      m_log_output = true;
      strncpy( m_log_filename, argv[a+1], 255 );
      printf( "[Logfile %s]", m_log_filename );

      //store the command line for logging later
      memset( m_cmdline, 0, sizeof(m_cmdline) );
	  
      for( int g=0; g<argc; g++ )
	    {
	      strcat( m_cmdline, argv[g] );
	      strcat( m_cmdline, " " );
	    }

      a++;
	  
      // open the log file and write out a header
      LogOutputHeader();
    }
      
    // FAST MODE - run as fast as possible - don't attempt t match real time
    if( strcmp( argv[a], "-fast" ) == 0 )
    {
      m_real_timestep = 0.0;
      printf( "[Fast]" );
    }

    // DIS/ENABLE XS
    if( strcmp( argv[a], "-xs" ) == 0 )
    {
      m_run_xs = false;
      printf( "[No GUI]" );
    }
    else if( strcmp( argv[a], "+xs" ) == 0 )
    {
      m_run_xs = true;
      printf( "[GUI]" );
    }

    // SET GOAL REAL CYCLE TIME
    // Stage will attempt to update at this speed
    else if( strcmp( argv[a], "-u" ) == 0 )
    {
      m_real_timestep = atof(argv[a+1]);
      printf( "[Real time per cycle %f sec]", m_real_timestep );
      a++;
    }

    // SET SIMULATED UPDATE CYCLE
    // one cycle simulates this much time
    else if( strcmp( argv[a], "-v" ) == 0 )
    {
      m_sim_timestep = atof(argv[a+1]);
      printf( "[Simulated time per cycle %f sec]", m_sim_timestep );
      a++;
    }

    // change the pose port 
    else if( strcmp( argv[a], "-tp" ) == 0 )
    {
      m_pose_port = atoi(argv[a+1]);
      printf( "[Pose %d]", m_pose_port );
      a++;
    }

    // change the environment port
    else if( strcmp( argv[a], "-ep" ) == 0 )
    {
      m_env_port = atoi(argv[a+1]);
      printf( "[Env %d]", m_env_port );
      a++;
    }

    // SWITCH ON SYNCHRONIZED (distributed) MODE
    // if this option is given, Stage will only run when connected
    // to an external synchronous pose connection
    else if( strcmp( argv[a], "-s" ) == 0 )
    {
      m_external_sync_required = true; 
      m_enable = false; // don't run until we have a sync connection
      printf( "[External Sync]");
    }      
    else if(!strcmp(argv[a], "-time"))
    {
      m_stoptime = atoi(argv[++a]);
      printf("setting time to: %d\n",m_stoptime);
    }

    //else if( strcmp( argv[a], "-id" ) == 0 )
    //{
    //  memset( m_hostname, 0, 64 );
    //  strncpy( m_hostname, argv[a+1], 64 );
    //  printf( "[ID %s]", m_hostname ); fflush( stdout );
    //  a++;
    //}
  }
    
  return true;
}


