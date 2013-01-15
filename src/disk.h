#ifndef DISK_H
#define DISK_H

#include "main.h"
#include "file.h"

class Disk
{
public:
  Disk() {};
  ~Disk() {};
  static bool ListFiles(const string& Path, vector<string>& Files);
};

#endif // DISK_H
