#ifndef BOOK_H
#define BOOK_H

#include "main.h"
#include "story.h"
#include "session.h"

class Story;
class Session;
class Properties;

class Book
{
public:
  Book() { };
  virtual ~Book() { };

  bool Open(const string& Title);

  bool Tick();

  void SetAction(const string_pair& Choice);
  string Action();
  string QuickMenu();
  string GetVerbList(const string& Noun);
  void GetNouns(Properties& Result);

  bool GetChoice(string_pair& Choice);

  bool AddAssetDefinition(const string& StoryText);
  inline const vector<string_pair>& GetAssetDefinitions() {
    return Assets;
  };
  bool GetAssetState(const string& AssetName) {
    return Progress.AssetStates[AssetName].Playing;
  };
  void SetAssetState(const string& AssetName, const bool Playing) {
    if (Progress.AssetStates[AssetName].Playing != Playing) {
      Progress.AssetsChanged = true;
      Progress.AssetStates[AssetName].Playing = Playing;
    }
  };

public:
  string BookTitle;

private:
  Session Progress;
  vector<string_pair> Assets;
  Story StoryDefinition;
};

#endif // BOOK_H
