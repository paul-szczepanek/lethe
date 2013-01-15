#include "disk.h"
#include <dirent.h>

/** @brief Return false if path doesn't exist, add files to vector
  */
bool Disk::ListFiles(const string& Path, vector<string>& Files)
{
  DIR* directory = opendir(Path.c_str());
  if (directory == NULL) {
    cout << errno << " error trying to access " << Path << endl;
    return false;
  }

  struct dirent* content;
  while ((content = readdir(directory)) != NULL) {
    if (DT_DIR != content->d_type) { // ignore . and ..
      string filename(content->d_name);
      Files.push_back(filename);
    }
  }

  closedir(directory);
  return true;
}
