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

/** @brief Return true only if a non-empty line is returned, remove indentation
  */
bool File::GetLine(string& Buffer)
{
  if (!Opened || Stream.eof()) {
    return false;
  } else {
    getline(Stream, Buffer);
    if (Buffer.empty()) {
      return false;
    }
    // remove whitespace from start
    sz posStart = 0;
    csz length = Buffer.size();
    while (posStart < length
           && (Buffer[posStart] == ' ' || Buffer[posStart] == '\t')) {
      ++posStart;
    }
    // remove whitespace from end
    sz posEnd = length;
    while (posEnd > posStart
           && (Buffer[posEnd - 1] == ' ' || Buffer[posEnd - 1] == '\t')) {
      --posEnd;
    }
    if (posStart || posEnd < length) {
      if (posStart == posEnd) {
        Buffer.clear();
        return false;
      } else {
        Buffer = CutString(Buffer, posStart, posEnd);
      }
    }
    return true;
  }
}

/** @brief Return true only if a non-empty line is returned
  */
bool File::GetRawLine(string& Buffer)
{
  if (!Opened || Stream.eof()) {
    return false;
  } else {
    getline(Stream, Buffer);
    return !Buffer.empty();
  }
}
