#include "session.h"
#include "disk.h"

/** @brief you need to init system nouns beforehand
  */
bool Session::LoadSnapshot(const size_t Index)
{
  if (Index >= Snapshots.size() || !Index) {
    // if it's not within the history or is the start of the story
    // there's nothing to load and the story is ready to go from the init
    return false;
  }
  CurrentSnapshot = Index;
  size_t queueI = Snapshots[Index].QueueIndex;
  size_t assetI = Snapshots[Index].AssetsIndex;
  size_t changeI = Snapshots[Index].ChangesIndex;

  // load the current queue
  if (queueI > 0) {
    const string& queueValue = QueueHistory[--queueI];
    *QueueNoun = Properties(queueValue);
  } else {
    // this is at the start of the book, no loading needed
    return false;
  }

  // find the most up to date positions of value histories for each history
  vector<size_t> currentIndex;
  currentIndex.resize(ValuesHistories.size(), 0);
  for (size_t i = 0; i < changeI; ++i) {
    const size_t_pair& change = ValuesChanges[i];
    currentIndex[change.X] = change.Y;
  }

  for (const auto& value : ValuesHistoryNames) {
    const string& name = value.first;
    const size_t i = value.second;
    // all indeces here are 1-based, 0 meaning book values should be used
    if (currentIndex[i]) {
      // it's safe to decrement as it's a temporary
      const string& oldValue = ValuesHistories[i][--currentIndex[i]];
      UserValues[name] = Properties(oldValue);
    } else {
      // remove user values that are the same as in the book
      // because we don't have a record of the initial state
      auto it = UserValues.find(name);
      if (it != UserValues.end()) {
        // but don't remove system nouns, they are initialised
        bool nonSystem = true;
        for (const Properties* systemNoun : SystemNouns) {
          if (&(it->second) == systemNoun) {
            nonSystem = false;
            break;
          }
        }
        if (nonSystem) {
          UserValues.erase(it);
        }
      }
    }
  }

  AssetStates.clear();
  // assetI 0 means all assets are off
  if (assetI > 0) {
    const string& assets = AssetsHistory[--assetI];
    size_t lastPos = 0;
    size_t pos = FindCharacter(assets, '\n', lastPos);

    while (pos != string::npos && pos > lastPos) {
      const string& assetName = CutString(assets, lastPos, pos);
      AssetStates[assetName] = true;
      lastPos = ++pos;
      pos = FindCharacter(assets, '\n', lastPos);
    }
    if (lastPos < assets.size()) {
      // last value doesn't have a \n at the end
      const string& assetName = CutString(assets, lastPos);
      AssetStates[assetName] = true;
    }
  }

  return true;
}

/** @brief parse the save file and fill histories
  * Doesn't load the last snapshot.
  */
bool Session::Load()
{
  File Save;
  const string& savePath = STORY_DIR + SLASH + BookName + SLASH
                           + Filename + SESSION_EXT;
  if (!Disk::Exists(savePath) || !Save.Read(savePath)) {
    LOG(savePath + " - savefile missing");
    return false;
  }

  string buffer;
  Save.GetLine(buffer); // session name:
  Save.GetLine(Name);
  Save.GetLine(buffer); // book title:
  Save.GetLine(BookName);
  Save.GetLine(buffer);
  Save.GetLine(buffer); // steps taken:
  Save.GetLine(buffer); // #
  Save.GetLine(buffer);

  // queue
  while (Save.GetLine(buffer)) {
    QueueHistory.push_back(buffer);
  }

  // assets
  Save.GetLine(buffer); // asset states:
  Save.GetLine(buffer); // #
  Save.GetLine(buffer);
  while (Save.GetLine(buffer)) {
    string historyEntry = buffer;
    while (Save.GetLine(buffer)) {
      historyEntry += "\n";
      historyEntry += buffer;
    }
    AssetsHistory.push_back(historyEntry);
  }

  // values
  Save.GetLine(buffer); // tracked values:
  Save.GetLine(buffer); // #
  // resize the array to fit all the values tracked
  ValuesHistories.resize(IntoSizeT(buffer));
  Save.GetLine(buffer);
  while (Save.GetLine(buffer)) {
    string indexBuffer;
    Save.GetLine(indexBuffer);
    const size_t index = IntoSizeT(indexBuffer);
    ValuesHistoryNames[buffer] = IntoSizeT(index);
    while (Save.GetLine(buffer)) {
      ValuesHistories[index].push_back(buffer);
    }
  }

  // changes
  while (Save.GetLine(buffer)) {
    const size_t pos = FindCharacter(buffer, VALUE_SEPARATOR);
    const string& valueIndex = CutString(buffer, 0, pos);
    const string& changeIndex = CutString(buffer, pos + 1);
    const size_t_pair change(IntoSizeT(valueIndex), IntoSizeT(changeIndex));
    ValuesChanges.push_back(change);
  }

  // snapshots
  while (Save.GetLine(buffer)) {
    const size_t pos = FindCharacter(buffer, VALUE_SEPARATOR);
    const size_t pos2 = FindCharacter(buffer, VALUE_SEPARATOR, pos + 1);
    const string& queueIndex = CutString(buffer, 0, pos);
    const string& assetIndex = CutString(buffer, pos + 1, pos2);
    const string& changeIndex = CutString(buffer, pos2 + 1);
    const Snapshot loadedSnapshot(IntoSizeT(queueIndex),
                                  IntoSizeT(assetIndex),
                                  IntoSizeT(changeIndex));
    Snapshots.push_back(loadedSnapshot);
  }

  // bookmarks
  while (Save.GetLine(buffer)) {
    const size_t index = IntoSizeT(buffer);
    Save.GetLine(Bookmarks[index].Action);
    string bookmarkDescription;
    while (Save.GetLine(buffer)) {
      bookmarkDescription += buffer;
    }
    Bookmarks[index].Description = bookmarkDescription;
  }

  // repeat last snapshot
  LoadSnapshot(Snapshots.size() - 1);
  return true;
}

/** @brief get the string for the whole session, ready for writing into a file
  */
const string Session::GetSessionText() const
{
  string text;
  text.reserve(1024*1024); // reserve a meg outright
  // basic info
  text += "Session name:\n";
  text += Name;
  text += "\nBook title:\n";
  text += BookName;
  // queue
  text += "\n\nSteps taken:\n";
  text += IntoString(QueueHistory.size());
  text += "\n\n";
  for (const string& value : QueueHistory) {
    text += value;
    text += '\n';
  }
  // assets
  text += "\nAsset states:\n";
  text += IntoString(AssetsHistory.size());
  text += "\n\n";
  for (const string& value : AssetsHistory) {
    text += value;
    text += "\n\n";
  }
  // history of all values
  text += "\nTracked Values:\n";
  text += IntoString(ValuesHistoryNames.size());
  text += "\n\n";
  for (const auto& valueMap : ValuesHistoryNames) {
    text += valueMap.first;
    text += '\n';
    text += IntoString(valueMap.second);
    text += '\n';
    for (const string& value : ValuesHistories[valueMap.second]) {
      text += value;
      text += '\n';
    }
    text += '\n';
  }
  text += '\n';
  // value change indexes
  for (const size_t_pair& value : ValuesChanges) {
    text += IntoString(value.X);
    text += VALUE_SEPARATOR;
    text += IntoString(value.Y);
    text += '\n';
  }
  text += '\n';
  // snapshots (don't save the first snapshot, it's part of initialisation)
  for (size_t i = 1, for_size = Snapshots.size(); i < for_size; ++i) {
    const Snapshot& value = Snapshots[i];
    text += IntoString(value.QueueIndex);
    text += VALUE_SEPARATOR;
    text += IntoString(value.AssetsIndex);
    text += VALUE_SEPARATOR;
    text += IntoString(value.ChangesIndex);
    text += '\n';
  }
  text += '\n';
  // bookmarks, queueindex and description
  for (const auto& value : Bookmarks) {
    text += IntoString(value.first);
    text += '\n';
    text += value.second.Action;
    text += '\n';
    text += value.second.Description;
    text += "\n\n";
  }
  text += "\nEnd of session file";

  return text;
}

/** @brief Return the user values ready for writing to a file
  */
string Session::GetUserValuesText() const
{
  string text;
  for (const auto& valuePair : UserValues) {
    // skip queue noun
    if (&valuePair.second == SystemNouns[systemQueue]) {
      break;
    }
    text += valuePair.first;
    text += '=';
    text += valuePair.second.PrintValues();
    text += '\n';
  }
  return text;
}

/** @brief Return just the values of Queue noun
  */
string Session::GetQueueValuesText() const
{
  const Properties& queueValues = *(SystemNouns[systemQueue]);
  return queueValues.PrintTextValues();
}

/** @brief Return the asset states ready for writing to a file
  */
string Session::GetAssetStatesText() const
{
  string text;
  bool addBreak = false;
  for (const auto& valuePair : AssetStates) {
    if (valuePair.second.Playing) {
      if (addBreak) {
        text += '\n';
      } else {
        addBreak = true;
      }
      text += valuePair.first;
    }
  }
  return text;
}

void Session::Reset()
{
  Name.clear();
  BookName.clear();
  UserValues.clear();
  AssetStates.clear();
  ValuesHistories.clear();
  Snapshots.clear();
  // create the zeroth step so we can go back in history to the start
  Snapshots.push_back(Snapshot(0, 0, 0));
  CurrentSnapshot = 1;
  ValuesChanged = true;
  AssetsChanged = true;
}

/** @brief creates a full record of progress we can retrieve later
  */
bool Session::CreateSnapshot()
{
  if (CurrentSnapshot < Snapshots.size()) {
    // we have loaded an old snapshot, so the future is already written
    ++CurrentSnapshot;
    return false;
  }
  Snapshot newSnapshot = Snapshots.back();

  // record queue
  QueueHistory.push_back(GetQueueValuesText());
  newSnapshot.QueueIndex = QueueHistory.size();
  QueueNoun->Dirty = false;

  // record active assets
  if (AssetsChanged) {
    AssetsChanged = false;
    const string& text = GetAssetStatesText();
    // index 0 means no assets active
    if (text.empty()) {
      newSnapshot.AssetsIndex = 0;
    } else {
      // only push new history if last asset was empty or different from new
      if (newSnapshot.AssetsIndex == 0 || AssetsHistory.back() != text) {
        AssetsHistory.push_back(text);
      }
      newSnapshot.AssetsIndex = AssetsHistory.size();
    }
  }

  // record user values that have changed
  if (ValuesChanged) {
    ValuesChanged = false;
    bool valuedAdded = false;

    for (auto& valuePair : UserValues) {
      Properties& newValue = valuePair.second;
      const string& valueName = valuePair.first;
      if (newValue.Dirty) {
        newValue.Dirty = false;
      } else {
        // value hasn't changed
        continue;
      }
      // if it's not already in the map add the history and the map entry
      const auto it = ValuesHistoryNames.find(valueName);
      if (it == ValuesHistoryNames.end()) {
        // create a new history to hold the values
        const size_t historyI = ValuesHistories.size();
        ValuesHistories.resize(historyI + 1);
        vector<string>& history = ValuesHistories[historyI];
        // create a new map entry for this new history
        ValuesHistoryNames[valueName] = historyI;
        // create the first history value in the history
        history.push_back(newValue.PrintValues());
        ValuesChanges.push_back(size_t_pair(historyI, history.size()));
        valuedAdded = true;
      } else {
        // add the value to the existing history
        const size_t historyI = it->second;
        vector<string>& history = ValuesHistories[historyI];
        const string& oldValue = history.back();
        // todo: check all former values
        if (!newValue.IsEquivalent(Properties(oldValue))) {
          history.push_back(newValue.PrintValues());
          ValuesChanges.push_back(size_t_pair(historyI, history.size()));
          valuedAdded = true;
        }
      }
    }

    // we've added a value, move up the index tracking the newest values
    if (valuedAdded) {
      newSnapshot.ChangesIndex = ValuesChanges.size();
    }
  }

  // each move creates a snapshot, because queue is always recorded
  Snapshots.push_back(newSnapshot);
  CurrentSnapshot = Snapshots.size();
  return true;
}

/** @brief return a bookmark for current time
  * will also fill in the action from the queue
  */
Bookmark& Session::CreateBookmark()
{
  const size_t queueI = CurrentSnapshot > 1? CurrentSnapshot - 2 : 0;
  Bookmark& mark = Bookmarks[queueI];
  if (mark.Action.empty()) {
    if (queueI < QueueHistory.size()) {
      const string& queueString = QueueHistory[queueI];
      string noun, verb;
      if (ExtractNounVerb(queueString, noun, verb)) {
        mark.Action += verb;
        mark.Action += ' ';
        mark.Action += noun;
      }
    } else {
      mark.Action = "open book";
    }
  }
  return mark;
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
bool Session::GetUserTextValues(const string& Noun,
                                Properties& ReturnValue) const
{
  map<string, Properties>::const_iterator it = UserValues.find(Noun);
  if (it != UserValues.end()) {
    ReturnValue.AddValues(it->second);
    return true;
  }
  return false;
}
