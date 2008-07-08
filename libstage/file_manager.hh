#ifndef _FILE_MANAGER_HH_
#define _FILE_MANAGER_HH_

#include <fstream>
#include <string>
#include <vector>

namespace Stg {

	class FileManager {
	private:
		std::vector<std::string> paths;
		std::string SharePath;
		std::string AssetPath;
		std::string CtrlPath;
		std::string WorldsRoot;
		
		std::string stripFilename( std::string path );
	public:
		FileManager();
		
		std::string fullPath( std::string filename );
		//std::string fullPathImage( std::string filename );
		
		inline const std::string worldsRoot() const { return WorldsRoot; }
		inline void newWorld( std::string worldfile ) { 
			WorldsRoot = stripFilename( worldfile ); }
		
		bool readable( std::string path );
	};

};
#endif
