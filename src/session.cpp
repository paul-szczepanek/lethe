#include "session.h"

Session::Session(const string& Text) : ValuesChanged(false)
{

  size_t pos = 0;
  size_t endPos = Text.find('\n', pos);

  Name = cutString(Text, pos, endPos);
  pos = ++endPos;

  const size_t length = Text.size();

  if (pos < length) {
    endPos = Text.find('\n', pos);
    BookName = cutString(Text, pos, endPos);
    pos = ++endPos;

    if (pos < length) {
      SetUserValues(cutString(Text, pos, length));
    }
  }
}

/** @brief Takes a multiline string and parses the values
  *
  * expects a single snapshot
  */
void Session::SetUserValues(const string& Text)
{
  const size_t length = Text.size();
  size_t pos = 0;

  while (pos < length) {
    size_t endPos = Text.find('\n', pos);
    const string& noun = cutString(Text, pos, endPos);
    pos = ++endPos;

    if (pos < length) {
      endPos = Text.find('\n', pos);
      const string& valueText = cutString(Text, pos, endPos);

      UserValues[noun] = Properties(valueText);
      pos = ++endPos;
    }
  }
}


/** @brief GetSessionText
  *
  * @todo: document this function
  */
string Session::GetUserValues() const
{
  string progress;

  //print values to string

  return progress;
}

/** @brief Make Snapshot
  *
  * creates a full record of progress we can retrieve later
  */
void Session::MakeSnapshot(const string& Progress,
                           const string& Noun,
                           const string& Verb)
{
  if (!Progress.empty()) {
    // add a new string if needed
    if (ValuesHistory.back() != Progress) {
      ValuesHistory.push_back(Progress);
    }
    // add the command and index of the progress string
    History.push_back(Snapshot(ValuesHistory.size()-1, Noun, Verb));
  }
}


/** @brief Checks if this page has a user value
  */
bool Session::IsUserValues(const string& Noun) const
{
  return UserValues.count(Noun);
}

/** @brief Check for presence in user values
  *
  * will fill in the passed in values with user values if found
  */
bool Session::GetUserValuesInteger(const string& Noun,
                                   lint& ReturnValue) const
{
  map<string, Properties>::const_iterator it = UserValues.find(Noun);
  if (it != UserValues.end()) {
    ReturnValue = it->second.IntValue;
    return true;
  }
  return false;
}

/** @brief Check for presence in user values
  *
  * will fill in the passed in values with user values if found
  */
bool Session::GetUserValuesText(const string& Noun,
                                vector<string>& ReturnValue) const
{
  map<string, Properties>::const_iterator it = UserValues.find(Noun);
  if (it != UserValues.end()) {
    ReturnValue = it->second.TextValues;
    return true;
  }
  return false;
}

