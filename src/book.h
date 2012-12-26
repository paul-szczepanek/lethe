#ifndef BOOK_H
#define BOOK_H

#include "main.h"
#include "story.h"
#include "session.h"

class Story;
class Session;

class Book
{
public:
  Book() { };
  virtual ~Book() { };

  bool Open(string Title);

  string Read(const string& Noun, const string& VerbName);
  string Start();
  string QuickMenu();
  string GetVerbList(const string& Noun);

private:
  Session Progress;
  Story StoryDefinition;
};

#endif // BOOK_H
