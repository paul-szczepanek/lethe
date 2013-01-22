#ifndef DISK_H
#define DISK_H

#include "main.h"
#include "file.h"

class Disk
{
public:
  Disk() {};
  ~Disk() {};

  static bool Write(const string& Filename, const string& Text);
  static bool Delete(const string& Filename);
  static bool Exists(const string& Filename);
  static vector<string> ListFiles(const string& Path,
                                  const string& Extension = "",
                                  bool StripExtension = false);
};

#endif // DISK_H
