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
  // break up the expected [[pattern]] or [noun=value]
  szt_pair nounPos = FindToken(text, token::noun); // [] outer
  szt_pair patPos = FindToken(text, token::noun, nounPos.X + 1, nounPos.Y); // []
  szt assignPos = FindTokenStart(text, token::assign, nounPos.X + 1, nounPos.Y);

  // cut the noun name
  cszt nounEnd = min(min(nounPos.Y, patPos.X), assignPos);
  const string& noun = CutString(text, nounPos.X + 1, nounEnd);

  // check if it's already defined
  if (Pages.find(noun) != Pages.end()) {
    LOG(noun + " - already defined");
    return false;
  }

  if (assignPos != string::npos) {
    // if it's not a pattern apply the initial values if present
    ++assignPos; // skip the =
    Pages[noun].PageValues = Properties(CutString(text, assignPos, nounPos.Y));
  }

  string pageText;

  // check if the keyword contains a pattern name
  if (patPos.X != string::npos) {
    // if this is a pattern definition
    if (nounPos.X == patPos.X - 1) {
      const string& pattern = CutString(text, patPos.X + 1, patPos.Y);
      // try find the pattern in the defined patterns
      map<string, string>::iterator it = Patterns.find(pattern);
      // add a new pattern definition
      if (it == Patterns.end()) {
        Patterns[pattern] = CutString(text, nounPos.Y + 1);
        return true;
      } else {
        LOG(pattern + " - pattern already defined");
        return false;
      }
    } else {
      // this is a noun using patterns
      szt_pair curPos(patPos.X, patPos.Y);

      // there can be multiple patterns defined inn sequence [p1, p2, p3]
      while (curPos.X < patPos.Y) {
        // pattern name ends in ] or , if there are multiple pattern names
        curPos.Y = min(FindTokenStart(text, token::separator, curPos.X + 1, patPos.Y),
                       curPos.Y);

        const string& pattern = CutString(text, curPos.X + 1, curPos.Y);
        // try find the pattern in the defined patterns
        map<string, string>::iterator it = Patterns.find(pattern);
        if (it != Patterns.end()) {
          pageText += PreparePattern(noun, it->first, it->second);
        } else {
          LOG(pattern + " - pattern definition missing");
        }

        // move onto the next pattern name
        curPos.X = FindTokenStart(text, token::separator, curPos.Y + 1, patPos.Y);
      }
    }
  }

  // append the rest of the noun definition
  pageText += CutString(text, nounPos.Y + 1);

  Pages[noun].Parse(pageText);

  return true;
}

/** @brief returns the patterns text with occurrences of the pattern keyword
  * replaced with with the real keyword ready to be prepended to the noun definition
  */
string Story::PreparePattern(const string& Keyword,
                             const string& PatternName,
                             const string& PatternText)
{
  string text = PatternText;
  szt pos = 0;
  // replace the pattern name with the keyword
  while (pos < text.size()) { // size changes along the way
    szt patternPos = text.find(PatternName, pos);
    if (patternPos != string::npos) {
      text.replace(patternPos, PatternName.size(), Keyword);
      pos = patternPos + Keyword.size();
    } else {
      break;
    }
  }

  return text;
}
