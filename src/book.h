#ifndef BOOK_H
#define BOOK_H

#include "main.h"

class Story;
class Session;

class Book
{
public:
  Book();
  virtual ~Book();

  bool Open(string Title);

  string Read(string& Progress, const string& Noun,
              const string& VerbName);
  string Read(Session& Progress, const string& Noun,
              const string& VerbName);

  string GetVerbList(const string& Progress, const string& Noun);
  string GetVerbList(Session& Progress, const string& Noun);

private:
  Story* StoryDefinition;
};

#endif // BOOK_H
