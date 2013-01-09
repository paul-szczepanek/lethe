#include "story.h"
#include "page.h"
#include "tokens.h"
#include "session.h"
#include "storyquery.h"
#include "properties.h"

Page Story::MissingPage = Page("");

Story::Story()
{
  //ctor
}

Story::~Story()
{
  Purge();
}

void Story::Purge()
{
  page_it it = Pages.begin();
  while (it != Pages.end()) {
    delete it->second;
    ++it;
  }
  Pages.clear();
}

void Story::Fixate()
{
  Patterns.clear();
}

/** @brief ParseKeywordDefinition
  *
  * this expects a single keyword definition block
  * to turn into an internal keyword definition
  */
bool Story::ParseKeywordDefinition(const string& StoryText)
{
  string text = CleanWhitespace(StoryText);

  size_t_pair defPos = FindToken(text, token::expression); // []
  size_t_pair keyPos = FindToken(text, token::keyword, defPos.X+1, // <>
                                 defPos.Y);
  size_t_pair patPos = FindToken(text, token::expression, defPos.X+1, // []
                                 defPos.Y);
  size_t_pair valuesPos = FindToken(text, token::assign, defPos.X+1, // =
                                    defPos.Y);

  // cut the text inside <>
  string keyword = cutString(text, keyPos.X+1, keyPos.Y);
  // and the definition that follows
  string pageText = cutString(text, defPos.Y+1);

  // check if the keyword contains a pattern name
  if (patPos.X != string::npos) {
    const string pattern = cutString(text, patPos.X+1, patPos.Y);
    // try find the pattern in the defined patterns
    map<string, string>::iterator it = Patterns.find(pattern);

    // if this is a pattern definition
    if (keyPos.X == patPos.X-1) {
      // add a new pattern definition
      if (it == Patterns.end()) {
        Patterns[pattern] = pageText;
        return true;
      } else {
        LOG(pattern + " - pattern already defined");
        return false;
      }
    } else { // otherwise this is a keyword definition
      // append pattern text above page text
      if (it != Patterns.end()) {
        // exclude the pattern name from the keyword
        keyword = cutString(text, keyPos.X+1, patPos.X);
        pageText = PrependPattern(keyword, pageText, it->first, it->second);
      } else {
        LOG(pattern + " - pattern definition missing, define before using it");
        return false;
      }
    }
  }

  // check if it's already defined
  page_it it = Pages.find(keyword);
  if (it != Pages.end()) {
    LOG(keyword + " - already defined");
    return false;
  }

  // parsing happens in the constructor
  Page* page = new Page(pageText);

  if (valuesPos.X != string::npos) {
    page->PageValues = Properties(cutString(text, valuesPos.X+1, defPos.Y));
  }

  Pages[keyword] = page; // parsing done in the constructor

  return true;
}

/** @brief Prependa a Pattern definition to a keyword definition
  *
  * It will replace occurences of the pattern keyword with the real keyword
  */
string Story::PrependPattern(const string& Keyword,
                             const string& PageText,
                             const string& PatternName,
                             const string& PatternText)
{
  string text = PatternText;
  size_t pos = 0;

  // replace the pattern name with the keyword
  while (pos < text.size()) { // size changes along the way
    size_t patternPos = text.find(PatternName, pos);
    if (patternPos != string::npos) {
      text.replace(patternPos, PatternName.size(), Keyword);
      pos = patternPos + Keyword.size();
    } else {
      break;
    }
  }

  return text + PageText;
}

string Story::Read(Session& Progress,
                   const string& Noun,
                   const string& VerbName)
{
  string pageText;
  StoryQuery query(Progress, pageText, *this);

  const Page& page = FindPage(Noun);
  const Verb& verb = page.GetVerb(VerbName);

  query.ExecuteBlock(Noun, verb.BlockTree);

  return query.Text;
}

string Story::Start(Session& Progress)
{
  string pageText;
  StoryQuery query(Progress, pageText, *this);

  query.ExecuteExpression(BEGIN, BEGIN_CONTENTS);

  return query.Text;
}

string Story::QuickMenu(Session& Progress)
{
  string pageText;
  StoryQuery query(Progress, pageText, *this);

  query.ExecuteExpression(QUICK, QUICK_CONTENTS);

  return query.Text;
}

/** \brief get list of active verbs
 *
 * \return array of verbs that pass their top level condition
 */
vector<string> Story::GetVerbs(Session& Progress,
                               const string& Noun)
{
  string text; // discarded
  StoryQuery query(Progress, text, *this);
  vector<string> verbs;
  const Page& page = FindPage(Noun);

  for (size_t i = 0, for_size = page.Verbs.size(); i < for_size; ++i) {
    const Verb& verb = page.Verbs[i];
    if (!verb.VisualName.empty()) {
      if (query.ExecuteExpression(Noun, verb.BlockTree.Expression)) {
        verbs.push_back(verb.VisualName);
      }
    }
  }

  return verbs;
}
