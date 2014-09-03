#ifndef PAGE_H
#define PAGE_H

#include "main.h"
#include "properties.h"

class PageParser;

struct Block {
  Block(const string& Text, bool Executable = true)
    : Expression(Text), Execute(Executable) { };
  Block() { };
  ~Block() { };
  string Expression;
  vector<Block> Blocks;
  bool Execute;
  bool Else = false;
};

struct VerbBlock {
  void Reset()
  {
    VisualName.clear();
    BlockTree.Blocks.clear();
    Names.clear();
  };
  string VisualName;
  Block BlockTree;
  vector<string> Names;
};

class Page
{
public:
  Page() { };
  ~Page() { };

  void Parse(const string& SourceText);
  const VerbBlock& GetVerb(const string& Verb) const;
  void AddVerb(const VerbBlock& Verb);
  void SetValues(const string& Values);
  void AddValues(const string& Values);
  void RemoveValues(const string& Values);


public:
  Properties PageValues;
  vector<VerbBlock> Verbs;

private:
  // this is the normalised Text of the page
  // TODO: don't duplicate strings, use indices to Text
  string Text;

  static VerbBlock MissingVerb;

  friend class PageParser;
};

#endif // PAGE_H
