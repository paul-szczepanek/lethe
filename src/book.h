#ifndef BOOK_H
#define BOOK_H

#include "main.h"
#include "story.h"
#include "session.h"
#include "mediamanager.h"

class Story;
class Session;
class Properties;

struct Dialog {
  string Noun;
  vector<string> Buttons;
  string Message;
  bool InputBox = false;
};

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
  bool ShowSaveMenu();
  bool ShowHistoryMenu();
  bool ShowBookmarkMenu();
  void Quit();

  Properties GetSessions(const string& Title = "");
  const string GetSessionName();
  bool NewSession();
  bool SaveSession(const string& NewName = "");
  bool BranchSession(const string& NewName = "");
  bool LoadSession(const string& SessionName = "");

  bool LoadSnapshot(const string& Description);
  void GetSnapshots(Properties& SnapshotItems);
  void GetBookmarks(Properties& SnapshotItems);
  szt GetCurrentSnapshot();

  void SetBookmark(const Properties& Description);
  void SetBookmark(const string& Description);
  bool LoadSnapshot(cszt SnapshotIndex);
  bool UndoSnapshot();
  bool RedoSnapshot();

  void AddDialog(const Dialog& dialog);

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
  Properties& GetMenuValues(const string& Noun);

  inline bool GetAssetState(const string& AssetName);
  inline void SetAssetState(const string& AssetName, const bool Playing);

  inline MediaManager* GetMediaManagerPointer()
  {
    return &Media;
  };

#ifdef DEVBUILD
  const string ShowVariables();
#endif

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
  bool MenuOpen = false;
  bool BookOpen = false;
  bool DialogOpen = false;
  bool SessionOpen = false;
  bool ActiveBranch = true;

  vector<Dialog> Dialogs;

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




#endif // BOOK_H
