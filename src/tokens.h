#ifndef TOKEN_H
#define TOKEN_H

#include "main.h"

const string QUEUE = "QUEUE";
const string NOUNS = "NOUNS";
const string PLACE = "PLACE";
const string EXITS = "EXITS";
const string CALLS = "CALLS";
const string QUICK = "QUICK";

const string CONTENTS = "@";
const string QUEUE_CONTENTS = CONTENTS + QUEUE;
const string NOUNS_CONTENTS = CONTENTS + NOUNS;
const string PLACE_CONTENTS = CONTENTS + PLACE;
const string EXITS_CONTENTS = CONTENTS + EXITS;
const string CALLS_CONTENTS = CONTENTS + CALLS;
const string QUICK_CONTENTS = CONTENTS + QUICK;

enum bookFunction {
  bookFunctionSize,
  bookFunctionPlay,
  bookFunctionStop,
  bookFunctionKeyword,
  bookFunctionSelectValue,
  bookFunctionPrint,
  bookFunctionCloseMenu,
  bookFunctionOpenMenu,
  bookFunctionCloseBook,
  bookFunctionOpenBook,
  bookFunctionQuit,
  bookFunctionGetBooks,
  bookFunctionIsInGame,
  bookFunctionGetSessions,
  bookFunctionGetSessionName,
  bookFunctionSaveSession,
  bookFunctionBranchSession,
  bookFunctionLoadSession,
  bookFunctionNewSession,
  bookFunctionBookmark,
  bookFunctionUserBookmark,
  bookFunctionLoadSnapshot,
  bookFunctionGetSnapshots,
  bookFunctionGetBookmarks,
  bookFunctionGetSnapshotIndex,
  bookFunctionDialog,
  bookFunctionInput,
  BOOK_FUNCTION_MAX
};

enum systemNoun {
  systemQueue,
  systemNouns,
  systemPlace,
  systemExits,
  systemCalls,
  systemQuick,
  SYSTEM_NOUN_MAX
};

const string SystemNounNames[SYSTEM_NOUN_MAX] = {
  QUEUE,
  NOUNS,
  PLACE,
  EXITS,
  CALLS,
  QUICK
};

enum buttonType {
  buttonMenu,
  buttonBookmark,
  buttonDisk,
  buttonHistory,
  buttonLayout,
  BUTTON_TYPE_MAX
};

const string ButtonTypeNames[BUTTON_TYPE_MAX] = {
  "menu",
  "bookmark",
  "disk",
  "history",
  "layout"
};

namespace token
{
csz NUM_TOKENS_INSTRUCTION = 4;
csz NUM_TOKENS_CONDITION = 8;
csz NUM_SPACE_REMOVERS = 20;

csz isPaired = 0x0001;
csz isWide   = 0x0002;

enum operationName {
  plus,
  minus,
  divide,
  multiply,
  evaluate,
  integer,
  parens,
  OPERATION_NAME_MAX
};

const char Operations[OPERATION_NAME_MAX] = {
  '+',
  '-',
  '/',
  '*',
  '@',
  '#',
  '('
};

enum tokenName {
  comment,

  asset,
  noun,
  keyword,
  textBlock,

  condition,
  instruction,

  scope,

  contains,
  notContains,
  equals,
  notEquals,
  equalsOrLess,
  equalsOrMore,
  isMore,
  isLess,

  logicalAnd,
  logicalOr,

  add,
  remove,
  assign,

  number,
  value,
  function,

  separator,
  block,
  stop,

  styleTitle,
  styleMono,
  styleQuote,

  TOKEN_NAME_MAX
};

const char Start[TOKEN_NAME_MAX] = {
  '/',    //comment,

  '$',    //asset
  '[',    //noun,
  '<',    //keyword,
  '"',    //textBlock,

  '?',    //condition, cannot be wide
  '!',    //instruction, cannot be wide

  ':',    //scope, cannot be wide

  '+',    //contains,
  '-',    //notContains,
  '=',    //equals,
  '<',    //notEquals,
  '<',    //equalsOrLess,
  '>',    //equalsOrMore,
  '>',    //isMore,
  '<',    //isLess,

  '&',    //and,
  '|',    //or,

  '+',    //add,
  '-',    //remove,
  '=',    //assign,

  '#',    //number,
  '@',    //value,
  '(',    //function,

  ',',    //separator,
  '{',    //block,
  '<',    //stop

  '*',    //styleTitle,
  '=',    //styleMono,
  '_'     //styleQuote,
};

const char End[TOKEN_NAME_MAX] = {
  '/',    //comment,

  ' ',    //asset,
  ']',    //noun,
  '>',    //keyword,
  '"',    //textBlock,

  ' ',    //condition,
  ' ',    //instruction,

  ' ',    //scope,

  '=',    //contains,
  '=',    //notContains,
  ' ',    //equals,
  '>',    //notEquals,
  '=',    //equalsOrLess,
  '=',    //equalsOrMore,
  ' ',    //isMore,
  ' ',    //isLess,

  ' ',    //and,
  ' ',    //or,

  '=',    //add,
  '=',    //remove,
  ' ',    //assign,

  ' ',    //number,
  ' ',    //value,
  ')',    //function,

  ' ',    //separator,
  '}',    //block,
  '<',    //stop,

  '*',    //styleTitle,
  '=',    //styleMono,
  '_'     //styleQuote,
};

csz Type[TOKEN_NAME_MAX] = {
  isWide,       //comment,

  0,            //asset,
  isPaired,     //noun,
  isPaired,     //keyword,
  isPaired,     //textBlock,

  0,            //condition,
  0,            //instruction,

  0,            //scope,

  isWide,       //contains,
  isWide,       //notContains,
  0,            //equals,
  isWide,       //notEquals,
  isWide,       //equalsOrLess,
  isWide,       //equalsOrMore,
  0,            //isMore,
  0,            //isLess,

  0,            //and,
  0,            //or,

  isWide,       //add,
  isWide,       //remove,
  0,            //assign,

  0,            //number,
  0,            //value,
  isPaired,     //function,

  0,            //separator,
  isPaired,     //block,
  isWide,       //stop,

  isWide|isPaired, //styleTitle,
  isWide|isPaired, //styleMono,
  isWide|isPaired  //styleQuote,
};

const tokenName Conditions[NUM_TOKENS_CONDITION] = {
  contains,
  notContains,
  equals,
  notEquals,//4
  equalsOrLess,
  equalsOrMore,
  isMore,
  isLess//8
};

const tokenName Instructions[NUM_TOKENS_INSTRUCTION] = {
  add,
  remove,
  assign,
  function
};

const char WhitespaceRemovers[NUM_SPACE_REMOVERS] = {
  ':',
  '?',
  '!',
  '#',
  '@',
  '-',
  '+',
  '=',
  ',',
  '<',
  '>',
  '{',
  '}',
  '(',
  ')',
  '[',
  ']',
  '&',
  '|',
  '$'
};

} // namespace token

sz_pair FindToken(const string& Text, token::tokenName TokenName,
                  sz Start = 0, sz End = 0);
sz FindTokenStart(const string& Text, token::tokenName TokenName,
                  sz Start = 0, sz End = 0);
sz FindTokenEnd(const string& Text, token::tokenName TokenName,
                sz Start = 0, sz End = 0);
void CleanWhitespace(string& Text);
string CleanEscapeCharacters(const string& Text);
void StripComments(string& Text);

// handy text parsing function for escaping special chars
inline bool IsEscaped(const string& Text, sz Pos)
{
  if (Pos > 0) {
    return (Text[Pos-1] == '\\');
  } else {
    return false;
  }
}

inline bool IsSpecial(const string& Text, sz Pos, char token)
{
  if ((Pos > 0) && (Text[Pos-1] == '\\')) {
    return false;
  } else {
    return (Text[Pos] == token);
  }
}

/** @brief Find first unescaped passed in character
  * \return npos if not found
  */
inline sz FindCharacter(const string& Text,
                        char Char,
                        sz Start = 0,
                        sz End = 0)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }
  sz pos = Start;

  while (pos < End) {
    if (IsSpecial(Text, pos, Char)) {
      return pos;
    }
    ++pos;
  }

  return string::npos;
}

inline string GetCleanWhitespace(const string& Text)
{
  string clean = Text;
  CleanWhitespace(clean);
  return clean;
}

inline bool ExtractNounVerb(const string& Text, string& Noun, string& Verb)
{
  if (!Text.empty()) {
    csz scopePos = FindTokenStart(Text, token::scope);
    if (scopePos != string::npos) { // recurse with the new block
      if (scopePos) {
        Noun = CutString(Text, 0, scopePos);
      } else {
        Noun.clear();
      }
      Verb = CutString(Text, scopePos+1);
      return true;
    }
  }
  return false;
}

#endif // TOKEN_H
