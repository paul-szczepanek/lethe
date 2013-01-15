#include "file.h"

File::~File()
{
  if (Opened) {
    Stream.close();
  }
}

bool File::Open(const string& Filename)
{
  Stream.open(Filename.c_str());
  if (Stream.is_open()) {
    Opened = true;
    return true;
  } else {
    cout << Filename << " - missing file" << endl;
    return false;
  }
}

bool File::Empty()
{
  return Stream.eof();
}

bool File::GetLine(string& Buffer)
{
  if (!Opened || Stream.eof()) {
    return false;
  } else {
    getline(Stream, Buffer);
    return true;
  }
}
