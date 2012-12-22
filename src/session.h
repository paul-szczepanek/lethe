#ifndef SESSION_H
#define SESSION_H

#include "main.h"
#include "properties.h"

struct Snapshot {
  Snapshot() : HistoryIndex(0) { };
  Snapshot(size_t Index, const string& LastNoun, const string& LastVerb)
    : HistoryIndex(Index), Noun(LastNoun), Verb(LastVerb) { };
  size_t HistoryIndex; // this is the Progress from before the noun:verb
  string Noun;
  string Verb;
};

class Session
{
public:
  Session() : ValuesChanged(false) { };
  Session(const string& Text);
  virtual ~Session() { };

  string GetUserValues() const;
  void SetUserValues(const string& Text);

  bool IsUserValues(const string& Noun) const;
  bool GetUserInteger(const string& Noun, Properties& ReturnValue) const;
  bool GetUserValues(const string& Noun, Properties& ReturnValue) const;

  void MakeSnapshot(const string& Progress, const string& Noun,
                    const string& Verb);

public:
  string Name;
  string BookName;

  // only a local reader can have a state read by the book rather than sent
  map<string, Properties> UserValues;

private:
  bool ValuesChanged;
  vector<string> ValuesHistory;
  vector<Snapshot> History;

};

#endif // SESSION_H
