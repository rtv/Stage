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

//extern bool g_log_continue;
extern CWorld* world;


long int g_bytes_output = 0;
long int g_bytes_input = 0;

extern int g_timer_expired;

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
  printf( "[%d:%d] %s ID:(%s:%d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\techo: %d\n",
	  truth.stage_id, truth.parent_id,
	  StringType( truth.stage_type ),
	  truth.hostname,
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
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
  printf( "Send Buffer at %p is %d bytes and contains %u records:\n", 
	  send_buf, len, ((stage_header_t*)send_buf)->data );
  
  for( int t=0; t< (int)((stage_header_t*)send_buf)->data; t++ )
    {
      printf( "[%d] ", t );
      
      stage_pose_t pose;
      memcpy( &pose, (send_buf+sizeof(stage_header_t)+sizeof(stage_pose_t)*t),
	      sizeof( stage_pose_t ) );

      PrintPose( pose );
    }
}

void CWorld::DestroyConnection( int con )
{
#ifdef VERBOSE
  printf( "\nStage: Closing connection %d\n", con );
#endif

  close( m_pose_connections[con].fd );
  
  // if this was a sync connection, reduce the number of syncs we wait for
  if( m_conn_type[con] == STAGE_SYNC ) m_sync_counter--;
  
  m_pose_connection_count--;
  
  // shift the rest of the array 1 place left
  for( int p=con; p<m_pose_connection_count; p++ )
    {
      // the pollfd array
      memcpy( &(m_pose_connections[p]), 
	      &(m_pose_connections[p+1]),
	      sizeof( struct pollfd ) );
      
      // the connection type array
      memcpy( &(m_conn_type[p]), 
	      &(m_conn_type[p+1]),
	      sizeof( char ) );
    }      
  
  if( m_sync_counter < 1 && m_external_sync_required )
    m_enable = false;
  
  //printf( "Stage: remaining connections %d\n", m_pose_connection_count );
}


void CWorld::ReadPosePacket( uint32_t num_poses, int con )
{
  stage_pose_t *poses = new stage_pose_t[ num_poses ];
  size_t packet_len = sizeof(stage_pose_t) * num_poses;
  
  //printf( "Stage: expecting to read %u poses (%u bytes)\n",
  //num_poses, packet_len );
  
  // read until we have a all the poses
  int r = 0;
  while( r < (int)packet_len )
    {
      int v=0;
      // read bytes from the socket, quitting if read returns no bytes
      if( (v = read( m_pose_connections[con].fd ,
		     ((char*)poses)+r,
		     packet_len-r)) < 1 )
	{
	  DestroyConnection( con ); 
	  
	  return; // give up for this cycle
	  // we'll try again in a few milliseconds
	}
      
      r+=v;
      
      // record the total bytes input
      g_bytes_input += v; 
    }
  
  //printf( "[%d:POSES %d]",con, packet_len / sizeof(stage_pose_t) );
  //fflush( stdout );
  
  // update the model with these new poses
  for( int g=0; g<(int)num_poses; g++ )
    InputPose( poses[g], con );

  delete[] poses;
}

bool CWorld::ReadHeader( stage_header_t *hdr, int con )
{
  int r = 0;
  while( r < (int)sizeof(stage_header_t) )
    {
      int v=0;
      // read bytes from the socket, quitting if read returns no bytes
      if( (v = read( m_pose_connections[con].fd,
		     ((char*)hdr)+r,
		     sizeof(stage_header_t)-r)) < 1 )
	{
	  DestroyConnection( con ); 
	  return false; // no header
	}
      r+=v;
            
      // record the total bytes input
      g_bytes_input += v;
            
      //printf( "[%d:HDR (%d:%x)]",con, hdr->type, hdr->data ); 
      //fflush( stdout );      
    }
  return true;// got a header
}

  // read stuff until we get a continue message on each channel
void CWorld::PoseRead( void )
{
  int readable = 0;
  int syncs = 0;  
  int timeout;

  // if we have nothing to set the time, just increment it
  if( m_sync_counter == 0 ) m_step_num++;

  // in real time-mode, poll blocks until it is interrupted by
  // a timer signal, so we give it a time-out of -1. Otherwise,
  // we give it a zero time-out so it returns quickly.
  m_real_timestep > 0 ? timeout = -1 : timeout = 0;
  
  // we loop on this poll until have the syncs. in realtime mode we
  // ALSO wait for the timer to run out
  while( 1 )
    {
      //printf( "polling with timeout: %d\n", timeout );
      // use poll to see which pose connections have data
      if((readable = 
	  poll( m_pose_connections,
		m_pose_connection_count,
		timeout )) == -1) 
	{
	  if( errno == EINTR ) // interrupted by the real-time timer
	    {
	      //printf( "EINTR: syncs %d / %d\n",
	      //      syncs, m_sync_counter );
	      // if we have all our syncs, we;re done
	      if( syncs >= m_sync_counter )
		return;
	    }
	  else
	    {
	      perror( "Stage: CWorld::PoseRead poll(2)");	  
	      exit( -1 );
	    }
	}
      
      //printf( "polled %d readable\n", readable );

      if( readable > 0 ) // if something was available
	for( int t=0; t<m_pose_connection_count; t++ )// all the connections
	  {
	    short revents = m_pose_connections[t].revents;
	    
	    // if poll reported some badness on this fd
	    if( revents & POLLHUP || revents & POLLNVAL || revents & POLLERR )
	      {
		//printf( "connection %d seems bad\n", t );
		DestroyConnection( t ); // zap this connection
		
		// if we're out of connections, we'll never get an ACK!
		if(  m_pose_connection_count < 1 )
		  return;
	      }
	    else if( revents & POLLIN )// data available
	      { 
		stage_header_t hdr;
		if( ReadHeader( &hdr, t ) ) switch( hdr.type )
		  {
		  case PosePacket: // some poses are coming in 
		    ReadPosePacket( hdr.data, t );
		    break;
		  case Continue: // this marks the end of the data
		    syncs++;
		    
		    // set the step number
		    m_step_num=hdr.data;
	
		    //printf( "syncs: %d/%d timer: %d\n", 
		    //    syncs, m_sync_counter, g_timer_expired );
	    
		    // if that's all the syncs and the timer is up,
		    //we're done
		    if( syncs >= m_sync_counter && g_timer_expired > 0 ) 
		      return;

		    break;
		  default:
		    printf( "Stage warning: unknown mesg type %d\n",hdr.type);
		  }
		
	      }
	  }
      // if we're not realtime we can bail now...
      if( m_real_timestep == 0 ) break;
      
      //printf( "end\n" );
    } 
}

void CWorld::InputPose( stage_pose_t& newpose, int connection )
{		  
  stage_pose_t pose;
  
  memcpy( &pose, &newpose, sizeof(pose) );
 
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
		      
     //printf( "Stage: received " );
     //PrintPose( pose );
		      
     double newx = (double)pose.x / 1000.0;
     double newy = (double)pose.y / 1000.0;
     double newth = DTOR((double)pose.th);
		      
     //printf( "Move %d to (%.2f,%.2f,%.2f)\n",
     //      pose.stage_id, newx, newy, newth );
		      
     // update the entity with the pose
     //ent->SetGlobalPose( newx, newy, newth );
     ent->SetPose( newx, newy, newth );
		      
     double x,y,th;
     ent->GetGlobalPose( x, y, th );
		      
     //printf( "Entity %d moved to (%.2f,%.2f,%.2f)\n",
     //      pose.stage_id, x, y, th );
		      
     ent->Update( m_sim_time );
		      
     // this ent is now dirty to all channels 
     ent->MakeDirty();
		      
     // unless this channel doesn't want an echo
     // in which case it is clean just here.
     if( !pose.echo_request )
       ent->m_dirty[ connection ]= false;
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
  
  // for all the connections
  for( int t=0; t< m_pose_connection_count; t++ )
    {
      int connfd = m_pose_connections[t].fd;
      
      assert( connfd > 0 ); // must be good
      
      int send_count = 0;
      // count the MAXIMUM number of objects to be sent
      for( i=0; i < m_object_count; i++ )
	{  
	  // is the entity marked dirty for this connection?
	  if( m_object[i]->m_dirty[t] ) send_count++;
	}
      
      //printf( "Stage: Sending %d poses on connection %d/%d\n",
      //send_count, t, m_pose_connection_count );
      
      // allocate a buffer for the data
      // a header plus one stage_pose_t per object 
      size_t max_send_buf_len = 
	sizeof(stage_header_t) + send_count * sizeof( stage_pose_t ); 
      
      char* send_buf = new char[ max_send_buf_len ];
            
      // point to the first pose slot in the buffer
      stage_pose_t* next_entry = 
	(stage_pose_t*)(send_buf+sizeof(stage_header_t));


      // now we'll count the objects that really need sending
      send_count  = 0;

      // copy the data into the buffer      
      for( i=0; i < m_object_count; i++ )
	{  
	  // is the entity marked dirty for this connection?
	  if( m_object[i]->m_dirty[t] )
	    {
	      //printf( "dirty object: [%d]: %d (%d,%d,%d)\n",
	      //i, 
	      //m_object[i]->m_stage_type,
	      //m_object[i]->m_player_port,
	      //m_object[i]->m_player_type,
	      //m_object[i]->m_player_index );
	      
	      // mark it clean on this connection
	      // it won't get re-sent here until this flag is set again
	      m_object[i]->m_dirty[t] = false;
	      
	      // get the pose and fill in the structure
	      double x,y,th;
	      stage_pose_t pose;
	      
	      //m_object[i]->GetGlobalPose( x,y,th );
	      m_object[i]->GetPose( x,y,th );
	      
	      // position and extents
	      pose.x = (uint32_t)( x * 1000.0 );
	      pose.y = (uint32_t)( y * 1000.0 );
	      
	      // normalize degrees 0-360 (th is -/+PI)
	      int degrees = (int)RTOD( th );
	      if( degrees < 0 ) degrees += 360;
	      pose.th = (uint16_t)degrees;  
	      
	      // we don't want this echoed back to us
	      pose.echo_request = false;
	      pose.stage_id = i;

              pose.parent_id = -1;
              // find the index of our parent to use as an id
              for(int h=0; h<world->GetObjectCount(); h++)
              {
                if(world->GetObject(h) == m_object[i]->m_parent_object)
                {
                  pose.parent_id = h;
                  break;
                }
              }
	      
	      // copy it into the right place
	      memcpy( next_entry++, 
		      &pose, 
		      sizeof(stage_pose_t) );
	      
	      // count the number we're sending
	      send_count++;
	    }
	}
      
      
      // complete the header
      ((stage_header_t*)send_buf)->type = PosePacket;
      ((stage_header_t*)send_buf)->data = (uint32_t)send_count;
 
      size_t send_buf_len = 
	sizeof(stage_header_t) + send_count * sizeof( stage_pose_t ); 

      //PrintSendBuffer( send_buf, send_buf_len );

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

      delete[] send_buf;
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
 
  // catch signals generated by socket closures
   signal( SIGPIPE, CatchSigPipe );
 
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
      if( errno != EINTR ) // timer interrupts are OK
	{ 
	  perror( "Stage warning: poll error (not EINTR)");
	  return;
	}
    }
  
  // if the socket had a request
  if( readable && (m_pose_listen.revents & POLLIN ) ) 
    {
      // set up a socket for this connection
      struct sockaddr_in cliaddr;  
      bzero(&cliaddr, sizeof(cliaddr));
      socklen_t clilen = sizeof(cliaddr);
      
      int connfd = 0;
      
      connfd = accept( m_pose_listen.fd, (SA *) &cliaddr, &clilen);
      

      // set the dirty flag for all entities on this connection
      for( int i=0; i < m_object_count; i++ )
	m_object[i]->m_dirty[ m_pose_connection_count ] = true;


      // determine the type of connection, sync or async, by reading
      // the first byte
      char b = 0;
      int r = 0;

      if( (r = read( connfd, &b, 1 )) < 1 ) 
	{
	  puts( "failed to read sync type byte. Quitting\n" );
	  if( r < 0 ) perror( "read error" );
	  exit( -1 );
	}
      
      // record the total bytes input
      g_bytes_input += r; 


      // if this is a syncronized connection, increase the sync counter 
      switch( b )
	{
	case STAGE_SYNC: 
	  m_sync_counter++; 
#ifdef VERBOSE      
	  printf( "\nStage: SYNC pose connection accepted (id: %d fd: %d)\n", 
		  m_pose_connection_count, connfd );
	  fflush( stdout );
#endif            
	  if( m_external_sync_required ) m_enable = true;

	  break;
	case STAGE_ASYNC:
#ifdef VERBOSE      
	  printf( "\nStage: ASYNC pose connection accepted (id: %d fd: %d)\n", 
		  m_pose_connection_count, connfd );
	  fflush( stdout );
#endif            
	  break;
	default: printf( "Stage: unknown sync on %d. Quitting\n",
			 connfd );
	}
   
      // add the new connection to the arrays
      // store the connection type to help us destroy it later
      m_conn_type[ m_pose_connection_count ] = b;
      // record the pollfd data
      m_pose_connections[ m_pose_connection_count ].fd = connfd;
      m_pose_connections[ m_pose_connection_count ].events = POLLIN;
      m_pose_connection_count++;
    }


}

void CWorld::HandlePoseConnections( void )
{
  // look for new connections 
  ListenForPoseConnections();
  // write out anything that is dirty
  PoseWrite(); 
  // read in and import new poses, waiting for timers and sync messages
  PoseRead();
}
