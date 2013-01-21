#ifndef VALUESTORE_H
#define VALUESTORE_H

#include "main.h"
#include "tokens.h"

class ValueStore
{
public:
  ValueStore(const string& StoreFilename);
  ~ValueStore();

  inline const string& GetValue(const string& Key);
  inline void SetValue(const string& Key, const string& Value);
  inline vector<string> GetValues(const string& Key);
  inline void SetValues(const string& Key, const vector<string>& Values);

  inline size_t GetSizeT(const string& Key);

private:
  map<string, string> StoredValues;
  string Filename;
};

/** @brief Return the string broken by newlines as a vector of strings
  */
inline vector<string> ValueStore::GetValues(const string& Key)
{
  vector<string> values;
  const string& valuesText = StoredValues[Key];
  size_t lastPos = 0;
  size_t pos = FindCharacter(valuesText, '\n', lastPos);
  while (pos != string::npos && pos > lastPos) {
    const string& value = CutString(valuesText, lastPos, pos);
    values.push_back(value);
    lastPos = ++pos;
    pos = FindCharacter(valuesText, '\n', lastPos);
  }
  if (lastPos < valuesText.size()) {
    // last value doesn't have a \n at the end
    const string& value = CutString(valuesText, lastPos);
    values.push_back(value);
  }
  return values;
}

inline void ValueStore::SetValues(const string& Key,
                                  const vector<string>& Values)
{
  string value;
  if (!Values.empty()) {
    value += Values[0];
    for (size_t i = 1, fSz = Values.size(); i < fSz; ++i) {
      value += '\n';
      value += Values[i];
    }
  }
  StoredValues[Key] = value;
}

inline const string& ValueStore::GetValue(const string& Key)
{
  return StoredValues[Key];
}

inline void ValueStore::SetValue(const string& Key,
                                 const string& Value)
{
  StoredValues[Key] = Value;
}

inline size_t ValueStore::GetSizeT(const string& Key)
{
  const string& value = StoredValues[Key];
  if (value.empty()) {
    return (size_t)0;
  } else {
    return IntoSizeT(value);
  }
}

inline bool IsKey(const string& Key)
{
  return Key.size() > 2 && Key[0] == '[' && Key[Key.size() - 1] == ']';
}

#endif // VALUESTORE_H
