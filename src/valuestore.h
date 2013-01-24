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
  inline void SetValue(const string& Key, const size_t& Value);
  inline void SetValue(const string& Key, const real& Value);
  inline void SetValue(const string& Key, const vector<string>& Values);

  inline bool GetValue(const string& Key, vector<string>& Values);

  inline bool GetValue(const string& Key, size_t& Value);
  inline bool GetValue(const string& Key, real& Value);
  inline bool GetValue(const string& Key, string& Value);

private:
  inline vector<string> GetValues(const string& Key);


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

/** @brief Returns only the values that will fit in the passed in vector
  */
inline bool ValueStore::GetValue(const string& Key,
                                  vector<string>& Values)
{
  const vector<string>& stored = GetValues(Key);
  if (!stored.empty()) {
    // Overwrite the defalts with new values but only if they're present
    for (size_t i = 0, fSz = min(Values.size(), stored.size()); i < fSz; ++i) {
      Values[i] = stored[i];
    }
    return true;
  }
  return false;
}

inline const string& ValueStore::GetValue(const string& Key)
{
  return StoredValues[Key];
}

inline bool ValueStore::GetValue(const string& Key,
                                 size_t& Value)
{
  const string& value = StoredValues[Key];
  if (value.empty()) {
    return false;
  } else {
    Value = IntoSizeT(value);
    return true;
  }
}

inline bool ValueStore::GetValue(const string& Key,
                                 real& Value)
{
  const string& value = StoredValues[Key];
  if (value.empty()) {
    return false;
  } else {
    Value = IntoReal(value);
    return true;
  }
}

inline bool ValueStore::GetValue(const string& Key,
                                 string& Value)
{
  const string& value = StoredValues[Key];
  if (value.empty()) {
    return false;
  } else {
    Value = value;
    return true;
  }
}

inline void ValueStore::SetValue(const string& Key,
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

inline void ValueStore::SetValue(const string& Key,
                                 const string& Value)
{
  StoredValues[Key] = Value;
}

inline void ValueStore::SetValue(const string& Key,
                                 const real& Value)
{
  StoredValues[Key] = IntoString(Value);
}

inline void ValueStore::SetValue(const string& Key,
                                 const size_t& Value)
{
  StoredValues[Key] = IntoString(Value);
}

inline bool IsKey(const string& Key)
{
  return Key.size() > 2 && Key[0] == '[' && Key[Key.size() - 1] == ']';
}

#endif // VALUESTORE_H
