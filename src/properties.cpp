#include "properties.h"
#include "tokens.h"

Properties::Properties(const string& Value) : IntValue(0)
{
  size_t pos = 0;
  const size_t length = Value.size();
  while (pos < length) {
    size_t sepPos = FindTokenStart(Value, token::separator, pos);
    size_t endPos = sepPos != string::npos? sepPos : length;

    // is it a number?
    if (Value[pos] == token::Start[token::number]) {
      IntValue = IntoInt(CutString(Value, pos+1, endPos));
    } else {
      // in case there is an escaped # (madness)
      if (Value[pos] == '\\') {
        ++pos;
      }
      AddValue(CutString(Value, pos, endPos));
    }
    pos = ++endPos;
  }
}


/** @brief print the values creation string, text and integer like so v1,v2,0
  */
const string Properties::PrintValues() const
{
  string text;
  for (size_t i = 0, for_size = TextValues.size(); i < for_size; ++i) {
    text += TextValues[i];
    text += VALUE_SEPARATOR;
  }
  text += "#";
  text += IntoString(IntValue);
  return text;
}

/** @brief return string with verbs as keywords
  */
const string Properties::PrintValueSelectList(const string& Noun,
    const string& VerbName,
    const string& Separator) const
{
  string text;
  for (size_t i = 0, for_size = TextValues.size(); i < for_size; ++i) {
    if (i && i < for_size + 1) {
      text += Separator;
    }
    text += "<";
    text += TextValues[i];
    text += "[";
    text += Noun;
    text += '=';
    text += TextValues[i];
    text += ":";
    text += VerbName;
    text += "]>";
  }
  return CleanEscapeCharacters(text);
}

/** @brief return string with verbs as keywords
  */
const string Properties::PrintKeywordList(const string& Separator) const
{
  string text;
  for (size_t i = 0, for_size = TextValues.size(); i < for_size; ++i) {
    if (i && i < for_size + 1) {
      text += Separator;
    }
    text += "<";
    text += TextValues[i];
    text += ">";
  }
  return CleanEscapeCharacters(text);
}

/** @brief return string with verbs as keywords
  */
const string Properties::PrintPlainList(const string& Separator) const
{
  string text;
  for (size_t i = 0, for_size = TextValues.size(); i < for_size; ++i) {
    if (i && i < for_size + 1) {
      text += Separator;
    }
    text += TextValues[i];
  }
  return CleanEscapeCharacters(text);
}

/** @brief Add all TextValues from the pased in Values if needed
  *
  * \return false if nothing needed doing
  */
bool Properties::AddValues(const Properties& Value)
{
  bool changed = false;
  for (const string& text : Value.TextValues) {
    changed |= AddValue(text);
  }
  return changed;
}

/** @brief Concatenate all text values with passed in text values
  *
  * \return false if nothing needed doing
  */
bool Properties::ConcatValues(const Properties& Value)
{
  for (string& text : TextValues) {
    for (const string& textAdded : Value.TextValues) {
      text += textAdded;
    }
  }
  return !TextValues.empty() && !Value.TextValues.empty();
}

/** @brief Remove all Text Values that aren't present in the passes in value
  *
  * \return false if nothing needed doing
  */
bool Properties::CommonValues(const Properties& Value)
{
  bool changed = false;
  Properties common;
  for (const string& text : TextValues) {
    if (Value.ContainsValue(text)) {
      common.AddValue(text);
    } else {
      changed = true;
    }
  }
  TextValues = common.TextValues;
  return changed;
}

/** @brief Remove all TextValues from the pased in Values if needed
  *
  * \return false if nothing needed doing
  */
bool Properties::RemoveValues(const Properties& Value)
{
  bool changed = false;
  for (const string& text : Value.TextValues) {
    changed |= RemoveValue(text);
  }
  return changed;
}

/** @brief Replace old values with new ones (except Int)
  *
  * \return false if nothing needed doing
  */
bool Properties::SetValues(const Properties& Value)
{
  if (IsEquivalent(Value)) {
    return false;
  }
  TextValues = Value.TextValues;
  Dirty = true;
  return true;
}

/** @brief Are the values the same (need not be in the same order)
  *
  * \return true if both text values and the integer are the same
  */
bool Properties::IsEquivalent(const Properties& Value) const
{
  if (Value.TextValues.size() == TextValues.size()
      && Value.IntValue == IntValue) {
    return ContainsValues(Value);
  }
  return false;
}
