#include "properties.h"
#include "tokens.h"

Properties::Properties() : IntValue(0)
{
  //ctor
}

Properties::Properties(const string& Value) : IntValue(0)
{
  size_t pos = 0;
  const size_t length = Value.size();
  while (pos < length) {
    size_t sepPos = FindTokenStart(Value, token::separator, pos);
    size_t endPos = sepPos != string::npos? sepPos : length;

    // is it a number?
    if (Value[pos] == token::Start[token::number]) {
      IntValue = intoInt(cutString(Value, pos+1, endPos));
    } else {
      // in case there is an escaped # (madness)
      if (Value[pos] == '\\') {
        ++pos;
      }
      AddValue(cutString(Value, pos, endPos));
    }

    pos = ++endPos;
  }
}

/** @brief Add all TextValues from the pased in Values if needed
  *
  * \return false if nothing needed doing
  */
bool Properties::AddValues(const Properties& Value)
{
  bool dirty = false;

  typedef vector<string>::const_iterator iter;

  iter it = Value.TextValues.begin();
  for (iter endIt = Value.TextValues.end(); it != endIt; ++it) {
    if (AddValue(*it)) {
      dirty = true;
    }
  }

  return dirty;
}

/** @brief Remove all TextValues from the pased in Values if needed
  *
  * \return false if nothing needed doing
  */
bool Properties::RemoveValues(const Properties& Value)
{
  bool dirty = false;

  typedef vector<string>::const_iterator iter;

  iter it = Value.TextValues.begin();
  for (iter endIt = Value.TextValues.end(); it != endIt; ++it) {
    if (RemoveValue(*it)) {
      dirty = true;
    }
  }

  return dirty;
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
