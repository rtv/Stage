
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>

#include <glib.h>

#define DEBUG

#include "replace.h"
#include "stage.h"

int _stg_quit = FALSE;

// -----------------------------------------------------------------------

// returns a human readable desciption of the property type [id]
const char* stg_property_string( stg_id_t id )
{
  switch( id )
    {
      //case STG_PROP_CHILDREN: return "children"; break;
      //case STG_PROP_STATUS: return "status"; break;
    case STG_PROP_TIME: return "time"; break;
    case STG_PROP_CIRCLES: return "circles"; break;
    case STG_PROP_COLOR: return "color"; break;
    case STG_PROP_GEOM: return "geom"; break;
    case STG_PROP_LASERRETURN: return "laser_return"; break;
    case STG_PROP_NAME: return "name"; break;
    case STG_PROP_OBSTACLERETURN: return "obstacle_return";break;
    case STG_PROP_ORIGIN: return "origin"; break;
    case STG_PROP_PARENT: return "parent"; break;
    case STG_PROP_PLAYERID: return "player_id"; break;
    case STG_PROP_POSE: return "pose"; break;
    case STG_PROP_POWER: return "sonar_power"; break;
    case STG_PROP_PPM: return "ppm"; break;
    case STG_PROP_PUCKRETURN: return "puck_return"; break;
    case STG_PROP_RANGEBOUNDS: return "rangebounds";break;
    case STG_PROP_RECTS: return "rects"; break;
    case STG_PROP_SIZE: return "size"; break;
    case STG_PROP_SONARRETURN: return "sonar_return"; break;
    case STG_PROP_NEIGHBORRETURN: return "neighbor_return"; break;
    case STG_PROP_NEIGHBORBOUNDS: return "neighbor_bounds"; break;
    case STG_PROP_NEIGHBORS: return "neighbors"; break;
    case STG_PROP_VELOCITY: return "velocity"; break;
    case STG_PROP_VISIONRETURN: return "vision_return"; break;
    case STG_PROP_VOLTAGE: return "voltage"; break;
    case STG_PROP_RANGERDATA: return "rangerdata";break;
    case STG_PROP_RANGERCONFIG: return "rangerconfig";break;
    case STG_PROP_LASERDATA: return "laserdata";break;
    case STG_PROP_LASERCONFIG: return "laserconfig";break;
    case STG_PROP_BLINKENLIGHT: return "blinkenlight";break;
    case STG_PROP_NOSE: return "nose";break;
    case STG_PROP_BOUNDARY: return "boundary";break;
    case STG_PROP_LOSMSG: return "los_msg";break;
    case STG_PROP_LOSMSGCONSUME: return "los_msg_consume";break;
    case STG_PROP_MOVEMASK: return "movemask";break;
    case STG_PROP_MATRIXRENDER: return "matrix_render";break;
    case STG_PROP_INTERVAL: return "interval";break;

    default:
      break;
    }
  return "<unknown>";
}


void stg_err( char* err )
{
  printf( "Stage error: %s\n", err );
  _stg_quit = TRUE;
}


// compose a human readable string desrcribing property [prop]
char* stg_property_sprint( char* str, stg_property_t* prop )
{
  sprintf( str, 
	   " %d(%s) at %p len %d\n",
	   prop->id, 
	   stg_property_string(prop->id), 
	   prop->data,
	   (int)prop->len );
  
  return str;
}


// print a human-readable property on [fp], an open file pointer
// (eg. stdout, stderr).
void stg_property_fprint( FILE* fp, stg_property_t* prop )
{
  char str[1000];
  stg_property_sprint( str, prop );
  fprintf( fp, "%s", str );
}

// print a human-readable property on stdout
void stg_property_print( stg_property_t* prop )
{
  stg_property_fprint( stdout, prop );
}

void stg_property_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_property_print( (stg_property_t*)value );
}




// write a message out	  
int stg_fd_msg_write( int fd, stg_msg_t* msg )
{
  return( stg_fd_packet_write( fd, msg, sizeof(stg_msg_t) + msg->payload_len ) );
}


// operations on file descriptors ----------------------------------------

// internal low-level functions. user code should probably use one of
// the interafce functions instead.

// attempt to read a packet from [fd]. returns the number of bytes
// read, or -1 on error (and errno is set as per read(2)).
ssize_t stg_fd_packet_read( int fd, void* buf, size_t len )
{
  //PRINT_DEBUG1( "attempting to read a packet of max %d bytes", len );
  
  assert( fd > 0 );
  assert( buf ); // data destination must be good
  
  size_t recv = 0;
  while( recv < len )
    {
      //PRINT_DEBUG3( "reading %d bytes from fd %d into %p", 
      //	    len - recv, fd,  ((char*)buf)+recv );

      /* read will block until it has some bytes to return */
      size_t r = 0;
      if( (r = read( fd, ((char*)buf)+recv,  len - recv )) < 0 )
	{
	  if( errno != EINTR )
	    {
	      PRINT_ERR1( "ReadPacket: read returned %d", (int)r );
	      perror( "system reports" );
	      return -1;
	    }
	}
      else if( r == 0 ) // EOF
	break; 
      else
	recv += r;
    }      
  
  //PRINT_DEBUG2( "read %d/%d bytes", recv, len );
  
  return recv; // the number of bytes read
}
// attempt to write a packet on [fd]. returns the number of bytes
// written, or -1 on error (and errno is set as per write(2)).
ssize_t stg_fd_packet_write( int fd, void* data, size_t len )
{
  size_t writecnt = 0;
  ssize_t thiswritecnt;
  
  //PRINT_DEBUG3( "writing packet on fd %d - %p %d bytes", 
  //	fd, data, len );
  
  
  while(writecnt < len )
  {
    thiswritecnt = write( fd, ((char*)data)+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }
  
  //PRINT_DEBUG2( "wrote %d/%d packet bytes", writecnt, len );
  
  return len; //success
}


// generate a hash value for an arbitrary buffer.
// I just add all the bytes together. overflow doesn't matter. 
int stg_checksum( void* buf, size_t len )
{
  int c=0;
  int total=0;
  
  for( c=0; c<len; c++ )
    total += (int)*((char*)buf)+len;
  
  return total;
}

stg_msg_t* stg_read_msg( int fd )
{
  ssize_t res = 0;
  
  stg_msg_t* msg = calloc( sizeof(stg_msg_t), 1 );
  
  // read a header
  res = stg_fd_packet_read( fd, msg, sizeof(stg_msg_t) );
  
  if( res < sizeof(stg_msg_t) )
    {
      PRINT_DEBUG1( "failed to read packet header (%d)", res );
      return NULL;
    }
  
  // add some space and read the rest of the message
  msg = realloc( msg, sizeof(stg_msg_t) + msg->payload_len );
  assert(msg);
  
  res = stg_fd_packet_read( fd, msg->payload, msg->payload_len );
  
  if( res < msg->payload_len )
    {
      PRINT_DEBUG2( "failed to read message payload (%d/%d bytes)", 
		    res, (int)msg->payload_len );
      return NULL;
    }
  
  return msg;
}

// return true if two properties have the same identifiers, else false
gboolean stg_equal( gconstpointer gp1, gconstpointer gp2 )
{
  assert( gp1 );
  assert( gp2 );

  stg_target_t* p1 = (stg_target_t*)gp1;
  stg_target_t* p2 = (stg_target_t*)gp2;
  
  if( p1->prop == p2->prop &&      
      p1->model == p2->model &&
      p1->world == p2->world )
    return TRUE;
  
  return FALSE;
}







stg_time_t stg_timenow( void )
{
  struct timeval tv;
  gettimeofday( &tv, NULL );
  return( (stg_time_t)((double)tv.tv_sec + ((double)tv.tv_usec / 1e6 )) );
}


void stg_target_error( stg_target_t* tgt, char* errstr )
{
  PRINT_ERR5( "%s %d:%d:%d(%s)",
	      errstr,
	       tgt->world,
	       tgt->model,
	       tgt->prop,
	       stg_property_string(tgt->prop) );
}

void stg_target_debug( stg_target_t* tgt, char* errstr )
{
  PRINT_DEBUG5( "%s %d:%d:%d(%s)",
	      errstr,
	       tgt->world,
	       tgt->model,
	       tgt->prop,
	       stg_property_string(tgt->prop) );
}

void stg_target_warn( stg_target_t* tgt, char* errstr )
{
  PRINT_WARN5( "%s %d:%d:%d(%s)",
	       errstr,
	       tgt->world,
	       tgt->model,
	       tgt->prop,
	       stg_property_string(tgt->prop) );
}

void stg_target_message( stg_target_t* tgt, char* errstr )
{
  PRINT_MSG5( "%s %d:%d:%d(%s)",
	      errstr,
	      tgt->world,
	      tgt->model,
	      tgt->prop,
	      stg_property_string(tgt->prop) );
}


stg_property_t* prop_create( stg_id_t id, void* data, size_t len )
{
  stg_property_t* prop = calloc( sizeof(stg_property_t), 1 );
  
  prop->id = id;
  prop->data = malloc(len);
  memcpy( prop->data, data, len );
  prop->len = len;
  
#ifdef DEBUG
  printf( "debug: created a prop: " );
  stg_property_print( prop );
#endif

  return prop;
}


void prop_destroy( stg_property_t* prop )
{
  free( prop->data );
  free( prop );
}

// create a new message of [type] containing [datalen] bytes of [data]
stg_msg_t* stg_msg_create( stg_msg_type_t type, void* data, size_t datalen )
{
  size_t msglen = sizeof(stg_msg_t) + datalen;
  stg_msg_t* msg = calloc( msglen,1  );
  
  msg->type = type;
  msg->payload_len = datalen;
  memcpy( msg->payload, data, datalen );
 
  return msg;
}
  
void stg_msg_destroy( stg_msg_t* msg )
{
  free( msg );
}

// append [data] to [msg]
stg_msg_t* stg_msg_append( stg_msg_t* msg, void* data, size_t len )
{
  // grow the header to fit the new data
  msg = realloc( msg, sizeof(stg_msg_t) + msg->payload_len + len );
  // copy the data to the end of the header
  memcpy( msg->payload + msg->payload_len, data, len );
  // store the new data length
  msg->payload_len += len;

  return msg;
}



// if stage wants to quit, this will return non-zero
int stg_quit_test( void )
{
  return _stg_quit;
}

void stg_quit_request( void )
{
  _stg_quit = 1;
}

void stg_quit_cancel( void )
{
  _stg_quit = 0;
}





// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
stg_color_t stg_lookup_color(const char *name)
{
  FILE *file;
  const char *filename;
  
  if( name == NULL ) // no string?
    return 0; // black
  
  if( strcmp( name, "" ) == 0 ) // empty string?
    return 0; // black

  filename = COLOR_DATABASE;
  file = fopen(filename, "r");
  if (!file)
  {
    PRINT_ERR2("unable to open color database %s : %s",
               filename, strerror(errno));
    fclose(file);
    return 0xFFFFFF;
  }
  
  while (TRUE)
  {
    char line[1024];
    if (!fgets(line, sizeof(line), file))
      break;

    // it's a macro or comment line - ignore the line
    if (line[0] == '!' || line[0] == '#' || line[0] == '%') 
      continue;

    // Trim the trailing space
    while (strchr(" \t\n", line[strlen(line)-1]))
      line[strlen(line)-1] = 0;

    // Read the color
    int r, g, b;
    int chars_matched = 0;
    sscanf( line, "%d %d %d %n", &r, &g, &b, &chars_matched );
      
    // Read the name
    char* nname = line + chars_matched;

    // If the name matches
    if (strcmp(nname, name) == 0)
    {
      fclose(file);
      return ((r << 16) | (g << 8) | b);
    }
  }
  PRINT_WARN1("unable to find color [%s]; using default (red)", name);
  fclose(file);
  return 0xFF0000;
}

//////////////////////////////////////////////////////////////////////////
// scale an array of rectangles so they fit in a unit square
void stg_normalize_rects( stg_rotrect_t* rects, int num )
{
  // assuming the rectangles fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  int r;
  for( r=0; r<num; r++ )
    {
      // test the origin of the rect
      if( rects[r].x < minx ) minx = rects[r].x;
      if( rects[r].y < miny ) miny = rects[r].y;      
      if( rects[r].x > maxx ) maxx = rects[r].x;      
      if( rects[r].y > maxy ) maxy = rects[r].y;

      // test the extremes of the rect
      if( (rects[r].x+rects[r].w)  < minx ) 
	minx = (rects[r].x+rects[r].w);
      
      if( (rects[r].y+rects[r].h)  < miny ) 
	miny = (rects[r].y+rects[r].h);
      
      if( (rects[r].x+rects[r].w)  > maxx ) 
	maxx = (rects[r].x+rects[r].w);
      
      if( (rects[r].y+rects[r].h)  > maxy ) 
	maxy = (rects[r].y+rects[r].h);
    }
  
  // now normalize all lengths so that the rects all fit inside
  // rectangle from 0,0 to 1,1
  double scalex = maxx - minx;
  double scaley = maxy - miny;

  for( r=0; r<num; r++ )
    { 
      rects[r].x = (rects[r].x - minx) / scalex;
      rects[r].y = (rects[r].y - miny) / scaley;
      rects[r].w = rects[r].w / scalex;
      rects[r].h = rects[r].h / scaley;
    }
}
