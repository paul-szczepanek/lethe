#ifndef TOKEN_H
#define TOKEN_H

#include "main.h"

const string QUEUE = "QUEUE";
const string NOUNS = "NOUNS";
const string PLACE = "PLACE";
const string EXITS = "EXITS";
const string CALLS = "CALLS";
const string QUICK = "QUICK";

const string QUEUE_CONTENTS = "[!@QUEUE]";
const string NOUNS_CONTENTS = "[!@NOUNS]";
const string PLACE_CONTENTS = "[!@PLACE]";
const string EXITS_CONTENTS = "[!@EXITS]";
const string CALLS_CONTENTS = "[!@CALLS]";
const string QUICK_CONTENTS = "[!@QUICK]";

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

namespace token
{
const size_t NUM_TOKENS_INSTRUCTION = 4;
const size_t NUM_TOKENS_CONDITION = 8;
const size_t NUM_SPACE_REMOVERS = 4;
const size_t NUM_EXPRESSION_SPACE_REMOVERS = 18;

const size_t isPaired = 0x0001;
const size_t isWide   = 0x0002;

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

  keyword,
  expression,

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

  keywordBlockMark, // not a real token, marks start and end of keyword defs
  verbBlockMark, // not a real token, marks start of a verb block
  assetBlockMark, // not a real token, marks start of a verb block

  TOKEN_NAME_MAX
};

const char Start[TOKEN_NAME_MAX] = {
  '/',    //comment,

  '<',    //keyword,
  '[',    //expression,

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

  '[',    //keywordBlockMark,
  '[',     //verbBlockMark,
  '['     //assetBlockMark,
};

const char End[TOKEN_NAME_MAX] = {
  '/',    //comment,

  '>',    //keyword,
  ']',    //expression,

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

  '<',    //keywordBlockMark,
  ':',    //verbBlockMark,
  '$'     //assetBlockMark,
};

const size_t Type[TOKEN_NAME_MAX] = {
  isWide,    //comment,

  isPaired,    //keyword,
  isPaired,    //expression,

  0,    //condition,
  0,    //instruction,

  0,    //scope,

  isWide,    //contains,
  isWide,    //notContains,
  0,    //equals,
  isWide,    //notEquals,
  isWide,    //equalsOrLess,
  isWide,    //equalsOrMore,
  0,    //isMore,
  0,    //isLess,

  0,   //and,
  0,    //or,

  isWide,    //add,
  isWide,    //remove,
  0,    //assign,

  0,    //number,
  0,    //value,
  isPaired,    //function,

  0,    //separator,
  isPaired,    //block,
  isWide,    //stop,

  isWide,    //keywordBlockMark,
  isWide,    //verbBlockMark,
  isWide    //assetBlockMark,
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
  '{',
  '}',
  '&',
  '|',
};

const char ExpressionWhitespaceRemovers[NUM_EXPRESSION_SPACE_REMOVERS] = {
  '{',
  '}',
  '<',
  '>',
  '&',
  '|',
  '+',
  '=',
  '?',
  '!',
  '#',
  '@',
  '-',
  ':',
  ',',
  '$',
  '(',
  ')'
};

} // namespace token

size_t_pair FindToken(const string& Text, token::tokenName TokenName,
                      size_t Start = 0, size_t End = 0);
size_t FindTokenStart(const string& Text, token::tokenName TokenName,
                      size_t Start = 0, size_t End = 0);
size_t FindTokenEnd(const string& Text, token::tokenName TokenName,
                    size_t Start = 0, size_t End = 0);
size_t FindCharacter(const string& Text, char Char,
                     size_t Start = 0, size_t End = 0);
void CleanWhitespace(string& Text);
string CleanEscapeCharacters(const string& Text);
void StripComments(string& Text);

inline string GetCleanWhitespace(const string& Text)
{
  string clean = Text;
  CleanWhitespace(clean);
  return clean;
}

inline bool ExtractNounVerb(const string& Text, string& Noun, string& Verb)
{
  if (!Text.empty()) {
    const size_t scopePos = FindTokenStart(Text, token::scope);
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

// handy text parsing function for escaping special chars
inline bool IsEscaped(const string& Text, size_t Pos)
{
  if (Pos > 0) {
    return (Text[Pos-1] == '\\');
  } else {
    return false;
  }
}

inline bool IsSpecial(const string& Text, size_t Pos, char token)
{
  if ((Pos > 0) && (Text[Pos-1] == '\\')) {
    return false;
  } else {
    return (Text[Pos] == token);
  }
}

#endif // TOKEN_H
