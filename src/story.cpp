#include "story.h"
#include "page.h"
#include "tokens.h"
#include "session.h"
#include "storyquery.h"
#include "properties.h"

Page Story::MissingPage = Page();

Story::~Story()
{
  Reset();
}

void Story::Reset()
{
  Pages.clear();
}

void Story::Fixate()
{
  Patterns.clear();
}

/** @brief ParseKeywordDefinition
  *
  * this expects a single noun definition block
  * to turn into an internal noun definition
  */
bool Story::ParseKeywordDefinition(const string& StoryText)
{
  string text = GetCleanWhitespace(StoryText);
  // break up the expected [noun[pattern]=value]
  sz_pair nounPos = FindToken(text, token::noun); // [] outer
  sz_pair patPos = FindToken(text, token::noun, nounPos.X+1, // []
                             nounPos.Y);
  sz assignPos = FindTokenStart(text, token::assign, nounPos.X+1, // =
                                nounPos.Y);
  // cut the noun name
  csz nounEnd = min(min(nounPos.Y, patPos.X), assignPos);
  const string& noun = CutString(text, nounPos.X+1, nounEnd);
  // and the definition that follows
  string pageText;
  if (nounPos.Y+1 < text.size()) {
    pageText = CutString(text, nounPos.Y+1);
  }

  // check if the keyword contains a pattern name
  if (patPos.X != string::npos) {
    const string& pattern = CutString(text, patPos.X+1, patPos.Y);
    // try find the pattern in the defined patterns
    map<string, string>::iterator it = Patterns.find(pattern);
    // if this is a pattern definition
    if (nounPos.X == patPos.X-1) {
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
        pageText = PrependPattern(noun, pageText, it->first, it->second);
      } else {
        LOG(pattern + " - pattern definition missing, define before using it");
        return false;
      }
    }
  }

  // check if it's already defined
  const auto it = Pages.find(noun);
  if (it != Pages.end()) {
    LOG(noun + " - already defined");
    return false;
  }

  Pages[noun].Parse(pageText);
  if (assignPos != string::npos) {
    ++assignPos; // skip the =
    Pages[noun].PageValues = Properties(CutString(text, assignPos, nounPos.Y));
  }

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
  sz pos = 0;
  // replace the pattern name with the keyword
  while (pos < text.size()) { // size changes along the way
    sz patternPos = text.find(PatternName, pos);
    if (patternPos != string::npos) {
      text.replace(patternPos, PatternName.size(), Keyword);
      pos = patternPos + Keyword.size();
    } else {
      break;
    }
  }

  return text + PageText;
}
