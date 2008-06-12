#include "file_manager.hh"
#include "stage.hh" // to get PRINT_DEBUG

namespace Stg
{
	FileManager::FileManager() {
		SharePath = INSTALL_PREFIX "/share/stage";
		AssetPath = SharePath + '/' + "assets";
		CtrlPath = getenv("STAGEPATH");
		WorldsRoot = ".";
		
		paths.push_back( "." );
		paths.push_back( SharePath );
		paths.push_back( AssetPath );
		paths.push_back( CtrlPath );
	}

	std::string FileManager::fullPath( std::string filename ) {
		PRINT_DEBUG1("FileManager: trying %s\n", filename.c_str());
		if ( readable( filename ) )
			return filename;
		
		for ( unsigned int i=0; i<paths.size(); i++ ) {
			std::string path = paths[i] + '/' + filename;
			PRINT_DEBUG1("FileManager: trying %s\n", path.c_str());
			if ( readable( path ) ) {
				return path;
			}
		}
		
		PRINT_DEBUG("FileManager: Not found.\n");
		return "";
	}
	
	/*std::string FileManager::fullPathImage( std::string filename ) {
		std::string path = ImgPath + '/' + filename;
		if ( readable ( path ) )
			return path;
		else
			return "";
	}*/
	
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
		size_t loc = path.find_last_of( pathChars );
		if ( loc < 0 )
			return path;
		else
			return path.substr( 0, loc );
	}
	
}; // namespace Stg 

