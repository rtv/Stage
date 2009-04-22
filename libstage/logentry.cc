#include "stage.hh"
using namespace Stg;

// static data members
std::vector<LogEntry> LogEntry::log;

LogEntry::LogEntry( stg_usec_t timestamp, Model* mod ) :
  timestamp( timestamp ),
  mod( mod ),
  pose( mod->GetPose() )
{ 
  // all log entries are added to the static vector history
  log.push_back( *this );
}



void LogEntry::Print()
{
  for( size_t i=0; i<log.size(); i++ )
	 {
		LogEntry* e = &log[i];

		printf( "%.3f\t%u\t%s\n",
				  (double)e->timestamp / 1e6,
				  e->mod->GetId(),
				  e->mod->PoseString().c_str() );
	 }
}
