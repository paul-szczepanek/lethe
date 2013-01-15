#ifndef FILE_H
#define FILE_H

#include "main.h"

class File
{
public:
  File() { };
  ~File();
  bool Open(const string& Filename);
  bool Empty();
  bool GetLine(string& Buffer);
public:
private:
  bool Opened = false;
  ifstream Stream;
};

#endif // FILE_H
