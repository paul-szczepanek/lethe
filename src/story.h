#ifndef STORY_H
#define STORY_H

#include "main.h"
#include "page.h"

class Session;

class Story
{
public:
  Story() { };
  ~Story();

  void Reset();
  void Fixate();

  bool ParseKeywordDefinition(const string& StoryText);

  inline Page& FindPage(const string& Noun);

private:
  string ApplyPatterns(const string& Keyword, const string& PageText);
  string PreparePattern(const string& Keyword, const string& PatternName,
                        const string& PatternText);


private:
  map<string, Page> Pages;
  map<string, string> Patterns;

  static Page MissingPage;
};

/** @brief Return page for noun
  */
Page& Story::FindPage(const string& Noun)
{
  const auto it = Pages.find(Noun);
  if (it != Pages.end()) {
    return it->second;
  }
  LOG(Noun + " - not defined in the story");
  return MissingPage;
}

#endif // STORY_H
