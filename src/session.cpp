#include "session.h"

bool Session::Load(const string& SessionState)
{
  // if save exists
  // Reset();
  return false;
}

Session::Session(const string& Text) : ValuesChanged(false)
{

  size_t pos = 0;
  size_t endPos = Text.find('\n', pos);

  Name = CutString(Text, pos, endPos);
  pos = ++endPos;

  const size_t length = Text.size();

  if (pos < length) {
    endPos = Text.find('\n', pos);
    BookName = CutString(Text, pos, endPos);
    pos = ++endPos;

    if (pos < length) {
      SetUserValues(CutString(Text, pos, length));
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
    const string& noun = CutString(Text, pos, endPos);
    pos = ++endPos;

    if (pos < length) {
      endPos = Text.find('\n', pos);
      const string& valueText = CutString(Text, pos, endPos);

      UserValues[noun] = Properties(valueText);
      pos = ++endPos;
    }
  }
}


/** @brief Return the user values ready for writing to a file
  */
string Session::GetUserValues() const
{
  string progress;

  //print values to string

  return progress;
}

void Session::Fixate()
{
  for (size_t i = 0; i < SYSTEM_NOUN_MAX; ++i) {
    auto it = UserValues.find(SystemNounNames[i]);
    if (it != UserValues.end()) {
      SystemNouns[i] = &(it->second);
    }
    //SystemNouns[i] = &UserValues[SystemNounNames[i]];
  }
}

void Session::Reset()
{
  Name.clear();
  BookName.clear();
  UserValues.clear();
  AssetStates.clear();
  ValuesHistory.clear();
  History.clear();
  ValuesChanged = false;
  AssetsChanged = false;
}

/** @brief Make Snapshot
  *
  * creates a full record of progress we can retrieve later
  */
void Session::MakeSnapshot(const string& BookSession,
                           const string& Noun,
                           const string& Verb)
{
  if (!BookSession.empty()) {
    // add a new string if needed
    if (ValuesHistory.back() != BookSession) {
      ValuesHistory.push_back(BookSession);
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
  * will add to the passed in value with user values if found
  */
bool Session::GetUserInteger(const string& Noun,
                             Properties& ReturnValue) const
{
  map<string, Properties>::const_iterator it = UserValues.find(Noun);
  if (it != UserValues.end()) {
    ReturnValue.IntValue += it->second.IntValue;
    return true;
  }
  return false;
}

/** @brief Check for presence in user values
  *
  * will fill in the passed in values with user values if found
  */
bool Session::GetUserValues(const string& Noun,
                            Properties& ReturnValue) const
{
  map<string, Properties>::const_iterator it = UserValues.find(Noun);
  if (it != UserValues.end()) {
    ReturnValue.AddValues(it->second);
    return true;
  }
  return false;
}

