#include "book.h"
#include "tokens.h"

/** @brief Open book of agiven title
  *
  * open the file containing the story and feed it to StoryDefinition to parse
  * one keyword block at a time
  */
bool Book::Open(string Title)
{
  string filename = STORY_DIR + Title + "/story";

  string storyText;
  string buffer;
  ifstream story;

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
      StoryDefinition.AddAssetDefinition(buffer);
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

  // read progress here
  Progress.Name = "FirstRead";
  Progress.BookName = Title;

  return true;
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
    Text = Text + "\n- <" + Verbs[i] + ">";
  }
  return Text;
}

/** @brief Read
  *
  * this is what the reader sends to the book to get the resulting text
  */
string Book::Read(const string& Noun,
                  const string& VerbName)
{
  const string pageSource = StoryDefinition.Read(Progress, Noun, VerbName);

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
