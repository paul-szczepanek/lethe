#include "book.h"
#include "page.h"
#include "tokens.h"
#include "storyquery.h"
#include "disk.h"

const string FIRST_PLAY = "First Playthrough";
cszt HISTORY_PAGE = 200;

Book::Book()
{
  OpenMenu();
}

/** @brief Marks the current snapshot with additional description
  */
void Book::SetBookmark(const Properties& Description)
{
  Bookmark& mark = BookSession.CreateBookmark();
  if (mark.Description.empty()) {
    StoryQuery query(*this, BookStory, BookSession, mark.Description);
    for (const string& text : Description.TextValues) {
      query.ExecuteExpression(QUEUE, text);
    }
    CleanWhitespace(mark.Description);
  }
}

void Book::SetBookmark(const string& Description)
{
  Bookmark& mark = BookSession.CreateBookmark();
  if (!Description.empty()) {
    mark.Description = Description;
  }
  CleanWhitespace(mark.Description);
}

void Book::AddDialog(const Dialog& dialog)
{
  Dialogs.push_back(dialog);
  DialogOpen = true;
}

/** @brief Open book of a given title
  *
  * open the file containing the story and feed it to StoryDefinition to parse
  * one keyword block at a time
  */
bool Book::OpenBook(const string& Title)
{
  if (BookOpen && BookTitle == Title) {
    return true;
  }
  CloseBook();

  BookTitle = Title;
  const string& path = STORY_DIR + SLASH + BookTitle;
  BookOpen = OpenStory(path, BookStory);
  BookSession.Reset();
  BookSession.BookName = BookTitle;
  Media.CreateAssets(Assets, BookTitle);

  return BookOpen;
}

bool Book::RedoSnapshot()
{
  // by loading the current move we can move forward if the current move
  // already exists in the snapshots
  cszt next = BookSession.CurrentSnapshot;
  // be careful not to overshoot because that will reset the story
  if (next < BookSession.Snapshots.size()) {
    return LoadSnapshot(next);
  }
  return false;
}

bool Book::UndoSnapshot()
{
  // the current snapshot is the future one, so we need to go back 2 snapshots
  cszt prev = BookSession.CurrentSnapshot - 2;
  // only load the snapshot if it's not the 0th step, which is the start
  // of the book before values existed to be loaded
  if (prev > 0) {
    return LoadSnapshot(prev);
  }
  return false;
}

bool Book::LoadSnapshot(cszt SnapshotIndex)
{
  // before loading, revert to book values
  InitSession(BookStory, BookSession);
  // if not the last snapshot then we're moving on the timeline and can't act
  ActiveBranch = (SnapshotIndex == BookSession.Snapshots.size() - 1);
  return BookSession.LoadSnapshot(SnapshotIndex);
}

bool Book::ShowMenu()
{
  if (!MenuOpen) {
    MenuOpen = true;
    GameSession.AddQueueValue("main:menu");
  }
  return MenuOpen;
}

bool Book::ShowSaveMenu()
{
  MenuOpen = true;
  GameSession.AddQueueValue("game:session");
  return MenuOpen;
}

bool Book::ShowHistoryMenu()
{
  MenuOpen = true;
  GameSession.AddQueueValue("game:history");
  return MenuOpen;
}

bool Book::ShowBookmarkMenu()
{
  MenuOpen = true;
  GameSession.AddQueueValue("game:bookmark");
  return MenuOpen;
}

void Book::Quit()
{
  MenuOpen = SessionOpen = BookOpen = DialogOpen = false;
}

bool Book::HideMenu()
{
  MenuOpen = !SessionOpen;
  return SessionOpen;
}

void Book::CloseBook()
{
  if (SessionOpen) {
    SaveSession();
    SessionOpen = false;
    BookSession.Reset();
  }
  if (BookOpen) {
    BookOpen = false;
    BookStory.Reset();
    Assets.clear();
  }
  ShowMenu();
}

Properties Book::GetBooks()
{
  Properties result;
  result.TextValues = Disk::ListFiles(STORY_DIR);
  return result;
}

bool Book::OpenMenu()
{
  GameSession.Name = "game";
  GameSession.BookName = "menu";
  OpenStory(MENU_DIR, MenuStory);
  InitSession(MenuStory, GameSession);
  return MenuOpen;
}

bool Book::Tick(real DeltaTime)
{
  return Media.Tick(DeltaTime, *this);
}

bool Book::OpenStory(const string& Path,
                     Story& MyStory)
{
  MyStory.Reset();
  vector<string> filenames = Disk::ListFiles(Path, STORY_EXT);
  filenames.push_back(STORY_FILE);

  // go through all *.story files but read story first
  szt i = filenames.size();
  while (i) { // reverse for loop
    const string& filename = filenames[--i];
    string textBlock;
    string buffer;
    File story;
    story.Read(Path + SLASH + filename);

    while (!story.Empty()) {
      story.GetLine(buffer);
      StripComments(buffer);
      if (buffer.empty()) {
        continue;
      }
      // look for a keyword definition or asset definition
      if (buffer.size() > 2
          && buffer[0] == token::Start[token::noun]
          && buffer[1] != token::Start[token::scope]) {
        szt nounPos = FindTokenEnd(buffer, token::noun);
        if (string::npos != nounPos) { // [noun]
          if (!textBlock.empty()) {
            // if it's the second keyword we hit on this run
            // parse the text and start a new run
            MyStory.ParseKeywordDefinition(textBlock);
            textBlock.clear();
          }
          textBlock += buffer;
        } else {
          LOG(buffer + " - malformed noun definition")
        }
      } else if (buffer[0] == token::Start[token::asset]) {
        AddAssetDefinition(buffer);
      } else if (!textBlock.empty()) {
        // we didn't find a keyword, keep adding lines if we already hit one
        textBlock += " "; // replace new lines with spaces
        textBlock += buffer;
      }
    }

    // last keyword definition
    if (!textBlock.empty()) {
      MyStory.ParseKeywordDefinition(textBlock);
    }
  }

  MyStory.Fixate();
  return true;
}

void Book::InitSession(Story& MyStory,
                       Session& MySession)
{
  MySession.BookName = BookTitle;
  MySession.UserValues.clear();
  MySession.AssetStates.clear();
  // initialise reserved keywords
  for (szt i = 0; i < SYSTEM_NOUN_MAX; ++i) {
    const string& name = SystemNounNames[i];
    const Properties& systemValues = MyStory.FindPage(name).PageValues;
    Properties& sysNoun = MySession.UserValues[SystemNounNames[i]];
    sysNoun.AddValues(systemValues);
    // and set the pointers to system nouns for easy (read only) access
    MySession.SystemNouns[i] = &sysNoun;
    if (i == systemQueue) {
      // and read-write access for the queue
      MySession.QueueNoun = &sysNoun;
    }
  }
  // record these system values, despite them not being changed
  MySession.ValuesChanged = true;
}

/** @brief AddAssetDefinition
  *
  * This saves the asset definition which is parsed later by the reader
  */
bool Book::AddAssetDefinition(const string& StoryText)
{
  string text = GetCleanWhitespace(StoryText);
  // get the strings [$name=def]
  // name can't be empty so we can start looking for = from position 2
  cszt defPos = FindTokenEnd(text, token::assign, 2);
  // definition can't be empty either
  if (defPos + 1 < text.size()) {
    const string& assetName = CutString(text, 1, defPos);
    const string& assetDefinition = CutString(text, defPos + 1);
    Assets.push_back(string_pair(assetName, assetDefinition));
    return true;
  }
  return false;
}

/** @brief fill the result with nouns that should remain highlighted
  */
void Book::GetStoryNouns(Properties& Result)
{
  Result += BookSession.GetSystemValues(systemPlace);
  Result += BookSession.GetSystemValues(systemExits);
  Result += BookSession.GetSystemValues(systemNouns);
}

/** @brief return true if the queue has pending actions
  */
bool Book::IsActionQueueEmpty()
{
  if (MenuOpen) {
    return GameSession.GetSystemValues(systemQueue).IsEmpty();
  } else {
    return BookSession.GetSystemValues(systemQueue).IsEmpty();
  }
}

/** @brief Get list of verbs of the noun
  * \return the verbs which do not fail their top level conditions
  */
const string Book::GetStoryVerbs(const string& Noun)
{
  string text; // discarded
  StoryQuery query(*this, BookStory, BookSession, text);
  return query.GetVerbs(Noun).PrintKeywordList("\n");
}

/** @brief Get list of verbs of the noun
  * \return the verbs which do not fail their top level conditions
  */
const string Book::GetMenuVerbs(const string& Noun)
{
  string text; // discarded
  StoryQuery query(*this, MenuStory, GameSession, text);
  return query.GetVerbs(Noun).PrintKeywordList("\n");
}
Properties& Book::GetMenuValues(const string& Noun)
{
  string text; // discarded
  StoryQuery query(*this, MenuStory, GameSession, text);
  return query.GetUserValues(Noun);
}

/** @brief If the first element of the pair is a complete action
  * rearrange to fill the proper fields
  */
bool Book::GetChoice(string_pair& Choice) const
{
  // is the first part of the pair a noun:verb?
  szt scopePos = FindTokenEnd(Choice.X, token::scope);
  if (scopePos != string::npos) {
    const string& verb = CutString(Choice.X, scopePos + 1, Choice.X.size());
    const string& noun = CutString(Choice.X, 0, scopePos);
    // rearrange the noun:verb,value into proper places
    if (Choice.Y.empty()) {
      Choice.X = noun;
    } else {
      Choice.X = noun + Choice.Y;
    }
    Choice.Y = verb;
    // noun=value,verb
    return true;
  }
  return false;
}

void Book::SetStoryAction(const string_pair& Choice)
{
  SetAction(Choice, BookSession);
}

/** @brief Add the action to the queue for later execution
  */
void Book::SetAction(const string_pair& Choice,
                     Session& MySession)
{
  //check for value assignment first
  cszt assignPos = FindTokenEnd(Choice.X, token::assign);
  string action;
  if (assignPos != string::npos) {
    // parse [noun=value:verb] which is in pair(noun=value, verb) form
    action += CutString(Choice.X, 0, assignPos);
    action += ":";
    action += Choice.Y;
    // queue up the extra action of setting the value
    MySession.AddQueueValue(Choice.X);
  } else {
    // regular noun:verb only
    action += Choice.X;
    action += ":";
    action += Choice.Y;
  }
  // queue up the noun:verb for execution
  MySession.AddQueueValue(action);
}

void Book::SetMenuAction(const string_pair& Choice)
{
  SetAction(Choice, GameSession);
}

string Book::ProcessStoryQueue()
{
  BookSession.CreateSnapshot();
  return ProcessQueue(BookStory, BookSession);
}

string Book::ProcessMenuQueue()
{
  return ProcessQueue(MenuStory, GameSession);
}

/** @brief Read the book by executing actions on the queue
  * Each value on the queue get's executed one by one
  * and during execution is the only value on the queue
  * at the end of execution the whole queue is cleared
  */
const string Book::ProcessQueue(Story& MyStory,
                                Session& MySession)
{
#ifdef DEVBUILD
  GTrace.clear();
#endif
  Properties& actions = *MySession.QueueNoun;
  string pageText;
  StoryQuery query(*this, MyStory, MySession, pageText);
  cszt safety = 1000; // to stop user generated infinite loops
  szt i = 0;
  // collect all the queue values here to look for system calls
  Properties pool;
  pool.AddValues(actions);

  while (pool.TextValues.size() > i) {
    if (i > safety) {
      LOG(pool.TextValues[i] + " - possible infinite loop, aborting");
      break;
    }
    // only the currently executed call is present during execution
    actions.Reset();
    const string& expression = pool.TextValues[i];
    actions.AddValue(expression);
#ifdef DEVBUILD
    const Properties& calls = MySession.GetSystemValues(systemCalls);
    if (!calls.TextValues.empty()) {
      GTrace += "\nCALLS ";
      for (const string& trace : calls.TextValues) {
        GTrace += trace;
        GTrace += ' ';
      }
      GTrace += "traces:";
    }
#endif
    // call the actions scheduled for every turn
    query.ExecuteExpression(CALLS, CALLS_CONTENTS);
#ifdef DEVBUILD
    GTrace += "\n\n[" + expression + "] trace:";
#endif
    // only one value present in QUEUE during this execution,
    // unless it has been removed by one of the CALLS
    query.ExecuteExpression(QUEUE, QUEUE_CONTENTS);
    // if the execution added more actions, add the to the pool
    pool.AddValues(actions);
    ++i;
  }

  actions.Reset();
  return pageText;
}

/** @brief This returns the text for the quick menu
  */
string Book::GetQuickMenu()
{
  string pageText;
  StoryQuery query(*this, BookStory, BookSession, pageText);
  query.ExecuteExpression(QUICK, QUICK_CONTENTS);
  return pageText + '\n';
}

/** @brief This find a unique name and filename and tries to load it
  */
bool Book::NewSession()
{
  if (BookOpen) {
    const string& path = STORY_DIR + SLASH + BookTitle;
    // get unique filename and session name
    BookSession.Filename = GetFreeSessionFilename(path);
    const vector<string_pair>& namemap = GetSessionNamemap(path);
    BookSession.Name = FIRST_PLAY;
    MakeSessionNameUnique(BookSession.Name, namemap);
    return LoadSession(BookSession.Name);
  }
  return false;
}

/** @brief Loading a non-existent file will start a new game, trying to load
  * an empty session name will continue the last available session
  * It will start a new session if no old ones are available
  */
bool Book::LoadSession(const string& SessionName)
{
  if (BookOpen) {
    if (SessionOpen) {
      SaveSession();
      BookSession.Reset();
    }
    SessionOpen = true;
    InitSession(BookStory, BookSession);
    const string& path = STORY_DIR + SLASH + BookTitle;
    // if no filename use the continue file to find the filename
    if (SessionName.empty()) {
      const string& sessionMap = path + SLASH + SESSION_MAP;
      if (Disk::Exists(sessionMap)) {
        // continue from the first file in the map file
        File continueFile;
        continueFile.Read(sessionMap);
        continueFile.GetLine(BookSession.Filename);
        return BookSession.Load();
      } else {
        NewSession();
      }
    } else {
      // try and load the session if it already exists
      const vector<string_pair>& namemap = GetSessionNamemap(path);
      for (const string_pair& existingName : namemap) {
        if (SessionName == existingName.Y) {
          BookSession.Filename = existingName.X;
          return BookSession.Load();
        }
      }
      // session name not found so it's the first play-through or a new game
      // session already initialised, no loading needed, add start bookmark
      Bookmark& mark = BookSession.CreateBookmark();
      mark.Description = "Story beginning";
      return true;
    }
  }
  return false;
}

/** @brief Return the session names that are stored in the session name map
  */
Properties Book::GetSessions(const string& Title)
{
  Properties result;
  string path = STORY_DIR + SLASH;
  if (Title.empty()) {
    if (BookOpen) {
      path += BookTitle;
    } else {
      // no book open, don't know where to look for sessions
      return result;
    }
  } else {
    path += Title;
  }
  // load the names of sessions from the meta file
  const vector<string_pair>& namemap = GetSessionNamemap(path);
  for (const string_pair& name : namemap) {
    result.AddValue(name.Y);
  }
  return result;
}

/** @brief get a list of pairs of filename and session name from the map file
  */
const vector<string_pair> Book::GetSessionNamemap(const string& Path)
{
  vector<string_pair> namemap;
  const vector<string>& files = Disk::ListFiles(Path, SESSION_EXT, true);
  if (files.empty()) {
    return namemap;
  }
  const string mapFilename = Path + SLASH + SESSION_MAP;
  if (Disk::Exists(mapFilename)) {
    File filemap;
    filemap.Read(mapFilename);
    // read the file and name pairs from the session file but only add
    // the ones that also exist as filename in the folder
    string existingFilename, sessionName;
    while (filemap.GetLine(existingFilename)) {
      filemap.GetLine(sessionName);
      for (const string& filename : files) {
        if (existingFilename == filename) {
          namemap.push_back(string_pair(existingFilename, sessionName));
          break;
        }
      }
    }
  }
  return namemap;
}

/** @brief Mangle the name so it's unique based on the name map
  * \return false if the name was already unique
  */
bool Book::MakeSessionNameUnique(string& Name,
                                 const vector<string_pair>& Namemap)
{
  bool dupeName = true;
  const string originalName = Name;
  szt i = 0;
  // get a unique session name
  while (dupeName) {
    if (i) {
      Name = originalName + IntoString(++i);
    } else {
      ++i;
    }
    dupeName = false;
    for (const string_pair& existingName : Namemap) {
      if (Name == existingName.Y) {
        dupeName = true;
      }
    }
  }
  return i;
}

/** @brief Save current session, trim the excess and rename
  */
bool Book::BranchSession(const string& NewName)
{
  if (SessionOpen) {
    SaveSession();
    BookSession.Trim();
    ActiveBranch = true;
    // create unique names
    const string& path = STORY_DIR + SLASH + BookTitle;
    BookSession.Filename = GetFreeSessionFilename(path);
    const vector<string_pair>& namemap = GetSessionNamemap(path);
    const string& turn = IntoString(BookSession.CurrentSnapshot - 1);
    if (NewName.empty() || NewName == BookSession.Name) {
      BookSession.Name = BookSession.Name + " T" + turn + " branch";
    } else {
      BookSession.Name = NewName;
    }
    MakeSessionNameUnique(BookSession.Name, namemap);
    SaveSession();
    return true;
  }
  return false;
}

bool Book::SaveSession(const string& NewName)
{
  if (SessionOpen) {
    const string& path = STORY_DIR + SLASH + BookTitle + SLASH;
    const string& filename = path + BookSession.Filename + ".session";
    const vector<string_pair>& namemap = GetSessionNamemap(path);
    if (!NewName.empty() && BookSession.Name != NewName) {
      BookSession.Name = NewName;
      MakeSessionNameUnique(BookSession.Name, namemap);
    }
    Disk::Write(filename, BookSession.GetSessionText());
    // write the meta information for all the session files
    string meta = BookSession.Filename;
    meta += '\n';
    meta += BookSession.Name;
    meta += '\n';
    for (const string_pair& name : namemap) {
      if (name.X != BookSession.Filename) {
        meta += name.X;
        meta += '\n';
        meta += name.Y;
        meta += '\n';
      }
    }
    // save this filename as the last used session
    const string& continueFilename = path + SESSION_MAP;
    Disk::Write(continueFilename, meta);
    return true;
  }
  return false;
}

const string Book::GetSessionName()
{
  return BookSession.Name;
}

#ifdef DEVBUILD
const string Book::ShowVariables()
{
  string variables;
  for (const auto& value : BookSession.UserValues) {
    variables += value.first;
    variables +=  ": ";
    variables += value.second.PrintValues();
    variables +=  '\n';
  }
  return variables;
}
#endif

const string Book::GetFreeSessionFilename(const string& Path)
{
  szt i = 1;
  while (Disk::Exists(Path + SLASH + IntoString(i) + SESSION_EXT)) {
    ++i;
  }
  return IntoString(i);
}

void Book::GetSnapshots(Properties& SnapshotItems)
{
  // find the range ending with the desired location
  cszt desiredEnd = max((szt)SnapshotItems.IntValue, HISTORY_PAGE);
  cszt last = min(desiredEnd, BookSession.Snapshots.size());
  cszt first = last > HISTORY_PAGE ? last - HISTORY_PAGE : 1;
  SnapshotItems.IntValue = last;
  // print all the snapshots in range
  for (szt i = first; i < last; ++i) {
    // find the "noun:verb" or bookmark
    const Snapshot& snap = BookSession.Snapshots[i];
    cszt index = snap.QueueIndex;
    const auto it = BookSession.Bookmarks.find(index - 1);
    string entry = IntoString(index) + ". ";
    if (it != BookSession.Bookmarks.end()) {
      // use the bookmark for this history entry
      entry += it->second.Description;
    } else {
      // extract noun and verb
      string_pair action;
      action.X = BookSession.QueueHistory[index - 1];
      GetChoice(action);
      // add as "1. verb noun"
      entry += action.Y + ' ' + action.X;
    }
    if (index == BookSession.CurrentSnapshot - 1) {
      entry += " - present -";
    }
    SnapshotItems.AddValue(entry);
  }
}

void Book::GetBookmarks(Properties& SnapshotItems)
{
  cszt desiredEnd = max((szt)SnapshotItems.IntValue, HISTORY_PAGE);
  cszt last = min(desiredEnd, BookSession.Bookmarks.size());
  cszt first = last > HISTORY_PAGE ? last - HISTORY_PAGE : 1;
  SnapshotItems.IntValue = last;
  szt i = 0;
  // the bookmarks are sorted but not contiguous so we iterate
  // until we pass over the range we want
  for (const auto& mark : BookSession.Bookmarks) {
    if (first > ++i) {
      continue;
    } else if (i > last) {
      break;
    }
    cszt index = mark.first;
    string entry = IntoString(index + 1) + ". " + mark.second.Description;
    if (index == BookSession.CurrentSnapshot - 1) {
      entry += " - present -";
    }
    SnapshotItems.AddValue(entry);
  }
}

bool Book::LoadSnapshot(const string& Description)
{
  // extract the index from the snapshot description
  cszt indexPos = FindCharacter(Description, '.');
  if (indexPos && indexPos != string::npos) {
    const string& indexName = CutString(Description, 0, indexPos);
    cszt index = IntoInt(indexName);
    LoadSnapshot(index);
    return true;
  }
  return false;
}

szt Book::GetCurrentSnapshot()
{
  return BookSession.CurrentSnapshot;
}
