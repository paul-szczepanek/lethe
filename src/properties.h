#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "main.h"

class Properties
{
public:
  Properties();
  explicit Properties(const string& Value);
  virtual ~Properties() { };

  inline bool ContainsValue(const string& Value) const;
  inline bool ContainsValues(const Properties& Value) const;

  // they return false if nothing needed doing
  bool AddValues(const Properties& Value);
  bool RemoveValues(const Properties& Value);
  bool SetValues(const Properties& Value);
  bool CommonValues(const Properties& Value);
  bool ConcatValues(const Properties& Value);

  inline bool AddValue(const string& Value);
  inline bool RemoveValue(const string& Value);
  inline bool SetValue(const string& Value);

  bool IsEquivalent(const Properties& Value) const;

public:
  lint IntValue;
  // not using set as the number of values will usually be 1
  vector<string> TextValues;

};

typedef vector<string>::iterator TextValues_it;

/** @brief Check if a value is there
  * Done on a vector since the set will usually be small (mostly 1)
  */
bool Properties::ContainsValue(const string& Value) const
{
  for (const string& text : TextValues) {
    if (text == Value) {
      return true;
    }
  }
  return false;
}

/** @brief Contains all the value of the passed in Value
  * \return if any is missing return false immediately
  */
bool Properties::ContainsValues(const Properties& Value) const
{
  for (const string& text : Value.TextValues) {
    if (!ContainsValue(text)) {
      return false;
    }
  }
  return true;
}

/** @brief Add Value if needed
  * \return false if nothing needed doing
  */
bool Properties::AddValue(const string& Value)
{
  if (ContainsValue(Value) || Value.empty()) {
    return false;
  }

  TextValues.push_back(Value);
  return true;
}

/** @brief Remove Value if needed
  * \return false if nothing needed doing
  */
bool Properties::RemoveValue(const string& Value)
{
  if (Value.empty()) {
    return false;
  }
  // iterators have the most annoying syntax
  // not using erase-remove because elements are unique so we can return early
  TextValues_it it = TextValues.begin();
  for (TextValues_it endIt = TextValues.end(); it != endIt; ++it) {
    if (*it == Value) {
      TextValues.erase(it);
      return true;
    }
  }
  return false;
}

/** @brief Set Value
  * \return false if nothing needed doing
  */
bool Properties::SetValue(const string& Value)
{
  if (TextValues.size() == 1 && TextValues[0] == Value) {
    return false;
  }

  TextValues.clear();

  return AddValue(Value);
}

#endif // PROPERTIES_H
