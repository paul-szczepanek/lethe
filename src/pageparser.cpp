#include "pageparser.h"
#include "tokens.h"

void PageParser::PrintBlock(string& Text,
                            const Block& Block)
{
  Text += Block.Expression;
  if (Block.Blocks.size()) {
    Text += token::Start[token::block];
    for (sz j = 0, fSz = Block.Blocks.size(); j < fSz; ++j) {
      PrintBlock(Text, Block.Blocks[j]);
    }
    Text += token::End[token::block];
  }
}

void PageParser::AddTextBlock()
{
  // find the end of text at the next "
  csz textEnd = FindCharacter(Text, token::End[token::textBlock], ++Pos);
  const string& plainText = CutString(Text, Pos, textEnd);
  if (textEnd == string::npos) {
    LOG("Unmatched \" in segment - " + CutString(Text, Pos-1, Pos+20));
  }
  Pos = textEnd;

  // add the text inside as a new child block to the current block
  if (Verb.Names.empty()) {
    LOG("illegal text outside of a verb");
  } else if (!plainText.empty()) {
    if (Blocks.empty()) {
      LOG("No verb to add the text to, maybe you forgot to close a \": "
          + plainText + " <-is this meant to be plain text?");
    } else {
      Block newBlock(plainText);
      newBlock.Execute = false;
      Blocks.back()->Blocks.push_back(newBlock);
    }
  }

  // because we end on " which is part of this text block
  ++Pos;
}

void PageParser::AddCondition()
{
  // find the end, which ignores & | letting statements chain
  csz condEnd = FindNextStatement(++Pos);
  const string& expression = CutString(Text, Pos, condEnd);

  // global condition have different scope handling
  bool isVerbCondition = false;
  if (Verb.Names.empty()) {
    isVerbCondition = true;
  } else {
    // we keep looking for a verb definition until we find it
    // or we hit something illegal
    // this is not ideal but allows for implied scope
    csz verbPos = FindTokenStart(Text, token::noun, condEnd);
    if (verbPos < Length) {
      // ignore if it's just another condition, otherwise
      // only { block designations allowed between conditions and verbs
      // no need to check for escaped characters as they will exit early
      sz ignore = condEnd;
      const char& condChar = token::Start[token::condition];
      while (ignore < verbPos) {
        if (Text[ignore] == token::Start[token::block]) {
          ++ignore;
        } else {
          if (Text[ignore] == condChar) {
            ignore = FindNextStatement(++ignore);
          } else {
            break;
          }
        }
      }

      if (ignore >= verbPos) {
        isVerbCondition = true;
        // we have to add the verb now so we can pop old verb conditions
        // before we add this one to the pool
        if (!Verb.Names.empty()) {
          MyPage.Verbs.push_back(Verb);
          Verb.Reset();
        }
      }
    }
  }

  Pos = condEnd;
  if (isVerbCondition) {
    VerbCondition condition;
    condition.Expression = expression;
    // find the scope of this condition
    if (Pos < Length &&
        Text[Pos] == token::Start[token::block]) {
      // if the closing } doesn't exist it's set to be max size_t
      condition.End = FindTokenEnd(Text, token::block, Pos);
      ++Pos;
    } else {
      // end at size of text means implied scope of until we hit the next verb
      condition.End = Length;
    }

    PopOldVerbConditions();
    VerbConditions.push_back(condition);
  } else {
    // this is a regular condition, below a verb
    if (PopScopePending) {
      if (Blocks.empty()) {
        //Illegal definition, verb not started
        LOG("Trying to go back a block because of implied scope: "
            + expression + " but there is no block to go up to")
      } else {
        // end implied scope of the previous condition
        Blocks.pop_back();
      }
    }

    if (Blocks.empty()) {
      //Illegal definition, verb not started
      LOG("Regular conditions need to be below a verb: " + expression +
          " can be a verb condition but needs to be followed by a verb.")
    }

    // put a new block into the current block's array of blocks
    // and set it as the new current block
    Block newBlock(expression);
    newBlock.Execute = true;
    Blocks.back()->Blocks.push_back(newBlock);
    Blocks.push_back(&(Blocks.back()->Blocks.back()));

    if (Pos < Length && Text[Pos] == token::Start[token::block]) {
      ++Pos; // skip { on next iteration
      // scope ends on the next } token hit
      PopScopePending = false;
    } else {
      // scope ends on next local condition
      PopScopePending = true;
    }
  }
}

void PageParser::AddInstruction()
{
  // find the end, which ignores & | letting statements chain
  csz instEnd = FindNextStatement(++Pos);
  if (Verb.Names.empty()) {
    LOG(CutString(Text, Pos, instEnd) +
        " - illegal instruction position, must be under a named verb");
  } else {
    Block newBlock(CutString(Text, Pos, instEnd));
    newBlock.Execute = true;
    Blocks.back()->Blocks.push_back(newBlock);
  }
  Pos = instEnd;
}

void PageParser::PopOldVerbConditions()
{
  if (VerbScopePending) {
    VerbScopePending = false;
    // pop old conditions that no longer apply
    // or didn't have an explicit { } scope
    while (!VerbConditions.empty()) {
      csz& verbEnd = VerbConditions.back().End;
      if (verbEnd < Pos || verbEnd == Length) {
        VerbConditions.pop_back();
      } else {
        break;
      }
    }
  }
}

/** \brief Start a new verb block
 *
 *  If a verb block already exists it will add the verb first and then start
 *  a new one. Old verb conditions that are now out of scope are discarded.
 *  Active conditions are gathered and written to the new verb definition.
 */
void PageParser::AddVerb()
{
  // find the end of verb name at ]
  sz verbEnd = FindCharacter(Text, token::End[token::noun], ++Pos);
  if (verbEnd == string::npos) {
    LOG("Unmatched [ in segment - "+CutString(Text, Pos-1, Pos+20));
  }

  // flush the existing verb definition
  if (!Verb.Names.empty()) {
    MyPage.Verbs.push_back(Verb);
    Verb.Reset();
  }

  // make sure this is a verb definition starting with a :
  if (Text[Pos] == token::Start[token::scope]) {
    const string& verbName = CutString(Text, ++Pos, verbEnd);
    Pos = verbEnd;
    Verb.VisualName = verbName;
    if (!verbName.empty()) {
      Verb.Names.push_back(verbName);
    }

    // add all chained [:verb1][:verb2] into Names
    while (verbEnd+3 < Length
           && Text[verbEnd+1] == token::Start[token::noun]
           && Text[verbEnd+2] == token::Start[token::scope]) {
      // get the next alias of the verb
      verbEnd = FindCharacter(Text, token::End[token::noun], verbEnd+2);
      Verb.Names.push_back(CutString(Text, Pos, verbEnd));
      Pos = verbEnd;
    }

    PopOldVerbConditions();
    VerbScopePending = true;

    // and append all conditions that still apply
    string verbExpression;
    for (sz i = 0, fSz = VerbConditions.size(); i < fSz; ++i) {
      if (i) { // after the first one keep prepending &
        verbExpression += token::Start[token::logicalAnd];
      }
      verbExpression += VerbConditions[i].Expression;
    }
    Verb.BlockTree.Expression = verbExpression;
  } else {
    LOG("Illegal token " + CutString(Text, Pos, verbEnd)
        + " expected : at start of it");
  }
  // because we end on ] which is part of this token
  ++Pos;

  // new verb resets local scope and sets the verb expression as root
  Blocks.clear();
  Blocks.push_back(&Verb.BlockTree);
  PopScopePending = false;
}

sz PageParser::FindNextStatement(csz Pos)
{
  csz textPos = FindCharacter(Text, token::Start[token::textBlock], Pos);
  csz nounPos = FindCharacter(Text, token::Start[token::noun], Pos);
  csz condPos = FindCharacter(Text, token::Start[token::condition], Pos);
  csz instPos = FindCharacter(Text, token::Start[token::instruction], Pos);
  csz blockPos = FindCharacter(Text, token::Start[token::block], Pos);
  csz blockEndPos = FindCharacter(Text, token::End[token::block], Pos);
  // TODO: take a guess
  return min(textPos,
             min(nounPos,
                 min(condPos,
                     min(instPos,
                         min(blockPos, blockEndPos)))));
}

PageParser::PageParser(const string& SourceText,
                       Page& _MyPage)
  : Text(SourceText), MyPage(_MyPage)
{
  Length = Text.size();
  while (Pos < Length) {
    const char c = Text[Pos];
    if (c == token::Start[token::textBlock]) { // "
      AddTextBlock();
    } else if (c == token::Start[token::condition]) { // ?
      AddCondition();
    } else if (c == token::Start[token::instruction]) { // !
      AddInstruction();
    } else if (c == token::Start[token::noun]) { // [
      AddVerb();
    } else if (c == token::End[token::block]) { // }
      ++Pos;
      Blocks.pop_back();
      // handle } { else clause
      if (Pos < Length && Text[Pos] == token::Start[token::block]) { // {
        Blocks.back()->Blocks.push_back(Block());
        Blocks.back()->Blocks.back().Else = true;
        Blocks.push_back(&(Blocks.back()->Blocks.back()));
        ++Pos;
      }
    } else {
      LOG(string("Illegal token ")+c+" near "+CutString(Text, Pos, Pos+20));
      ++Pos;
      break;
    }
  }

  if (!Verb.Names.empty()) {
    MyPage.Verbs.push_back(Verb);
  }

  string ParsedText;
  for (sz i = 0, fSz = MyPage.Verbs.size(); i < fSz; ++i) {
    Block& topBlock = MyPage.Verbs[i].BlockTree;
    ParsedText += topBlock.Expression;
    for (sz j = 0, fSz = MyPage.Verbs[i].Names.size(); j < fSz; ++j) {
      ParsedText = ParsedText + token::Start[token::noun]
                   + token::Start[token::scope]
                   + MyPage.Verbs[i].Names[j] + token::End[token::noun];
    }
    for (sz j = 0, fSz = topBlock.Blocks.size(); j < fSz; ++j) {
      PrintBlock(ParsedText, topBlock.Blocks[j]);
    }
  }

  MyPage.Text = ParsedText;
}
