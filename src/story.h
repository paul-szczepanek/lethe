#ifndef STORY_H
#define STORY_H

#include "main.h"

class Page;
class Session;

class Story
{
public:
  Story();
  virtual ~Story();

  bool ParseKeywordDefinition(const string& StoryText);

  string Read(Session& Progress, const string& Noun, const string& VerbName);
  string Start(Session& Progress);
  string QuickMenu(Session& Progress);

  vector<string> GetVerbs(Session& Progress, const string& Noun);

  inline Page& FindPage(const string& Noun);

private:
  string PrependPattern(const string& Keyword, const string& PageText,
                        const string& PatternName, const string& PatternText);

private:
  map<string, Page*> Pages;
  map<string, string> Patterns;

  static Page MissingPage;
};

typedef map<string, Page*>::iterator page_it;

/** @brief Return page for noun
  */
Page& Story::FindPage(const string& Noun)
{
  page_it it = Pages.find(Noun);

  if (it != Pages.end()) {
    return *(it->second);
  }

  LOG(Noun + " - not defined in the story");

  return MissingPage;
}

#endif // STORY_H
