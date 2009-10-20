//#define DEBUG

#include "file_manager.hh"
#include "stage.hh" // to get PRINT_DEBUG
#include "config.h" // to get INSTALL_PREFIX

#include <sstream>
#include <fstream>

std::string searchDirs( const std::vector<std::string> dirs, const std::string filename ) {
	for ( unsigned int i=0; i<dirs.size(); i++ ) {
		std::string path = dirs[i] + '/' + filename;
		PRINT_DEBUG1("FileManager: trying %s\n", path.c_str());
		if ( Stg::FileManager::readable( path ) ) {
			return path;
		}
	}
	
	PRINT_DEBUG1("FileManager: %s not found.\n", filename.c_str() );
	return "";
}

namespace Stg
{
	FileManager::FileManager() : WorldsRoot( "." )
	{ }

	std::string FileManager::stagePath() {
		static char* stgPath = getenv("STAGEPATH");
		if ( stgPath == NULL )
			return "";
		else
			return std::string( stgPath );
	}

	std::string FileManager::findFile( const std::string filename ) {
		PRINT_DEBUG1("FileManager: trying %s\n", filename.c_str());
		if ( readable( filename ) )
			return filename;

		static std::vector<std::string> paths;
		static bool ranOnce = false;

		// initialize the path list, if necessary
		if ( !ranOnce ) {
			std::string SharePath = INSTALL_PREFIX "/share/stage";
			paths.push_back( SharePath );

			std::string stgPath = stagePath();

			std::istringstream is( stgPath );
			std::string path;
			while ( getline( is, path, ':' ) ) {
				paths.push_back( path );
				PRINT_DEBUG1("FileManager - INIT: added path %s\n", path.c_str() );
			}

			ranOnce = true;
			
			PRINT_DEBUG1("FileManager - INIT: %d paths in search paths\n", paths.size() );
		}

		// search the path list
		return searchDirs( paths, filename );
	}

	bool FileManager::readable( std::string path ) {
		std::ifstream iFile;
		iFile.open( path.c_str() );
		if ( iFile.is_open() ) {
			iFile.close();
			return true;
		}
		else {
			return false;
		}
	}

	std::string FileManager::stripFilename( std::string path ) {
		std::string pathChars( "\\/" );
                std::string::size_type loc = path.find_last_of( pathChars );
		if ( loc == std::string::npos )
			return path;
		else
			return path.substr( 0, loc );
	}

}; // namespace Stg

