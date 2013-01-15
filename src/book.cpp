#include "book.h"
#include "page.h"
#include "tokens.h"
#include "storyquery.h"
#include "disk.h"

/** @brief Open book of agiven title
  *
  * open the file containing the story and feed it to StoryDefinition to parse
  * one keyword block at a time
  */
bool Book::OpenBook(const string& Title)
{
  Assets.clear();
  BookTitle = Title;
  string path = STORY_DIR + SLASH + BookTitle;

  BookSession.Name = "FirstRead";
  BookSession.BookName = BookTitle;

  BookOpen = OpenStory(path, BookStory, BookSession);

  Media.CreateAssets(Assets, BookTitle);

  return BookOpen;
}

bool Book::ShowMenu()
{
  MenuOpen = true;
  return MenuOpen;
}

bool Book::HideMenu()
{
  MenuOpen = !BookOpen;
  return BookOpen;
}

bool Book::CloseBook()
{
  BookOpen = false;
  // do saving here
  return true;
}

Properties Book::GetBooks()
{
  Properties result;

  Disk::ListFiles(STORY_DIR, result.TextValues);

  return result;
}

bool Book::OpenMenu()
{
  GameSession.Name = "game";
  GameSession.BookName = "menu";

  return OpenStory(MENU_DIR, MenuStory, GameSession);
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
  string filename = Path + "/story";

  MyStory.Purge();

  if (!story.Open(filename)) {
    return false;
  }

  while (story.GetLine(buffer)) {
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
      storyText += "\n";
      storyText += buffer;
    }
  }

  // last keyword definition
  if (!storyText.empty()) {
    MyStory.ParseKeywordDefinition(storyText);
  }

  //finished reading the file
  MyStory.Fixate();

  if (!MySession.Load("")) {
    // initialise reserved keywords
    for (size_t i = 0; i < SYSTEM_NOUN_MAX; ++i) {
      const string& name = SystemNounNames[i];
      const Properties& systemValues = MyStory.FindPage(name).PageValues;
      MySession.UserValues[SystemNounNames[i]].AddValues(systemValues);
    }
  }

  MySession.Fixate();

  return true;
}

/** @brief AddAssetDefinition
  *
  * This saves the asset definition which is parsed later by the reader
  */
bool Book::AddAssetDefinition(const string& StoryText)
{
  string text = CleanWhitespace(StoryText);

  const size_t_pair namePos = FindToken(text, token::expression); // []
  const size_t defPos = FindTokenEnd(text, token::assign, namePos.X+1, // =
                                     namePos.Y);
  if (namePos.Y == string::npos || defPos == string::npos) {
    return false;
  }

  // get the strings [$name=def]
  string assetName = CutString(text, namePos.X+2, defPos);
  string assetDefinition = CutString(text, defPos+1, namePos.Y);

  if (!assetName.empty() && !assetDefinition.empty()) {
    Assets.push_back(string_pair(assetName, assetDefinition));
  }

  return true;
}

/** @brief fill the result with nouns that should remain highlighted
  */
void Book::GetStoryNouns(Properties& Result) const
{
  Result += BookSession.GetSystemValues(systemPlace);
  Result += BookSession.GetSystemValues(systemExits);
  Result += BookSession.GetSystemValues(systemNouns);
}

/** @brief return true if the queue has pending actions
  */
bool Book::IsActionQueueEmpty() const
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

void Book::SetAction(const string_pair& Choice, Session& MySession)
{
  // parse [noun=value:verb] which is in pair(noun=value, verb) form
  const size_t assignPos = FindTokenEnd(Choice.X, token::assign);
  string action;
  if (assignPos != string::npos) {
    action += CutString(Choice.X, 0, assignPos);
    action += ":";
    action += Choice.Y;
    MySession.GetSystemValues(systemQueue).AddValue(Choice.X);
    MySession.GetSystemValues(systemQueue).AddValue(action);
  } else {
    action += Choice.X;
    action += ":";
    action += Choice.Y;
  }
  MySession.GetSystemValues(systemQueue).AddValue(action);
}

void Book::SetMenuAction(const string_pair& Choice)
{
  SetAction(Choice, GameSession);
}

string Book::ExecuteStoryAction()
{
  return ExecuteAction(BookStory, BookSession);
}

string Book::ExecuteMenuAction()
{
  return ExecuteAction(MenuStory, GameSession);
}

/** @brief Read the book by executing actions on the queue
  * Each value on the queue get's executed one by one
  * and during execution is the only value on the queue
  * at the end of execution the whole queue is cleared
  */
string Book::ExecuteAction(Story& MyStory,
                           Session& MySession)
{
  Properties& actions = MySession.GetSystemValues(systemQueue);

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
  MySession.ValuesChanged = true;

  return pageText + "\n";
}

/** @brief This returns the text for the quick menu
  */
string Book::GetQuickMenu()
{
  string pageText;

  StoryQuery query(*this, BookStory, BookSession, pageText);
  query.ExecuteExpression(QUICK, QUICK_CONTENTS);

  return pageText + "\n";
}
