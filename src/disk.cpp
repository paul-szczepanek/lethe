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
    string filename(content->d_name);
    if (filename != "." && filename != "..") {
      files.push_back(filename);
    }
  }

  // filter files
  if (Extension.empty()) {
    return files;
  }
  vector<string> result;
  for (const string& file : files) {
    csz length = file.size();
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

/** @brief Return files that start with the stem (stem0, stem1, etc.)
  */
vector<string> Disk::GetFileSeries(const string& Path,
                                   const string& Stem)
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
  if (Stem.empty()) {
    return files;
  }
  vector<string> result;
  sz series = 0;
  bool found = true;
  while (found) {
    found = false;
    const string suffix = IntoString(series);
    const string match = Stem + suffix;
    for (const string& file : files) {
      if (file.size() > match.size()) {
        const string& name = CutString(file, 0, match.size());
        if (name == match) {
          result.push_back(file);
          found = true;
          ++series;
          break;
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
    writeFile << Text << flush;
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
