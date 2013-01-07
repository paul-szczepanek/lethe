#ifndef BOOK_H
#define BOOK_H

#include "main.h"
#include "story.h"
#include "session.h"

class Story;
class Session;
class Properties;

class Book
{
public:
  Book() { };
  virtual ~Book() { };

  bool Open(string Title);

  string Read(const string_pair& Choice);
  string Start();
  string QuickMenu();
  string GetVerbList(const string& Noun);
  void GetNouns(Properties& Result);

  bool GetChoice(string_pair& Choice);

  bool AddAssetDefinition(const string& StoryText);
  inline const vector<string_pair>& GetAssetDefinitions() {
    return Assets;
  };

private:
  vector<string_pair> Assets;
  Session Progress;
  Story StoryDefinition;
};

#endif // BOOK_H
