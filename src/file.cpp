#include "file.h"

File::~File()
{
  if (Opened) {
    Stream.close();
  }
}

bool File::Read(const string& Filename)
{
  Stream.open(Filename.c_str());
  if (Stream.is_open()) {
    Opened = true;
    return true;
  } else {
    LOG(Filename + " - missing file");
    return false;
  }
}

bool File::Empty()
{
  return !Opened || Stream.eof();
}

/** @brief Return true only if a non-empty line is returned
  */
bool File::GetLine(string& Buffer)
{
  if (!Opened || Stream.eof()) {
    return false;
  } else {
    getline(Stream, Buffer);
    return !Buffer.empty();
  }
}
