#ifndef PAGE_H
#define PAGE_H

#include "main.h"
#include "properties.h"

struct Block {
  Block(const string& Text) : Expression(Text) { };
  Block() { };
  ~Block() { };
  string Expression;
  vector<Block> Blocks;
  bool Else = false;
};

struct VerbBlock {
  string VisualName;
  Block BlockTree;
  vector<string> Names;
};

struct VerbCondition {
  string Expression;
  size_t End;
};

class Page
{
public:
  Page(const string& SourceText);
  ~Page() { };

  const VerbBlock& GetVerb(const string& Verb) const;
  void SetValues(const string& Values);
  void AddValues(const string& Values);
  void RemoveValues(const string& Values);

private:
  void NewPlainText(size_t_pair& TokenPos, vector<Block*>& Blocks,
                    VerbBlock& VerbBlock);
  void NewVerb(size_t_pair& TokenPos, VerbBlock& VerbBlock,
               vector<VerbCondition>& VerbConditions);
  void NewVerbCondition(size_t_pair& TokenPos,
                        vector<VerbCondition>& VerbConditions);

  void AddVerb(VerbBlock& VerbBlock);
  void PrintBlock(string& Text, const Block& Block);


public:
  Properties PageValues;
  vector<VerbBlock> Verbs;

private:
  // this is the normalised Text of the page
  // TODO: don't duplicate strings, use indices to Text
  string Text;

  static VerbBlock MissingVerb;
};

#endif // PAGE_H
