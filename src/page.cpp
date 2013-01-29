#include "page.h"
#include "tokens.h"

VerbBlock Page::MissingVerb = { "", Block("You can't do that."), { } };

Page::Page(const string& SourceText)
  : Text(SourceText)
{
  VerbBlock verb; // gathers the verb definition before being copied to Verbs
  vector<Block*> blocks; // keeps track of local conditions stack
  vector<VerbCondition> verbConditions; // top level conditions

  size_t pos = 0;
  const size_t length = Text.size();
  bool popScopePending = false;

  while (pos < length) {
    size_t_pair keywordPos; // < >
    size_t_pair tokenPos; // [ ]

    // skip keyword definitions <name[real name]> until we hit [expression]
    keywordPos.Y = pos;
    do {
      tokenPos = FindToken(Text, token::expression, keywordPos.Y);
      keywordPos = FindToken(Text, token::keyword, keywordPos.Y);
    } while (keywordPos.X < tokenPos.X && keywordPos.Y > tokenPos.Y);

    // do we have any plaintext in front of the expression?
    if (pos < tokenPos.X) {
      // move the token position to point at the plain text
      tokenPos.Y = min(tokenPos.X, length);
      tokenPos.X = pos;

      // if inside verb copy the text to the expression on a new block
      NewPlainText(tokenPos, blocks, verb);
    } else if (tokenPos.Y != string::npos) { // expression found
      //we already know it's not escaped
      const char c = Text[tokenPos.X+1];

      if (c == token::Start[token::scope]) { // [:verb]
        // this finishes the last verb definition and starts a new one
        NewVerb(tokenPos, verb, verbConditions);

        // new verb resets local scope and sets the verb expression as root
        blocks.clear();
        blocks.push_back(&verb.BlockTree);
        popScopePending = false;
      } else { // it's a condition or instruction
        // merge []&[] and []|[]
        while (tokenPos.Y+1 < length &&
               (Text[tokenPos.Y+1] == token::Start[token::logicalAnd] ||
                Text[tokenPos.Y+1] == token::Start[token::logicalOr])) {
          tokenPos.Y = FindTokenEnd(Text, token::expression, tokenPos.Y+1);
        }

        if (c == token::Start[token::condition]) { // [?condition]
          // global condition have different scope handling
          bool isVerbCondition = false;

          if (verb.Names.empty()) {
            isVerbCondition = true;
          } else {
            // we keep looking for a verb definition until we find it
            // or we hit something illegal
            // this is not ideal but allows for implied scope
            const size_t verbPos = FindTokenStart(Text, token::verbBlock,
                                                  tokenPos.Y+1);

            if (verbPos < length) {
              // ignore if it's just another condition, otherwise
              // only block { designations allowed between conditions and verbs
              // no need to check for escaped characters as they will exit early
              size_t ignore = tokenPos.Y+1;
              while (ignore < verbPos) {
                if (Text[ignore] != token::Start[token::block]) {
                  if (Text[ignore] == token::Start[token::expression]
                      && Text[ignore+1] == token::Start[token::condition]) {
                    ignore = FindTokenEnd(Text, token::expression, ignore);
                  } else {
                    break;
                  }
                }
                ++ignore;
              }

              if (ignore == verbPos) {
                AddVerb(verb);
                isVerbCondition = true;
              }
            }
          }

          if (isVerbCondition) {
            NewVerbCondition(tokenPos, verbConditions);
          } else {
            assert(!blocks.empty()); //Illegal definition, verb not started

            // end implied scope of the previous condition
            if (popScopePending) {
              blocks.pop_back();
            }

            assert(!blocks.empty()); //Illegal definition, verb not started

            // put a new block into the current block's array of blocks
            // and set it as the new current block
            Block newBlock(CutString(Text, tokenPos.X, tokenPos.Y+1));
            blocks.back()->Blocks.push_back(newBlock);
            blocks.push_back(&(blocks.back()->Blocks.back()));

            if (tokenPos.Y+1 < length &&
                Text[tokenPos.Y+1] == token::Start[token::block]) {
              ++tokenPos.Y; // skip { on next iteration
              // scope ends on the next } token hit
              popScopePending = false;
            } else {
              // scope ends on next local condition
              popScopePending = true;
            }
          }
        } else if (c == token::Start[token::instruction]) { // [!instruction]
          if (verb.Names.empty()) {
            LOG(CutString(Text, tokenPos.X, tokenPos.Y+1) +
                " - illegal instruction position, must be under a verb");
          } else {
            assert(!blocks.empty()); //Illegal definition, verb not started

            Block newBlock(CutString(Text, tokenPos.X, tokenPos.Y+1));
            blocks.back()->Blocks.push_back(newBlock);
          }
        } else { // illegal
          LOG(CutString(Text, tokenPos.X, tokenPos.Y+1) +
              " - illegal character: " + c +", use :, ? or ! after [");
        }
      } // end condition or instruction
    } // end expression

    pos = tokenPos.Y;
    ++pos;
  } // end while

  if (!verb.Names.empty()) {
    AddVerb(verb);
  }

  string oldText = Text;
  Text.clear();

  for (size_t i = 0, fSz = Verbs.size(); i < fSz; ++i) {
    Block& topBlock = Verbs[i].BlockTree;
    Text += topBlock.Expression;
    for (size_t j = 0, fSz = Verbs[i].Names.size(); j < fSz; ++j) {
      Text = Text + token::Start[token::expression]
             + token::Start[token::scope]
             + Verbs[i].Names[j] + token::End[token::expression];
    }
    for (size_t j = 0, fSz = topBlock.Blocks.size(); j < fSz; ++j) {
      PrintBlock(Text, topBlock.Blocks[j]);
    }
  }
  Text.reserve(0);
}

void Page::PrintBlock(string& Text,
                      const Block& Block)
{
  Text += Block.Expression;
  if (Block.Blocks.size()) {
    Text += token::Start[token::block];
    for (size_t j = 0, fSz = Block.Blocks.size(); j < fSz; ++j) {
      PrintBlock(Text, Block.Blocks[j]);
    }
    Text += token::End[token::block];
  }
}

/** \brief Add a new block containing plain text
 *
 *  It will append a new block with the text as its expression.
 *  It will also deal with closing } scope for any conditions.
 */
void Page::NewPlainText(size_t_pair& TokenPos,
                        vector<Block*>& Blocks,
                        VerbBlock& Verb)
{
  bool popLocalScope = false;

  if (Verb.Names.empty()) {
    LOG("illegal text outside of a verb");
  } else if (Text[TokenPos.X] == token::Start[token::block]) {
    // handle } { else clause
    TokenPos.Y = TokenPos.X;
    Blocks.back()->Blocks.push_back(Block());
    Blocks.back()->Blocks.back().Else = true;
    Blocks.push_back(&(Blocks.back()->Blocks.back()));
  } else {
    // look for block ending }
    size_t pos = TokenPos.X;
    char scopeEndChar = token::End[token::block];
    // can't use FindTokenEnd as it might not have a pair here
    while (pos < TokenPos.Y) {
      if (Text[pos] == scopeEndChar && !IsEscaped(Text, pos)) {
        popLocalScope = true;
        TokenPos.Y = pos; // skip } on next iteration
        break; // we'll copy the rest on the next call
      }
      ++pos;
    }

    string plainText = CutString(Text, TokenPos.X, TokenPos.Y);
    if (!plainText.empty()) {
      Blocks.back()->Blocks.push_back(Block(plainText));
    }

    if (popLocalScope) {
      Blocks.pop_back();
    } else {
      --TokenPos.Y; //catch the [ again on next iteration
    }
  }
}

/** @brief Get the reference to the top Block node of the verb
  *
  * \return the block belonging to a verb by the name passed in
  */
const VerbBlock& Page::GetVerb(const string& Verb) const
{
  for (size_t i = 0, fSz = Verbs.size(); i < fSz; ++i) {
    for (size_t j = 0, fSz = Verbs[i].Names.size(); j < fSz; ++j) {
      if (Verbs[i].Names[j] == Verb) {
        return Verbs[i];
      }
    }
  }
  MissingVerb.BlockTree.Expression = "You can't " + Verb + " that.";
  return MissingVerb;
}

/** \brief Add the verb definition to the page
 *
 *  The key and block get cleared
 */
void Page::AddVerb(VerbBlock& VerbBlock)
{
  Verbs.push_back(VerbBlock);
  VerbBlock.BlockTree.Blocks.clear();
  VerbBlock.VisualName.clear();
  VerbBlock.Names.clear();
}

/** \brief Add a verb (top level) condition
 *
 *  It also closes previously unclosed conditions that didn't have { }
 */
void Page::NewVerbCondition(size_t_pair& TokenPos,
                            vector<VerbCondition>& VerbConditions)
{
  VerbCondition condition;
  condition.Expression = CutString(Text, TokenPos.X, TokenPos.Y+1);

  // find the scope of this condition
  if (TokenPos.Y+1 < Text.size() &&
      Text[TokenPos.Y+1] == token::Start[token::block]) {
    //if the closing } doesn't exist it's set to be max size_t
    condition.End = FindTokenEnd(Text, token::block, TokenPos.Y+1);
    // skip the {
    ++TokenPos.Y;
  } else { // if no scope is set default to be until the next condition
    condition.End = Text.size(); // the next condition will close it
  }

  // pop old conditions that no longer apply
  // or didn't have an explicit { } scope
  while (!VerbConditions.empty()
         && (VerbConditions.back().End < TokenPos.Y
             || VerbConditions.back().End == Text.size())) {
    VerbConditions.pop_back();
  }

  VerbConditions.push_back(condition);
}

/** \brief Start a new verb block
 *
 *  If a verb block already exists it will add the verb first and then start
 *  a new one. Old verb conditions that are now out of scope are discarded.
 *  Active conditions are gathered and written to the new verb definition.
 */
void Page::NewVerb(size_t_pair& TokenPos,
                   VerbBlock& VerbBlock,
                   vector<VerbCondition>& VerbConditions)
{
  // flush the existing verb definition
  if (!VerbBlock.Names.empty()) {
    AddVerb(VerbBlock);
  }
  const string& name = CutString(Text, TokenPos.X+2, TokenPos.Y);
  VerbBlock.VisualName = name;
  VerbBlock.Names.push_back(name);

  // add all chained [:verb1][:verb2] into Names
  while (TokenPos.Y+2 < Text.size() &&
         Text[TokenPos.Y+1] == token::Start[token::expression] &&
         Text[TokenPos.Y+2] == token::Start[token::scope]) {
    // get the next alias of the verb
    TokenPos = FindToken(Text, token::expression, TokenPos.Y+1);
    VerbBlock.Names.push_back(CutString(Text, TokenPos.X+2, TokenPos.Y));
  }

  // pop old conditions that no longer apply
  while (!VerbConditions.empty() && VerbConditions.back().End < TokenPos.X) {
    VerbConditions.pop_back();
  }

  string verbExpression;
  // and append all conditions that still apply
  for (size_t i = 0, for_s = VerbConditions.size(); i < for_s; ++i) {
    if (i) { // after the first one keep prepending &
      verbExpression += token::Start[token::logicalAnd];
    }
    verbExpression += VerbConditions[i].Expression;
  }
  VerbBlock.BlockTree.Expression = verbExpression;
}
