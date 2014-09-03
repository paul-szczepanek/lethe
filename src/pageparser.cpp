#include "pageparser.h"
#include "tokens.h"

void PageParser::PrintBlock(string& BlockText,
                            const Block& Block)
{
  BlockText += Block.Expression;
  if (Block.Blocks.size()) {
    BlockText += token::Start[token::block];
    for (szt j = 0, fSz = Block.Blocks.size(); j < fSz; ++j) {
      PrintBlock(BlockText, Block.Blocks[j]);
    }
    BlockText += token::End[token::block];
  }
}

void PageParser::AddTextBlock()
{
  // find the end of text at the next "
  cszt textEnd = FindCharacter(Text, token::End[token::textBlock], ++Pos);
  const string& plainText = CutString(Text, Pos, textEnd);
  if (textEnd == string::npos) {
    LOG("Unmatched \" in segment - " + CutString(Text, Pos - 1, Pos + 20));
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
  cszt condEnd = FindNextStatement(++Pos);
  const string& expression = CutString(Text, Pos, condEnd);

  // global condition have different scope handling
  bool isVerbCondition = false;
  if (Verb.Names.empty()) {
    isVerbCondition = true;
  } else {
    // we keep looking for a verb definition until we find it
    // or we hit something illegal
    // this is not ideal but allows for implied scope
    cszt verbPos = FindTokenStart(Text, token::noun, condEnd);
    if (verbPos < Length) {
      // ignore if it's just another condition, otherwise
      // only { block designations allowed between conditions and verbs
      // no need to check for escaped characters as they will exit early
      szt ignore = condEnd;
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
          MyPage.AddVerb(Verb);
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
  cszt instEnd = FindNextStatement(++Pos);
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
      cszt& verbEnd = VerbConditions.back().End;
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
  szt verbEnd = FindCharacter(Text, token::End[token::noun], ++Pos);
  if (verbEnd == string::npos) {
    LOG("Unmatched [ in segment - " + CutString(Text, Pos - 1, Pos + 20));
  }

  // flush the existing verb definition
  if (!Verb.Names.empty()) {
    MyPage.AddVerb(Verb);
    Verb.Reset();
  }

  // make sure this is a verb definition starting with a :
  if (Text[Pos] == token::Start[token::scope]) {
    ++Pos;
    // find the end of the definition ]
    // or it can end with another verb name instead :
    szt nameEnd = min(FindCharacter(Text, token::End[token::noun], Pos),
                      FindCharacter(Text, token::Start[token::scope], Pos));
    const string& verbName = CutString(Text, Pos, nameEnd);
    Pos = nameEnd;
    // if the verb name is empty it will be hidden from the drop down menu
    Verb.VisualName = verbName;
    if (!verbName.empty()) {
      Verb.Names.push_back(verbName);
    }

    // add all chained [:verb1:verb2] into Names
    while (Pos < verbEnd
           && Text[Pos] == token::Start[token::scope]) {
      // get the next alias of the verb
      ++Pos;
      nameEnd = min(FindCharacter(Text, token::End[token::noun], Pos),
                    FindCharacter(Text, token::Start[token::scope], Pos));
      Verb.Names.push_back(CutString(Text, Pos, nameEnd));
      Pos = nameEnd;
    }

    PopOldVerbConditions();
    VerbScopePending = true;

    // and append all conditions that still apply
    string verbExpression;
    for (szt i = 0, fSz = VerbConditions.size(); i < fSz; ++i) {
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

szt PageParser::FindNextStatement(cszt From)
{
  cszt textPos = FindCharacter(Text, token::Start[token::textBlock], From);
  cszt nounPos = FindCharacter(Text, token::Start[token::noun], From);
  cszt condPos = FindCharacter(Text, token::Start[token::condition], From);
  cszt instPos = FindCharacter(Text, token::Start[token::instruction], From);
  cszt blockPos = FindCharacter(Text, token::Start[token::block], From);
  cszt blockEndPos = FindCharacter(Text, token::End[token::block], From);
  // TODO: take a guess
  return min(textPos,
             min(nounPos,
                 min(condPos,
                     min(instPos,
                         min(blockPos, blockEndPos)))));
}

PageParser::PageParser(const string& SourceText,
                       Page& aMyPage)
  : Text(SourceText), MyPage(aMyPage)
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
      LOG(string("Illegal token ") + c + " near " + CutString(Text, Pos, Pos + 20));
      ++Pos;
      break;
    }
  }

  if (!Verb.Names.empty()) {
    MyPage.AddVerb(Verb);
  }

  string ParsedText;
  for (szt i = 0, fSz = MyPage.Verbs.size(); i < fSz; ++i) {
    Block& topBlock = MyPage.Verbs[i].BlockTree;
    ParsedText += topBlock.Expression;
    for (szt j = 0, fSzj = MyPage.Verbs[i].Names.size(); j < fSzj; ++j) {
      ParsedText = ParsedText + token::Start[token::noun]
                   + token::Start[token::scope]
                   + MyPage.Verbs[i].Names[j] + token::End[token::noun];
    }
    for (szt j = 0, fSzj = topBlock.Blocks.size(); j < fSzj; ++j) {
      PrintBlock(ParsedText, topBlock.Blocks[j]);
    }
  }

  MyPage.Text = ParsedText;
}
