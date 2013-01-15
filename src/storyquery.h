#ifndef STORYQUERY_H
#define STORYQUERY_H

#include "main.h"
#include "properties.h"

class Book;
class Story;
class Session;

struct Block;

class StoryQuery
{
public:
  StoryQuery(Book& _StoryBook, Story& _StoryDefinition,
             Session& _Progress, string& _Text)
    : Text(_Text), StoryBook(_StoryBook), StoryDef(_StoryDefinition),
      BookSession(_Progress) { };
  virtual ~StoryQuery() { };

  bool ExecuteExpression(const string& Noun, const string& Expression);
  size_t ExecuteBlock(const string& Noun, const Block& CurBlock);

  Properties GetVerbs(const string& Noun);

  bool GetUserValues(const string& Noun, Properties& Result);
  bool GetUserInteger(const string& Noun, Properties& Result);

private:
  bool EvaluateExpression(Properties& Result, const string& Expression,
                          bool& IsNum, bool& IsText);
  bool ExecuteFunction(const Properties& FunctionName, Properties& FunctionArgs);

public:
  string& Text;

private:
  Book& StoryBook;
  Story& StoryDef;
  Session& BookSession;
};

#endif // STORYQUERY_H
