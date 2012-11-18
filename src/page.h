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
};

struct Verb {
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
  virtual ~Page();

  const Verb& GetVerb(const string& VerbName) const;
  void SetValues(const string& Values);
  void AddValues(const string& Values);
  void RemoveValues(const string& Values);

private:
  void NewPlainText(size_t_pair& TokenPos, vector<Block*>& Blocks,
                    Verb& VerbBlock);
  void NewVerb(size_t_pair& TokenPos, Verb& VerbBlock,
               vector<VerbCondition>& VerbConditions);
  void NewVerbCondition(size_t_pair& TokenPos,
                        vector<VerbCondition>& VerbConditions);

  void AddVerb(Verb& VerbBlock);
  void PrintBlock(string& Text, const Block& Block);

public:
  Properties PageValues;
  vector<Verb> Verbs;

private:
  // this is the normalised Text of the page
  // TODO: don't duplicate strings, use indices to Text
  string Text;

  static Verb MissingVerb;
};

typedef map<string, Page*>::iterator page_it;


#endif // PAGE_H
