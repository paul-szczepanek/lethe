#include "disk.h"
#include <sys/stat.h>
#include <dirent.h>

/** @brief Return false if path doesn't exist, add files to vector
  */
bool Disk::ListFiles(const string& Path, vector<string>& Files)
{
  DIR* directory = opendir(Path.c_str());
  if (directory == NULL) {
    LOG("Error " + IntoString(errno) + " trying to access: " + Path);
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

/** @brief Write file to disk and close
  */
bool Disk::Write(const string& Filename, const string& Text)
{
  ofstream writeFile(Filename.c_str());
  if (writeFile.is_open()) {
    writeFile << Text << std::flush;
    writeFile.close();
    return true;
  } else {
    LOG(Filename + " - can't write file");
    return false;
  }
}

bool Disk::Delete(const string& Filename)
{
  return (remove(Filename.c_str()) == 0);
}

bool Disk::Exists(const string& Filename)
{
  struct stat fileStat;
  return (stat(Filename.c_str(), &fileStat) != -1);
}
