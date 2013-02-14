#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include "main.h"
#include "page.h"

struct VerbCondition {
  string Expression;
  szt End;
};

class PageParser
{
public:
  PageParser(const string& SourceText, Page& _MyPage);
  ~PageParser() { };

private:
  szt FindNextStatement(cszt From);

  void AddTextBlock();
  void AddCondition();
  void AddInstruction();
  void AddVerb();

  void PopOldVerbConditions();

  void PrintBlock(string& BlockText, const Block& Block);


private:
  vector<Block*> Blocks; // keeps track of local conditions stack
  vector<VerbCondition> VerbConditions; // top level conditions
  VerbBlock Verb; // gathers the verb definition before being copied to Verbs
  szt Pos = 0;
  szt Length;
  bool PopScopePending = false;
  bool VerbScopePending = false;

  // passed in by the page that created the parser
  const string& Text;
  Page& MyPage;
};

#endif // PAGEPARSER_H
