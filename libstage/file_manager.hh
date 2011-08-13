#ifndef _FILE_MANAGER_HH_
#define _FILE_MANAGER_HH_

#include <string>
#include <vector>

namespace Stg {

	class FileManager {
	private:
		std::string WorldsRoot;

		std::string stripFilename( const std::string& path );
	public:
		FileManager();

		/// Return the path where the current worldfile was loaded from
		inline const std::string worldsRoot() const { return WorldsRoot; }
		/// Update the worldfile path
		inline void newWorld( const std::string& worldfile ) {
			WorldsRoot = stripFilename( worldfile ); }

		/// Determine whether a file can be opened for reading
		static bool readable( const std::string& path );

		/** Search for a file in the current directory, in the
		 *  prefix/share/stage location, and in the locations specified by
		 *  the STAGEPATH environment variable.  Returns the first match or
		 *  the original filename if not found.
		**/
		static std::string findFile( const std::string& filename );

		/// Return the STAGEPATH environment variable
		static std::string stagePath();
	};

};
#endif
