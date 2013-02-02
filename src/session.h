#ifndef SESSION_H
#define SESSION_H

#include "main.h"
#include "properties.h"
#include "tokens.h"

class File;

struct Snapshot {
  Snapshot() { };
  Snapshot(sz _QueueIndex, sz _AssetsIndex, sz _ChangesIndex)
    : QueueIndex(_QueueIndex),  AssetsIndex(_AssetsIndex),
      ChangesIndex(_ChangesIndex) { };
  sz QueueIndex = 0;
  sz AssetsIndex = 0;
  sz ChangesIndex = 0;
};

struct Bookmark {
  Bookmark() { };
  string Description;
};

struct AssetState {
  AssetState() { };
  AssetState(bool _Playing) : Playing(_Playing) { };
  bool Playing = false;
};

class Session
{
public:
  Session() { };
  ~Session() { };

  bool Load();
  void Reset();
  const string GetSessionText() const;

  bool IsUserValues(const string& Noun) const;
  bool GetUserInteger(const string& Noun, Properties& ReturnValue) const;
  bool GetUserTextValues(const string& Noun, Properties& ReturnValue) const;

  inline const Properties& GetSystemValues(systemNoun Noun);
  inline void AddQueueValue(const string& Value);

  Bookmark& CreateBookmark();
  bool CreateSnapshot();
  bool LoadSnapshot(csz Index);
  void Trim();

private:
  string GetUserValuesText() const;
  string GetAssetStatesText() const;
  string GetQueueValuesText() const;


public:
  string Filename;
  string Name;
  string BookName;

  bool ValuesChanged = true;
  bool AssetsChanged = true;

  sz CurrentSnapshot = 0;

private:
  map<string, Properties> UserValues;
  map<string, AssetState> AssetStates;
  // for quick access
  const Properties* SystemNouns[SYSTEM_NOUN_MAX];
  Properties* QueueNoun;

  vector<Snapshot> Snapshots;
  vector<string> QueueHistory;
  vector<string> AssetsHistory;
  // this keeps track of all the values individually
  vector<vector<string>> ValuesHistories;
  map<string, sz> ValuesHistoryNames;
  vector<sz_pair> ValuesChanges;
  map<sz, Bookmark> Bookmarks;

  friend class Book;
  friend class StoryQuery;
};

void Session::AddQueueValue(const string& Value)
{
  QueueNoun->AddValue(Value);
}

const Properties& Session::GetSystemValues(systemNoun Noun)
{
  return *(SystemNouns[Noun]);
}

#endif // SESSION_H
