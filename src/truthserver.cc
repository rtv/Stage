#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip.h>
#include <termios.h>
#include <strstream.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

//#define DEBUG
//#define VERBOSE
#undef DEBUG

#include "world.hh"
#include "truthserver.hh"

long int g_bytes_output = 0;
long int g_bytes_input = 0;

const int LISTENQ = 128;
//const long int MILLION = 1000000L;

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  cout << "** SIGPIPE! **" << endl;
#endif

}

void CWorld::PrintTruth( stage_truth_t &truth )
{
  printf( "[%d] %s ID:(%s:%d,%d,%d)\tPID:(%d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\techo: %d\n",
	  truth.stage_id,
	  StringType( truth.stage_type ),
	  truth.hostname,
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
	  truth.parent.port, 
	  truth.parent.type, 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h,
	  truth.echo_request );
  
  fflush( stdout );
}

void CWorld::PrintPose( stage_pose_t &pose )
{
  printf( "[%d] (%d,%d,%d) echo: %d\n",
	  pose.stage_id,
	  pose.x, pose.y, pose.th,
	  pose.echo_request );
  
  fflush( stdout );
}

void CWorld::PrintSendBuffer( char* send_buf, size_t len )
{
  printf( "Send Buffer at %p is %d bytes and contains %d records:\n", 
	  send_buf, len, (uint16_t)send_buf[0] );

  for( int t=0; t< (uint16_t)send_buf[0]; t++ )
    {
      printf( "[%d] ", t );

      stage_pose_t pose;
      memcpy( &pose, (send_buf+sizeof(uint16_t)+sizeof(stage_pose_t)*t),
	      sizeof( stage_pose_t ) );

      PrintPose( pose );
    }
}

void CWorld::DestroyConnection( int con )
{
  close( m_pose_connections[con].fd );
  
  m_pose_connection_count--;
  
  // shift the rest of the array 1 place left
  for( int p=con; p<m_pose_connection_count; p++ )
    memcpy( &(m_pose_connections[p]), 
	    &(m_pose_connections[p+1]),
	    sizeof( struct pollfd ) );
}
  
void CWorld::PoseRead( void )
{
  int readable = 1;
  
  int max_reads = 100;
  int reads = 0;
  int connfd = 0;

  // while there is stuff on the socket (subject to a sanity-check bail out)
  while( readable && (reads++ < max_reads) )
    {
      // use poll to see which pose connections have data
      if((readable = poll( m_pose_connections, m_pose_connection_count, 0 )) == -1)
	{
	  perror( "poll interrupted!");
	  return;
	}
      
      if( readable > 0 ) // if something was available
	for( int t=0; t<m_pose_connection_count; t++ )   // for all the connections
	  if( m_pose_connections[t].revents & POLLIN ) // if there was data available
	    {
	      // get the file descriptor
	      connfd = m_pose_connections[t].fd;
	
	      int r = 0;
	      uint16_t num_poses;

	      printf( "Reading header...\n" );

	      // read the two-byte header
	      while( r < (int)sizeof(num_poses) )
		{
		  int v=0;
		  // read bytes from the socket, quitting if read returns no bytes
		  if( (v = read(connfd,((char*)&num_poses)+r,
				sizeof(num_poses)-r)) < 1 )
		    {
#ifdef VERBOSE
		      printf( "Stage: pose connection broken (socket %d)\n", connfd );
#endif	  
		      DestroyConnection( t ); 
		      
		      return; // give up for this cycle
		      // we'll try again in a few milliseconds
		    }
		  
		  r+=v;
		  
		  printf( "Stage: read %d bytes of %d-byte header - value: %d\n",
			  v, sizeof(num_poses), num_poses );

		  // record the total bytes input
		  g_bytes_input += v;
		  
		  // THIS IS DEBUG OUTPUT
		  //if( v < (int)sizeof(pose) )
		  //printf( "STAGE: SHORT READ (%d/%d) r=%d\n",
		  //    v, (int)sizeof(pose), r );		  
		}
	      

	      stage_pose_t *poses = new stage_pose_t[ num_poses ];
	      
	      r = 0;
	      size_t packet_len = sizeof(stage_pose_t) * num_poses;

	      printf( "Stage: expecting to read %u poses (%u bytes)\n",
		      num_poses, packet_len );

	      // read until we have a all the poses
	      while( r < (int)packet_len )
		{
		  int v=0;
		  // read bytes from the socket, quitting if read returns no bytes
		  if( (v = read(connfd,((char*)poses)+r,packet_len-r)) < 1 )
		    {
#ifdef VERBOSE
		      printf( "Stage: pose connection broken (socket %d)\n", connfd );
#endif	  
		      DestroyConnection( t ); 
		      
		      return; // give up for this cycle
		      // we'll try again in a few milliseconds
		    }
		  
		  r+=v;

		  // record the total bytes input
		  g_bytes_input += v;

		  printf( "Stage: read %d/%d bytes\n",
			  r, packet_len );
		  // THIS IS DEBUG OUTPUT
		  //if( v < (int)sizeof(pose) )
		  //printf( "STAGE: SHORT READ (%d/%d) r=%d\n",
		  //    v, (int)sizeof(pose), r );		  
		}
	      
	      //assert( r == (int)packet_len );
	      
	      //PrintPose( pose );
	      
	      // INPUT THE POSE HERE

	      for( int g=0; g<num_poses; g++ )
		{
		  stage_pose_t pose;

		  memcpy( &pose, &(poses[g]), sizeof(pose) );
		  
		  if( pose.stage_id == -1 ) // its a command for the engine!
		    {
		      switch( pose.x ) // used to identify the command
			{
			  //case LOADc: Load( m_filename ); break;
			case SAVEc: Save( m_filename ); break;
			case PAUSEc: m_enable = !m_enable; break;
			default: printf( "Stage Warning: "
					 "Received unknown command (%d); ignoring.\n",
					 pose.x );
			}
		    }
		  else // it is an entity update - move it now
		    {
		      CEntity* ent = m_object[ pose.stage_id ];
		      assert( ent ); // there really ought to be one!
		      
		      printf( "Stage: received " );
		      PrintPose( pose );
		      
		      double newx = (double)pose.x / 1000.0;
		      double newy = (double)pose.y / 1000.0;
		      double newth = DTOR((double)pose.th);
		      
		      printf( "Move %d to (%.2f,%.2f,%.2f)\n",
			      pose.stage_id, newx, newy, newth );
		      
		      // update the entity with the pose
		      ent->SetGlobalPose( newx, newy, newth );
		      
		      double x,y,th;
		      ent->GetGlobalPose( x, y, th );
		      
		      printf( "Entity %d moved to (%.2f,%.2f,%.2f)\n",
			      pose.stage_id, x, y, th );
		      
		      ent->Update( m_sim_time );
		      
		      // this ent is now dirty to all channels 
		      ent->MakeDirty();
		      
		      // unless this channel doesn't want an echo
		      // in which case it is clean just here.
		      if( !pose.echo_request )
			ent->m_dirty[t]= false;
		    }
		}
	    }
    }
}

// recursive function that ORs an ent's dirty array with those of
// all it's ancestors 
void CEntity::InheritDirtyFromParent( int con_count )
{
  if( m_parent_object )
    {
      m_parent_object->InheritDirtyFromParent( con_count );
      
      for( int c=0; c < con_count; c++ )
	m_dirty[c] |=  m_parent_object->m_dirty[c];
    }
}

void CWorld::PoseWrite( void )
{
  int i;
  
  // every entity inherits dirty labels from its ancestors
  for( i=0; i < m_object_count; i++ )
    // recursively inherit from ancestors
    m_object[i]->InheritDirtyFromParent( m_pose_connection_count );

  // for all the connections
  for( int t=0; t< m_pose_connection_count; t++ )
    {
      int connfd = m_pose_connections[t].fd;
      
      assert( connfd > 0 ); // must be good

      int send_count = 0;
      // count the objects to be sent
      for( i=0; i < m_object_count; i++ )
	{  
	  // is the entity marked dirty for this connection?
	  if( m_object[i]->m_dirty[t] ) send_count++;
	}
      
      if( send_count > 0 )
	{
	  printf( "Stage: Sending %d poses on connection %d/%d\n",
		  send_count, t, m_pose_connection_count );
	  
	  // allocate a buffer for the data
	  // one stage_pose_t per object plus a count short
	  size_t send_buf_len = sizeof(uint16_t) + send_count * sizeof( stage_pose_t ); 
	  char* send_buf = new char[ send_buf_len ];

	  // set the count record
	  *(uint16_t*)send_buf = send_count; 

	  // point to the first pose slot in the buffer
	  stage_pose_t* next_entry = (stage_pose_t*)(send_buf+sizeof(uint16_t));
	  // copy the data into the buffer      
	  for( i=0; i < m_object_count; i++ )
	    {  
	      // is the entity marked dirty for this connection?
	      if( m_object[i]->m_dirty[t] )
		{
		  printf( "dirty object: [%d]: %d (%d,%d,%d)\n",
			  i, 
			  m_object[i]->m_stage_type,
			  m_object[i]->m_player_port,
			  m_object[i]->m_player_type,
			  m_object[i]->m_player_index );
		  
		  // get the pose and fill in the structure
		  double x,y,th;
		  stage_pose_t pose;
	      
		  m_object[i]->GetGlobalPose( x,y,th );
	      
		  pose.stage_id = i;
		  // position and extents
		  pose.x = (uint32_t)( x * 1000.0 );
		  pose.y = (uint32_t)( y * 1000.0 );

		  // normalize degrees 0-360 (th is -/+PI)
		  int degrees = (int)RTOD( th );
		  if( degrees < 0 ) degrees += 360;
	   
		  pose.th = (uint16_t)degrees;  

		  // we don't want this echoed back to us
		  pose.echo_request = false;

		  // mark it clean on this connection
		  // it won't get re-sent here until this flag is set again
		  m_object[i]->m_dirty[t] = false;

		  printf( "Assembled pose: " );
		  PrintPose( pose );

		  // copy it into the right place
		  memcpy( next_entry++, 
			  &pose, 
			  sizeof(stage_pose_t) );
		}
	    }
	  
	  PrintSendBuffer( send_buf, send_buf_len );
	  
	  // send the packet to the connected client
	  unsigned int  writecnt = 0;
	  int thiswritecnt;
	  while(writecnt < send_buf_len )
	    {
	      thiswritecnt = write(connfd, send_buf+writecnt, 
				   send_buf_len-writecnt);
	      
	      // check for error on write
	      if( thiswritecnt == -1 )
		{
		  // the fd is no good. give up on this connection
		  DestroyConnection(t);
		  break; 
		}
	      
	      writecnt += thiswritecnt;
	      
	      // record the total bytes output
	      g_bytes_output += thiswritecnt;
	    }
	}
    }
}


void CWorld::SetupPoseServer( void )
{
  m_pose_listen.fd = socket(AF_INET, SOCK_STREAM, 0);
  m_pose_listen.events = POLLIN; // notify me when a connection request happens

  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(m_pose_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( m_pose_listen.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(m_pose_listen.fd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
    {
      cout << "Port " << m_pose_port 
	   << " is in use. Quitting (but try again in a few seconds)." 
	   <<endl;
      exit( -1 );
    }
  
  // listen for requests on this socket
  // we poll it in ListenForPoseConnections()
  listen( m_pose_listen.fd, LISTENQ);
}


void CWorld::ListenForPoseConnections( void )
{
  int readable = 0;
  
  // poll for connection requests with a very fast timeout
  if((readable = poll( &m_pose_listen, 1, 0 )) == -1)
    {
      perror( "poll interrupted!");
      return;
    }
  
  // if the socket had a request
  if( readable && (m_pose_listen.revents | POLLIN ) ) 
    {
      // set up a socket for this connection
      struct sockaddr_in cliaddr;  
      bzero(&cliaddr, sizeof(cliaddr));
      socklen_t clilen = sizeof(cliaddr);
      
      int connfd = 0;
      
      connfd = accept( m_pose_listen.fd, (SA *) &cliaddr, &clilen);
      
#ifdef VERBOSE      
      printf( "Stage: pose connection accepted (socket %d)\n", 
	      connfd );
      fflush( stdout );
#endif            

      // set the dirty flag for all entities on this connection
      for( int i=0; i < m_object_count; i++ )
	m_object[i]->m_dirty[ m_pose_connection_count ] = true;

      // add the new connection to the array
      m_pose_connections[ m_pose_connection_count ].fd = connfd;
      m_pose_connections[ m_pose_connection_count ].events = POLLIN;
      m_pose_connection_count++;
    }

}

