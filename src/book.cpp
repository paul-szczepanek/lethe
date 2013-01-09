#include "book.h"
#include "page.h"
#include "tokens.h"

/** @brief Open book of agiven title
  *
  * open the file containing the story and feed it to StoryDefinition to parse
  * one keyword block at a time
  */
bool Book::Open(const string& Title)
{
  BookTitle = Title;
  string filename = STORY_DIR + BookTitle + "/story";

  string storyText;
  string buffer;
  ifstream story;

  Assets.clear();
  StoryDefinition.Purge();

  story.open(filename.c_str());

  if (!story.is_open()) {
    cout << filename << " - missing file" << endl;
    return false;
  }

  while (!story.eof()) {
    getline(story, buffer);
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
        StoryDefinition.ParseKeywordDefinition(storyText);
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
    StoryDefinition.ParseKeywordDefinition(storyText);
  }

  //finished reading the file
  story.close();
  StoryDefinition.Fixate();

  // read progress here
  Progress.Name = "FirstRead";
  Progress.BookName = Title;

  // initialise reserved keywords
  Progress.UserValues[NOUNS] = StoryDefinition.FindPage(NOUNS).PageValues;
  Progress.UserValues[EXITS] = StoryDefinition.FindPage(EXITS).PageValues;
  Progress.UserValues[PLACE] = StoryDefinition.FindPage(PLACE).PageValues;
  Progress.UserValues[CALLS] = StoryDefinition.FindPage(CALLS).PageValues;
  // we don't need BEGIN since it never changes

  Progress.Load("");

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
  string assetName = cutString(text, namePos.X+2, defPos);
  string assetDefinition = cutString(text, defPos+1, namePos.Y);

  if (!assetName.empty() && !assetDefinition.empty()) {
    Assets.push_back(string_pair(assetName, assetDefinition));
  }

  return true;
}



/** @brief ValidateKeywords
  *
  * @todo: document this function
  */
void Book::GetNouns(Properties& Result)
{
  Progress.GetUserValues(PLACE, Result);
  Progress.GetUserValues(EXITS, Result);
  Progress.GetUserValues(NOUNS, Result);
}


/** @brief Get list of verbs of the noun
  *
  * return the verbs which do not fail their top level conditions
  * \return string with verbs as keywords
  */
string Book::GetVerbList(const string& Noun)
{
  string Text;
  vector<string> Verbs = StoryDefinition.GetVerbs(Progress, Noun);
  for (size_t i = 0, for_size = Verbs.size(); i < for_size; ++i) {
    if (i) {
      Text += "\n- <";
    } else {
      Text += "- <";
    }
    Text += Verbs[i];
    Text += ">";
  }

  return Text;
}

/** @brief Get list of verbs of the noun
  *
  * return the verbs which do not fail their top level conditions
  * \return string with verbs as keywords
  */
bool Book::GetChoice(string_pair& Choice)
{
  // is it a noun:verb?
  size_t scopePos = FindTokenEnd(Choice.X, token::scope);
  if (scopePos != string::npos) {
    // rearange the noun:verb into proper places
    Choice.Y = cutString(Choice.X, scopePos+1, Choice.X.size());
    Choice.X = cutString(Choice.X, 0, scopePos);
    return true;
  }
  return false;
}

/** @brief Read
  *
  * this is what the reader sends to the book to get the resulting text
  */
string Book::Read(const string_pair& Choice)
{
  const string pageSource = StoryDefinition.Read(Progress, Choice.X, Choice.Y);

  return pageSource + "\n";
}

/** @brief Read the start of the book
  */
string Book::Start()
{
  const string pageSource = StoryDefinition.Start(Progress);

  return pageSource + "\n";
}

/** @brief Read the start of the book
  */
string Book::QuickMenu()
{
  const string pageSource = StoryDefinition.QuickMenu(Progress);

  return pageSource + "\n";
}
