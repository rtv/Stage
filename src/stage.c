
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
#include <math.h>
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
    case STG_PROP_COLOR: return "color"; break;
    case STG_PROP_COMMAND: return "command"; break;
    case STG_PROP_CONFIG: return "config"; break;
    case STG_PROP_DATA: return "data"; break;
    case STG_PROP_ENERGYCONFIG: return "energyconfig"; break; 
    case STG_PROP_ENERGYDATA: return "energydata"; break;
    case STG_PROP_GEOM: return "geom"; break;
    case STG_PROP_GUIFEATURES: return "guifeatures"; break;
    case STG_PROP_LASERRETURN: return "laser_return"; break;
    case STG_PROP_LINES: return "lines"; break;
    case STG_PROP_MASS: return "mass";break;
    case STG_PROP_NAME: return "name"; break;
    case STG_PROP_OBSTACLERETURN: return "obstacle_return";break;
    case STG_PROP_PARENT: return "parent"; break;
    case STG_PROP_PLAYERID: return "player_id"; break;
    case STG_PROP_POSE: return "pose"; break;
    case STG_PROP_PUCKRETURN: return "puck_return"; break;
    case STG_PROP_RANGERRETURN: return "ranger_return"; break;
    case STG_PROP_TIME: return "time"; break;
    case STG_PROP_VELOCITY: return "velocity"; break;
    case STG_PROP_VISIONRETURN: return "vision_return"; break;

      //case STG_PROP_BLINKENLIGHT: return "blinkenlight";break;
      //case STG_PROP_BLOBCONFIG: return "blobconfig";break;
      //case STG_PROP_BLOBDATA: return "blobdata";break;
      //case STG_PROP_FIDUCIALCONFIG: return "fiducialconfig";break;
      //case STG_PROP_FIDUCIALDATA: return "fiducialdata";break;
      //case STG_PROP_FIDUCIALRETURN: return "fiducialreturn";break;
      //case STG_PROP_LASERCONFIG: return "laserconfig";break;
      //case STG_PROP_LASERDATA: return "laserdata";break;
      //case STG_PROP_MATRIXRENDER: return "matrix_render";break;
      //case STG_PROP_POWER: return "sonar_power"; break;
      //case STG_PROP_RANGERCONFIG: return "rangerconfig";break;
      //case STG_PROP_RANGERDATA: return "rangerdata";break;
    default:
      break;
    }
  return "<unknown>";
}


const char* stg_event_string( stg_event_t event )
{
  switch( event )
    {
    case STG_EVENT_STARTUP: return "startup";
    case STG_EVENT_SHUTDOWN: return "shutdown";
    case STG_EVENT_UPDATE: return "update";
    case STG_EVENT_SERVICE: return "service";
    case STG_EVENT_GET: return "get";
    case STG_EVENT_SET: return "set";
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


void stg_buffer_append( GByteArray* buf, void* bytes, size_t len )
{
  g_byte_array_append( buf, (guint8*)bytes, len );

  //printf( "appended buffer %p now %d bytes\n", buf, (int)buf->len );
}

void stg_buffer_prepend( GByteArray* buf, void* bytes, size_t len )
{
  g_byte_array_prepend( buf, (guint8*)bytes, len );

  //printf( "prepended buffer %p now %d bytes\n", buf, (int)buf->len );
}


void stg_buffer_append_msg( GByteArray* buf, stg_msg_t* msg )
{
  stg_buffer_append( buf,
		     (guint8*)msg, 
		     sizeof(stg_msg_t) + msg->payload_len );
}

void stg_buffer_clear( GByteArray* buf )
{
  g_byte_array_set_size( buf, 0 );
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

void stg_print_geom( stg_geom_t* geom )
{
  printf( "geom pose: (%.2f,%.2f,%.2f) size: [%.2f,%.2f]\n",
	  geom->pose.x,
	  geom->pose.y,
	  geom->pose.a,
	  geom->size.x,
	  geom->size.y );
}

void stg_print_laser_config( stg_laser_config_t* slc )
{
  printf( "slc fov: %.2f  range_min: %.2f range_max: %.2f samples: %d\n",
	  slc->fov,
	  slc->range_min,
	  slc->range_max,
	  slc->samples );
}

void stg_print_target( stg_target_t* tgt )
{
  printf( "target (%d.%d.%d(%s)\n",
	  tgt->world, tgt->model, tgt->prop, stg_property_string(tgt->prop) );
}

// write a single message out, putting a package header on it
int stg_fd_msg_write( int fd, 
		      stg_msg_type_t type, 
		      void* data, size_t datalen )
{
  stg_msg_t* msg = stg_msg_create( type, data, datalen );
  assert( msg );

  GByteArray* buf = g_byte_array_new();
  stg_buffer_append_msg( buf, msg );
  
  // prepend a package header
  stg_package_t pkg;
  pkg.key = STG_PACKAGE_KEY;
  pkg.payload_len = buf->len;
  
  // a real-time timestamp for performance measurements
  struct timeval tv;
  gettimeofday( &tv, NULL );
  memcpy( &pkg.timestamp, &tv, sizeof(pkg.timestamp ) );

  stg_buffer_prepend( buf, &pkg, sizeof(pkg) );
  
  int retval = 
    stg_fd_packet_write( fd, buf->data, buf->len );
  
  stg_msg_destroy( msg );
  g_byte_array_free( buf, TRUE );

  return retval;
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

stg_msec_t stg_timenow( void )
{
  struct timeval tv;
  static stg_msec_t starttime = 0;
  
  gettimeofday( &tv, NULL );
  
  stg_msec_t timenow = (stg_msec_t)( tv.tv_sec*1000 + tv.tv_usec/1000 );
  
  
  if( starttime == 0 )
    starttime = timenow;
  
  return( timenow - starttime );
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


stg_prop_t* stg_prop_create( stg_msec_t timestamp, 
			     stg_id_t world_id, 
			     stg_id_t model_id, 
			     stg_id_t prop_id,
			     void* data, 
			     size_t data_len )
{
  size_t prop_len = sizeof(stg_prop_t) + data_len;
  stg_prop_t* prop =calloc(prop_len,1);
  
  prop->timestamp = timestamp;
  prop->world = world_id;
  prop->model = model_id;
  prop->prop =  prop_id;
  prop->datalen = data_len;
  memcpy( prop->data, data, data_len );
  return prop;
}

void stg_prop_destroy( stg_prop_t* prop )
{
  if( prop ) free( prop );
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
void stg_normalize_lines( stg_line_t* lines, int num )
{
  // assuming the rectangles fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  int l;
  for( l=0; l<num; l++ )
    {
      // find the bounding rectangle
      if( lines[l].x1 < minx ) minx = lines[l].x1;
      if( lines[l].y1 < miny ) miny = lines[l].y1;      
      if( lines[l].x1 > maxx ) maxx = lines[l].x1;      
      if( lines[l].y1 > maxy ) maxy = lines[l].y1;
      if( lines[l].x2 < minx ) minx = lines[l].x2;
      if( lines[l].y2 < miny ) miny = lines[l].y2;      
      if( lines[l].x2 > maxx ) maxx = lines[l].x2;      
      if( lines[l].y2 > maxy ) maxy = lines[l].y2;
    }
  
  // now normalize all lengths so that the lines all fit inside
  // rectangle from 0,0 to 1,1
  double scalex = maxx - minx;
  double scaley = maxy - miny;

  for( l=0; l<num; l++ )
    { 
      lines[l].x1 = (lines[l].x1 - minx) / scalex;
      lines[l].y1 = (lines[l].y1 - miny) / scaley;
      lines[l].x2 = (lines[l].x2 - minx) / scalex;
      lines[l].y2 = (lines[l].y2 - miny) / scaley;
    }
}

void stg_scale_lines( stg_line_t* lines, int num, double xscale, double yscale )
{
  int l;
  for( l=0; l<num; l++ )
    {
      lines[l].x1 *= xscale;
      lines[l].y1 *= yscale;
      lines[l].x2 *= xscale;
      lines[l].y2 *= yscale;
    }
}

void stg_translate_lines( stg_line_t* lines, int num, double xtrans, double ytrans )
{
  int l;
  for( l=0; l<num; l++ )
    {
      lines[l].x1 += xtrans;
      lines[l].y1 += ytrans;
      lines[l].x2 += xtrans;
      lines[l].y2 += ytrans;
    }
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
      if( rects[r].pose.x < minx ) minx = rects[r].pose.x;
      if( rects[r].pose.y < miny ) miny = rects[r].pose.y;      
      if( rects[r].pose.x > maxx ) maxx = rects[r].pose.x;      
      if( rects[r].pose.y > maxy ) maxy = rects[r].pose.y;

      // test the extremes of the rect
      if( (rects[r].pose.x+rects[r].size.x)  < minx ) 
	minx = (rects[r].pose.x+rects[r].size.x);
      
      if( (rects[r].pose.y+rects[r].size.y)  < miny ) 
	miny = (rects[r].pose.y+rects[r].size.y);
      
      if( (rects[r].pose.x+rects[r].size.x)  > maxx ) 
	maxx = (rects[r].pose.x+rects[r].size.x);
      
      if( (rects[r].pose.y+rects[r].size.y)  > maxy ) 
	maxy = (rects[r].pose.y+rects[r].size.y);
    }
  
  // now normalize all lengths so that the rects all fit inside
  // rectangle from 0,0 to 1,1
  double scalex = maxx - minx;
  double scaley = maxy - miny;

  for( r=0; r<num; r++ )
    { 
      rects[r].pose.x = (rects[r].pose.x - minx) / scalex;
      rects[r].pose.y = (rects[r].pose.y - miny) / scaley;
      rects[r].size.x = rects[r].size.x / scalex;
      rects[r].size.y = rects[r].size.y / scaley;
    }
}	

// returns an array of 4 * num_rects stg_line_t's
stg_line_t* stg_rects_to_lines( stg_rotrect_t* rects, int num_rects )
{
  // convert rects to an array of lines
  int num_lines = 4 * num_rects;
  stg_line_t* lines = (stg_line_t*)calloc( sizeof(stg_line_t), num_lines );
  
  int r;
  for( r=0; r<num_rects; r++ )
    {
      lines[4*r].x1 = rects[r].pose.x;
      lines[4*r].y1 = rects[r].pose.y;
      lines[4*r].x2 = rects[r].pose.x + rects[r].size.x;
      lines[4*r].y2 = rects[r].pose.y;
      
      lines[4*r+1].x1 = rects[r].pose.x + rects[r].size.x;;
      lines[4*r+1].y1 = rects[r].pose.y;
      lines[4*r+1].x2 = rects[r].pose.x + rects[r].size.x;
      lines[4*r+1].y2 = rects[r].pose.y + rects[r].size.y;
      
      lines[4*r+2].x1 = rects[r].pose.x + rects[r].size.x;;
      lines[4*r+2].y1 = rects[r].pose.y + rects[r].size.y;;
      lines[4*r+2].x2 = rects[r].pose.x;
      lines[4*r+2].y2 = rects[r].pose.y + rects[r].size.y;
      
      lines[4*r+3].x1 = rects[r].pose.x;
      lines[4*r+3].y1 = rects[r].pose.y + rects[r].size.y;
      lines[4*r+3].x2 = rects[r].pose.x;
      lines[4*r+3].y2 = rects[r].pose.y;
    }
  
  return lines;
}

void stg_copybuf( void** dest, size_t* dest_len, void* src, size_t src_len )
{
  *dest = realloc( *dest, src_len );
  memcpy( *dest, src, src_len );    
  *dest_len = src_len;
} 

// sets [result] to the pose of [p2] in [p1]'s coordinate system
void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 )
{
  double cosa = cos(p1->a);
  double sina = sin(p1->a);
  
  double tx = p1->x + p2->x * cosa - p2->y * sina;
  double ty = p1->y + p2->x * sina + p2->y * cosa;
  double ta = p1->a + p2->a;
  
  result->x = tx;
  result->y = ty;
  result->a = ta;
}
