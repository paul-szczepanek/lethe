#ifndef STORYQUERY_H
#define STORYQUERY_H

#include "main.h"
#include "properties.h"

class Book;
class Story;
class Session;
class Dialog;

struct Block;

class StoryQuery
{
public:
  StoryQuery(Book& StoryBook, Story& StoryDefinition,
             Session& Progress, string& Output)
    : Text(Output), QueryBook(StoryBook), QueryStory(StoryDefinition),
      QuerySession(Progress) { };
  ~StoryQuery() { };

  sz ExecuteBlock(const string& Noun, const Block& CurBlock, csz Level = 0);
  bool ExecuteExpression(const string& Noun, const string& Expression,
                         bool Condition = false);

  Properties GetVerbs(const string& Noun);

  Properties& GetUserValues(const string& Noun);
  const Properties& GetValues(const string& Noun);
  bool GetUserTextValues(const string& Noun, Properties& Result);
  bool GetUserInteger(const string& Noun, Properties& Result);

private:
  bool EvaluateExpression(Properties& Result, const string& Expression,
                          bool& IsNum, bool& IsText);
  bool ExecuteFunction(const Properties& FunctionName, Properties& FunctionArgs);

  bool CreateDialog(const string& Noun, Dialog& NewDialog);

public:
  string& Text;

private:
  Book& QueryBook;
  Story& QueryStory;
  Session& QuerySession;
};

#endif // STORYQUERY_H
