#ifndef BOOK_H
#define BOOK_H

#include "main.h"
#include "story.h"
#include "session.h"
#include "mediamanager.h"

class Story;
class Session;
class Properties;

class Book
{
public:
  Book() { };
  virtual ~Book() { };

  bool OpenBook(const string& Title);
  bool OpenMenu();
  bool OpenStory(const string& filename, Story& MyStory, Session& MySession);

  // these are exposed to the story
  bool ShowMenu();
  bool HideMenu();
  bool CloseBook();

  bool IsActionQueueEmpty() const;

  void SetStoryAction(const string_pair& Choice);
  void SetMenuAction(const string_pair& Choice);
  string ExecuteStoryAction();
  string ExecuteMenuAction();

  string GetQuickMenu();

  bool Tick(real DeltaTime);
  inline void DrawImage();
  inline void ShowImage(const Rect& Size);
  inline void HideImage();

  const string GetStoryVerbs(const string& Noun);
  const string GetMenuVerbs(const string& Noun);

  void GetStoryNouns(Properties& Result) const;

  bool GetChoice(string_pair& Choice) const;

  inline const vector<string_pair>& GetAssetDefinitions();
  inline bool GetAssetState(const string& AssetName);
  inline void SetAssetState(const string& AssetName, const bool Playing);

private:
  const vector<string>& GetVerbs(const string& Noun, Story& MyStory,
                                 Session& MySession);
  void SetAction(const string_pair& Choice, Session& MySession);
  string ExecuteAction(Story& MyStory, Session& MySession);
  bool AddAssetDefinition(const string& StoryText);

public:
  string BookTitle;
  bool MenuOpen = true;
  bool BookOpen = false;

private:
  Story MenuStory;
  Session GameSession;

  Story BookStory;
  Session BookSession;

  MediaManager Media;
  vector<string_pair> Assets;
};

bool Book::GetAssetState(const string& AssetName)
{
  return BookSession.AssetStates[AssetName].Playing;
};

void Book::SetAssetState(const string& AssetName, const bool Playing)
{
  if (BookSession.AssetStates[AssetName].Playing != Playing) {
    BookSession.AssetsChanged = true;
    BookSession.AssetStates[AssetName].Playing = Playing;
  }
};

void Book::DrawImage()
{
  Media.Draw();
}

void Book::ShowImage(const Rect& Size)
{
  Media.SetImageWindowSize(Size);
  Media.Visible = true;
}

void Book::HideImage()
{
  Media.Visible = false;
}

#endif // BOOK_H
