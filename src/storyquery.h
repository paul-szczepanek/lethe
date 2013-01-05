#ifndef STORYQUERY_H
#define STORYQUERY_H

#include "main.h"
#include "properties.h"

class Story;
class Session;
struct Block;

class StoryQuery
{
public:
  StoryQuery(Session& _Progress, string& _Text, Story& _StoryDefinition)
    : Text(_Text), Progress(_Progress), StoryDef(_StoryDefinition)
  { };
  virtual ~StoryQuery() { };

  bool ExecuteExpression(const string& Noun, const string& Expression);
  size_t ExecuteBlock(const string& Noun, const Block& CurBlock);

private:
  bool EvaluateExpression(Properties& Result, const string& Expression,
                          bool& IsNum, bool& IsText);
  bool ExecuteFunction(const Properties& FunctionName, Properties& FunctionArgs);
  bool GetUserInteger(const string& Noun, Properties& Result);
  bool GetUserValues(const string& Noun, Properties& Result);

public:
  string& Text;

private:
  Session& Progress;
  Story& StoryDef;
};

#endif // STORYQUERY_H
