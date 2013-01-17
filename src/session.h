#ifndef SESSION_H
#define SESSION_H

#include "main.h"
#include "properties.h"
#include "tokens.h"

class File;

struct Snapshot {
  Snapshot() { };
  Snapshot(size_t _QueueIndex, size_t _AssetsIndex, size_t _ChangesIndex)
    : QueueIndex(_QueueIndex),  AssetsIndex(_AssetsIndex),
      ChangesIndex(_ChangesIndex) {}
  size_t QueueIndex = 0;
  size_t AssetsIndex = 0;
  size_t ChangesIndex = 0;
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
  virtual ~Session() { };

  bool Load(File& Save);
  void Fixate();
  void Reset();
  const string GetSessionText() const;

  bool IsUserValues(const string& Noun) const;
  bool GetUserInteger(const string& Noun, Properties& ReturnValue) const;
  bool GetUserTextValues(const string& Noun, Properties& ReturnValue) const;

  inline const Properties& GetSystemValues(systemNoun Noun);
  inline void AddQueueValue(const string& Value);

  bool CreateSnapshot();
  bool LoadSnapshot(const size_t Index);

private:
  string GetUserValuesText() const;
  string GetAssetStatesText() const;
  string GetQueueValuesText() const;


public:
  string Name;
  string BookName;

  bool ValuesChanged = true;
  bool AssetsChanged = true;

  size_t CurrentSnapshot = 0;

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
  map<string, size_t> ValuesHistoryNames;
  vector<size_t_pair> ValuesChanges;

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
