#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include "main.h"
#include "page.h"

struct VerbCondition {
  string Expression;
  sz End;
};

class PageParser
{
public:
  PageParser(const string& SourceText, Page& _MyPage);
  ~PageParser() { };

private:
  sz FindNextStatement(csz Pos);

  void AddTextBlock();
  void AddCondition();
  void AddInstruction();
  void AddVerb();

  void PopOldVerbConditions();

  void PrintBlock(string& Text, const Block& Block);


private:
  vector<Block*> Blocks; // keeps track of local conditions stack
  vector<VerbCondition> VerbConditions; // top level conditions
  VerbBlock Verb; // gathers the verb definition before being copied to Verbs
  sz Pos = 0;
  sz Length;
  bool PopScopePending = false;
  bool VerbScopePending = false;

  // passed in by the page that created the parser
  const string& Text;
  Page& MyPage;
};

#endif // PAGEPARSER_H
