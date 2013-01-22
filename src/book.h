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
  Book();
  ~Book() { };

  bool Tick(real DeltaTime);

  Properties GetBooks();
  bool OpenBook(const string& Title);
  void CloseBook();

  bool ShowMenu();
  bool HideMenu();
  void Quit();

  Properties GetSessions(const string& Title = "");
  bool NewSession();
  bool SaveSession();
  bool BranchSession();
  bool LoadSession(const string& SessionName = "");

  void SetBookmark(const Properties& Description);
  void SetBookmark(const string& Description);
  bool LoadSnapshot(const size_t SnapshotIndex);
  bool UndoSnapshot();
  bool RedoSnapshot();

  bool AddDialog(const string& Noun);
  bool AddInputDialog(const string& Noun);

  void SetStoryAction(const string_pair& Choice);
  void SetMenuAction(const string_pair& Choice);

  bool IsActionQueueEmpty();
  string ProcessStoryQueue();
  string ProcessMenuQueue();
  string GetQuickMenu();

  void GetStoryNouns(Properties& Result);
  const string GetStoryVerbs(const string& Noun);
  const string GetMenuVerbs(const string& Noun);
  bool GetChoice(string_pair& Choice) const;

  inline void DrawImage();
  inline void ShowImage(const Rect& Size);
  inline void HideImage();
  inline bool GetAssetState(const string& AssetName);
  inline void SetAssetState(const string& AssetName, const bool Playing);

private:
  bool AddAssetDefinition(const string& StoryText);
  bool OpenMenu();
  bool OpenStory(const string& Path, Story& MyStory);

  const vector<string>& GetVerbs(const string& Noun, Story& MyStory,
                                 Session& MySession);
  void SetAction(const string_pair& Choice, Session& MySession);
  const string ProcessQueue(Story& MyStory, Session& MySession);

  void InitSession(Story& MyStory, Session& MySession);
  const string GetFreeSessionFilename(const string& Path);
  const vector<string_pair> GetSessionNamemap(const string& Path);
  bool MakeSessionNameUnique(string& Name, const vector<string_pair>& Namemap);

public:
  string BookTitle;
  bool MenuOpen = true;
  bool BookOpen = false;
  bool SessionOpen = false;
  bool ActiveBranch = true;

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
}

void Book::SetAssetState(const string& AssetName, const bool Playing)
{
  if (BookSession.AssetStates[AssetName].Playing != Playing) {
    BookSession.AssetsChanged = true;
    BookSession.AssetStates[AssetName].Playing = Playing;
  }
}

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
