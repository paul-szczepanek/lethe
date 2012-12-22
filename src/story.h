#ifndef STORY_H
#define STORY_H

#include "main.h"
#include "properties.h"

class Page;
class Session;
struct Block;

class Story
{
public:
  Story();
  virtual ~Story();

  bool ParseKeywordDefinition(const string& StoryText);
  bool AddAssetDefinition(const string& StoryText);

  string Read(Session& Progress, const string& Noun,
              const string& VerbName);

  vector<string> GetVerbs(Session& Progress, const string& Noun);

private:
  inline Page& FindPage(const string& Noun);

  uint ExecuteBlock(Session& Progress, const string& Noun, const Block& CurBlock,
                    string& Text);
  bool ExecuteExpression(Session& Progress, const string& Noun,
                         const string& Expression, string& Text);
  bool EvaluateExpression(const Session& Progress, Properties& Result,
                          const string& Expression, bool& IsNum, bool& IsText);
  bool ExecuteFunction(const Session& Progress, const Properties& FunctionName,
                       Properties& FunctionArgs);

  string PrependPattern(const string& Keyword, const string& PageText,
                        const string& PatternName, const string& PatternText);

private:
  map<string, Page*> Pages;
  map<string, string> Patterns;
  map<string, string> Assets;

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
