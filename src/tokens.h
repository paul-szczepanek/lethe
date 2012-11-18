#ifndef TOKEN_H
#define TOKEN_H

#include "main.h"

namespace token
{

const size_t NUM_TOKENS_INSTRUCTION = 3;
const size_t NUM_TOKENS_CONDITION = 8;
const size_t NUM_SPACE_REMOVERS = 6;
const size_t NUM_EXPRESSION_SPACE_REMOVERS = 17;

const uint isPaired = 0x0001;
const uint isWide   = 0x0002;

enum mathName {
  plus,
  minus,
  divide,
  multiply,
  MATH_NAME_MAX
};

const char Math[MATH_NAME_MAX] = {
  '+',
  '-',
  '/',
  '*'
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

  separator,
  block,
  stop,

  keywordBlockMark, // not a real token, marks start and end of keyword defs
  verbBlockMark, // not a real token, marks start of a verb block

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

  ',',    //separator,
  '{',    //block,
  '<',    //stop

  '[',    //keywordBlockMark,
  '['     //verbBlockMark,
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

  ' ',    //separator,
  '}',    //block,
  '<',    //stop,

  '<',    //keywordBlockMark,
  ':'     //verbBlockMark,
};

const uint Type[TOKEN_NAME_MAX] = {
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

  0,    //separator,
  isPaired,    //block,
  isWide,    //stop,

  isWide,    //keywordBlockMark,
  isWide    //verbBlockMark,
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
};

const char WhitespaceRemovers[NUM_SPACE_REMOVERS] = {
  '[',
  ']',
  '{',
  '}',//4
  '&',
  '|',
};

const char ExpressionWhitespaceRemovers[NUM_EXPRESSION_SPACE_REMOVERS] = {
  '[',
  ']',
  '{',
  '}',//4
  '<',
  '>',
  '&',
  '|',//8
  '+',
  '=',
  '?',
  '!',//12
  '#',
  '@',
  '-',
  ':',//16
  ','
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
string CleanWhitespace(const string& Text);
void StripComments(string& Text);



#endif // TOKEN_H
