#include "book.h"
#include "tokens.h"
#include "story.h"
#include "session.h"

Book::Book()
{
  StoryDefinition = new Story();
  //ctor
}

Book::~Book()
{
  delete StoryDefinition;
  //dtor
}

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

    // look for a keyword definition
    if (FindTokenStart(buffer, token::keywordBlockMark) != string::npos) {
      // if it's the second keyword we hit on this run
      if (storyText.empty()) {
        // this is the first keyword we have found
        storyText = buffer;
      } else {
        // parse the text and start a new run
        StoryDefinition->ParseKeywordDefinition(storyText);
        storyText = buffer;
      }
    } else if (!storyText.empty()) {
      // we didn't find a keyword, keep adding lines if we already hit one
      storyText += "\n" + buffer;
    }
  }

  // last keyword definition
  if (!storyText.empty()) {
    StoryDefinition->ParseKeywordDefinition(storyText);
  }

  //finished reading the file
  story.close();

  return true;
}

/** @brief Get list of verbs of the noun
  *
  * return the verbs which do not fail their top level conditions
  * \return string with verbs as keywords
  */
string Book::GetVerbList(Session& Progress,
                         const string& Noun)
{
  string Text;
  vector<string> Verbs = StoryDefinition->GetVerbs(Progress, Noun);
  for (size_t i = 0, for_size = Verbs.size(); i < for_size; ++i) {
    Text = Text + "\n- <" + Verbs[i] + ">";
  }
  return Text;
}

/** @brief Same as above except parses the Progress first
  *
  */
string Book::GetVerbList(const string& Progress,
                         const string& Noun)
{
  Session tempSession;
  tempSession.SetUserValues(Progress);

  return GetVerbList(tempSession, Noun);
}

/** @brief Read
  *
  * this is what the reader sends to the book to get the resulting text
  */
string Book::Read(Session& Progress,
                  const string& Noun,
                  const string& VerbName)
{
  const string pageSource = StoryDefinition->Read(Progress, Noun, VerbName);

  return pageSource + "\n";
}

/** @brief Same as above except parses the Progress first
  *
  */
string Book::Read(string& Progress,
                  const string& Noun,
                  const string& VerbName)
{
  Session tempSession;
  tempSession.SetUserValues(Progress);

  const string pageSource = StoryDefinition->Read(tempSession, Noun, VerbName);

  Progress = tempSession.GetUserValues();
  return pageSource + "\n";
}
