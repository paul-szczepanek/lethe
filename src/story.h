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
  Page& FindPage(const string& Noun);

  uint ExecuteBlock(const string& Noun, const Block& CurBlock,
                    Session& Progress, string& Text);
  bool ExecuteExpression(const string& Noun, Session& Progress,
                         const string& Expression);
  Properties EvaluateExpression(const Session& Progress,
                                const string& Expression,
                                bool& IsNum, bool& IsText);
  Properties ExecuteFunction(Session& Progress,
                             const string& FunctionName,
                             const string& FunctionParams);

  string PrependPattern(const string& Keyword, const string& PageText,
                        const string& PatternName, const string& PatternText);
  static inline bool CleanLeftSide(string& Left);

private:
  map<string, Page*> Pages;
  map<string, string> Patterns;
  map<string, string> Assets;

  static Page MissingPage;
};

#endif // STORY_H
