#include "file_manager.hh"
#include "config.h" // to get INSTALL_PREFIX
#include "stage.hh" // to get PRINT_DEBUG

#include <fstream>
#include <sstream>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

std::string searchDirs(const std::vector<std::string> &dirs, const std::string &filename)
{
  for (unsigned int i = 0; i < dirs.size(); i++) {
    std::string path = dirs[i] + '/' + filename;
    PRINT_DEBUG1("FileManager: trying %s\n", path.c_str());
    if (Stg::FileManager::readable(path)) {
      return path;
    }
  }

  PRINT_DEBUG1("FileManager: %s not found.\n", filename.c_str());
  return "";
}

namespace Stg {
FileManager::FileManager() : WorldsRoot(".")
{
}

std::string FileManager::stagePath()
{
  static char *stgPath = getenv("STAGEPATH");
  if (stgPath == NULL)
    return "";
  else
    return std::string(stgPath);
}

std::string FileManager::findFile(const std::string &filename)
{
  PRINT_DEBUG1("FileManager: trying %s\n", filename.c_str());
  if (readable(filename))
    return filename;

  static std::vector<std::string> paths;
  static bool ranOnce = false;

  // initialize the path list, if necessary
  if (!ranOnce) {
    std::string SharePath = INSTALL_PREFIX "/share/stage";
    paths.push_back(SharePath);

    std::string stgPath = stagePath();

    std::istringstream is(stgPath);
    std::string path;
    while (getline(is, path, ':')) {
      paths.push_back(path);
      PRINT_DEBUG1("FileManager - INIT: added path %s\n", path.c_str());
    }

    ranOnce = true;

    PRINT_DEBUG1("FileManager - INIT: %d paths in search paths\n", int(paths.size()));
  }

  // search the path list
  return searchDirs(paths, filename);
}

void FileManager::newWorld(const std::string &worldfile)
{
  if (!worldfile.empty()) {
    WorldsRoot = stripFilename(worldfile);
  } else {
    // Since we loaded from a stream, lets use the current user's home
    // directory:
    WorldsRoot = FileManager::homeDirectory();
  }
}

std::string FileManager::homeDirectory()
{
  const char *homedir = NULL;
  if ((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  }
  return homedir;
}

bool FileManager::readable(const std::string &path)
{
  std::ifstream iFile;
  iFile.open(path.c_str());
  if (iFile.is_open()) {
    iFile.close();
    return true;
  } else {
    return false;
  }
}

std::string FileManager::stripFilename(const std::string &path)
{
  std::string pathChars("\\/");
  std::string::size_type loc = path.find_last_of(pathChars);
  if (loc == std::string::npos)
    return path;
  else
    return path.substr(0, loc);
}

} // namespace Stg
