#include "book.h"
#include "page.h"
#include "tokens.h"
#include "storyquery.h"
#include "disk.h"

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
  mark.Description = Description;
  CleanWhitespace(mark.Description);
}

/** @brief AddDialog
  *
  * @todo: document this function
  */
bool Book::AddDialog(const string& Noun)
{
  return true;
}

/** @brief AddInputDialog
  *
  * @todo: document this function
  */
bool Book::AddInputDialog(const string& Noun)
{
  return true;
}

/** @brief Open book of agiven title
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
  const string& path = STORY_DIR + SLASH + BookTitle + SLASH;
  BookOpen = OpenStory(path, BookStory, BookSession);
  BookSession.BookName = BookTitle;
  Media.CreateAssets(Assets, BookTitle);

  return BookOpen;
}

bool Book::NewSession()
{
  const string& path = STORY_DIR + SLASH + BookTitle + SLASH + SESSION_CONTINUE;
  Disk::Delete(path);
  return LoadSession();
}

bool Book::LoadSession(const string& Filename)
{
  if (BookOpen) {
    SessionOpen = true;
    InitSession(BookStory, BookSession);
    const string& path = STORY_DIR + SLASH + BookTitle + SLASH;
    // if no filename use the continue file to find the filename
    if (Filename.empty()) {
      const string& continueFilename = path + SESSION_CONTINUE;
      if (Disk::Exists(continueFilename)) {
        // filename in the continue file
        File continueFile;
        continueFile.Read(continueFilename);
        continueFile.GetLine(BookSession.Filename);
        return BookSession.Load();
      } else {
        // first playthrough or a new game - find first available filename
        BookSession.Filename = GetFreeSessionFilename();
        BookSession.Name = "Playthrough" + BookSession.Filename;
        // session already initialised, no loading needed, add start bookmark
        Bookmark& mark = BookSession.CreateBookmark();
        mark.Description = "Story beginning";
        return true;
      }
    } else {
      BookSession.Filename = Filename;
      return BookSession.Load();
    }
  }
  return false;
}

string Book::GetFreeSessionFilename()
{
  const string& path = STORY_DIR + SLASH + BookTitle + SLASH;
  size_t i = 1;
  while (Disk::Exists(path + IntoString(i) + SESSION_EXT)) {
    ++i;
  }
  return IntoString(i);
}

bool Book::RedoSnapshot()
{
  // by loading the current move we can move forward if the current move
  // already exists in the snapshots
  const size_t next = BookSession.CurrentSnapshot;
  // be careful not to overshoot because that will reset the story
  if (next < BookSession.Snapshots.size()) {
    return LoadSnapshot(next);
  }
  return false;
}

bool Book::UndoSnapshot()
{
  // the current snapshot is the future one, so we need to go back 2 snapshots
  const size_t prev = BookSession.CurrentSnapshot - 2;
  // only load the snapshot if it's not the 0th step, which is the start
  // of the book before values existed to be loaded
  if (prev > 0) {
    return LoadSnapshot(prev);
  }
  return false;
}

bool Book::LoadSnapshot(const size_t SnapshotIndex)
{
  // before loading, revert to book values
  InitSession(BookStory, BookSession);
  // if not the last snapshot then we're moving on the timeline and can't act
  ActiveBranch = (SnapshotIndex == BookSession.Snapshots.size() - 1);
  return BookSession.LoadSnapshot(SnapshotIndex);
}

bool Book::ShowMenu()
{
  MenuOpen = true;
  return MenuOpen;
}

void Book::Quit()
{
  MenuOpen = SessionOpen = BookOpen = false;
}

bool Book::HideMenu()
{
  MenuOpen = !SessionOpen;
  return SessionOpen;
}

bool Book::BranchSession()
{
  if (SessionOpen) {
    SaveSession();
    BookSession.Filename = GetFreeSessionFilename();
    BookSession.Name = BookSession.Name + " branch";
    BookSession.Trim();
    ActiveBranch = true;
    return true;
  }
  return false;
}

bool Book::SaveSession()
{
  if (SessionOpen) {
    const string& path = STORY_DIR + SLASH + BookTitle + SLASH;
    const string& filename = path + BookSession.Filename + ".session";
    Disk::Write(filename, BookSession.GetSessionText());
    // save this filename as the last used session
    const string& continueFilename = path + SESSION_CONTINUE;
    Disk::Write(continueFilename, BookSession.Filename);
    return true;
  }
  return false;
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
  Disk::ListFiles(STORY_DIR, result.TextValues);
  return result;
}

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

  vector<string> files;
  Disk::ListFiles(path, files);
  for (const string& file : files) {
    // find .session files
    const size_t length = file.size();
    if (length > SESSION_EXT.size()) {
      const string& ext = CutString(file, length - SESSION_EXT.size());
      if (ext == SESSION_EXT) {
        const string& name = CutString(file, 0, length - SESSION_EXT.size());
        result.AddValue(name);
      }
    }
  }

  return result;
}

bool Book::OpenMenu()
{
  GameSession.Name = "game";
  GameSession.BookName = "menu";
  return OpenStory(MENU_DIR + SLASH, MenuStory, GameSession);
}

bool Book::Tick(real DeltaTime)
{
  return Media.Tick(DeltaTime, *this);
}

bool Book::OpenStory(const string& Path,
                     Story& MyStory,
                     Session& MySession)
{
  string storyText;
  string buffer;
  File story;
  string filename = Path + "story";
  MyStory.Reset();
  MySession.Reset();

  if (!story.Read(filename)) {
    return false;
  }

  while (!story.Empty()) {
    story.GetLine(buffer);
    StripComments(buffer);
    if (buffer.empty()) {
      continue;
    }

    // look for a keyword definition of asset definition
    if (FindTokenStart(buffer, token::keywordBlockMark) != string::npos) {
      // if it's the second keyword we hit on this run
      if (storyText.empty()) {
        // this is the first keyword we have found
        storyText = buffer;
      } else {
        // parse the text and start a new run
        MyStory.ParseKeywordDefinition(storyText);
        storyText = buffer;
      }
    } else if (FindTokenStart(buffer, token::assetBlockMark) != string::npos) {
      AddAssetDefinition(buffer);
    } else if (!storyText.empty()) {
      // we didn't find a keyword, keep adding lines if we already hit one
      storyText += '\n';
      storyText += buffer;
    }
  }

  // last keyword definition
  if (!storyText.empty()) {
    MyStory.ParseKeywordDefinition(storyText);
  }
  //finished reading the file
  MyStory.Fixate();

  InitSession(MyStory, MySession);

  return true;
}

void Book::InitSession(Story& MyStory,
                       Session& MySession)
{
  MySession.BookName = BookTitle;
  MySession.UserValues.clear();
  // initialise reserved keywords
  for (size_t i = 0; i < SYSTEM_NOUN_MAX; ++i) {
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
  const size_t_pair namePos = FindToken(text, token::expression); // []
  const size_t defPos = FindTokenEnd(text, token::assign, namePos.X+1, // =
                                     namePos.Y);

  if (namePos.Y != string::npos && defPos != string::npos) {
    // get the strings [$name=def]
    const string& assetName = CutString(text, namePos.X+2, defPos);
    const string& assetDefinition = CutString(text, defPos+1, namePos.Y);
    if (!assetName.empty() && !assetDefinition.empty()) {
      Assets.push_back(string_pair(assetName, assetDefinition));
      return true;
    }
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

/** @brief If the first element of the pair is a complete action
  * seperate it and fill the second element and return true
  */
bool Book::GetChoice(string_pair& Choice) const
{
  // is it a noun:verb?
  size_t scopePos = FindTokenEnd(Choice.X, token::scope);
  if (scopePos != string::npos) {
    // rearange the noun:verb into proper places
    Choice.Y = CutString(Choice.X, scopePos+1, Choice.X.size());
    Choice.X = CutString(Choice.X, 0, scopePos);
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
void Book::SetAction(const string_pair& Choice, Session& MySession)
{
  //check for value assignment first
  const size_t assignPos = FindTokenEnd(Choice.X, token::assign);
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
string Book::ProcessQueue(Story& MyStory,
                          Session& MySession)
{
  Properties& actions = *MySession.QueueNoun;
  string pageText;
  StoryQuery query(*this, MyStory, MySession, pageText);
  const size_t safety = 1000; // to stop user generated infite loops
  size_t i = 0;
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
    // call the actions scheduled for every turn
    query.ExecuteExpression(CALLS, CALLS_CONTENTS);
    // only one value present during this call, unless it has been removed
    query.ExecuteExpression(QUEUE, QUEUE_CONTENTS);
    // if the execution added more actions, add the to the pool
    pool.AddValues(actions);
    ++i;
  }

  actions.Reset();
  return pageText + '\n';
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
