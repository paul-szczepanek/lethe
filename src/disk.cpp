#include "disk.h"
#include <sys/stat.h>
#include <dirent.h>

/** @brief Return files of given extension withou stripping the extension
  */
vector<string> Disk::ListFiles(const string& Path,
                               const string& Extension,
                               bool StripExtension)
{
  vector<string> files;
  // get raw files
  DIR* directory = opendir(Path.c_str());
  if (directory == NULL) {
    LOG("Error " + IntoString(errno) + " trying to access: " + Path);
    return files;
  }
  struct dirent* content;
  while ((content = readdir(directory)) != NULL) {
    if (DT_DIR != content->d_type) { // ignore . and ..
      string filename(content->d_name);
      files.push_back(filename);
    }
  }

  // filter files
  if (Extension.empty()) {
    return files;
  }
  vector<string> result;
  for (const string& file : files) {
    const size_t length = file.size();
    if (length > Extension.size()) {
      const string& ext = CutString(file, length - Extension.size());
      if (ext == Extension) {
        if (StripExtension) {
          const string& name = CutString(file, 0, length - Extension.size());
          result.push_back(name);
        } else {
          result.push_back(file);
        }
      }
    }
  }
  return result;
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
