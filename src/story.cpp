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
  szt_pair patPos = FindToken(text, token::noun, nounPos.X+1, // []
                              nounPos.Y);
  szt assignPos = FindTokenStart(text, token::assign, nounPos.X+1, // =
                                 nounPos.Y);
  // cut the noun name
  cszt nounEnd = min(min(nounPos.Y, patPos.X), assignPos);
  const string& noun = CutString(text, nounPos.X+1, nounEnd);

  // and the definition that follows
  string pageText;
  if (nounPos.Y+1 < text.size()) {
    // check if the keyword contains a pattern name
    if (patPos.X != string::npos) {
      // if this is a pattern definition
      if (nounPos.X == patPos.X-1) {
        const string& pattern = CutString(text, patPos.X+1, patPos.Y);
        // try find the pattern in the defined patterns
        map<string, string>::iterator it = Patterns.find(pattern);
        // add a new pattern definition
        if (it == Patterns.end()) {
          Patterns[pattern] = CutString(text, nounPos.Y+1);
          return true;
        } else {
          LOG(pattern + " - pattern already defined");
          return false;
        }
      } else {
        // otherwise this is a keyword definition that also uses a pattern
        LOG(noun + " - noun needs to start with [noun] before any patterns");
      }
    } else {
      pageText = CutString(text, nounPos.Y+1);
    }
  }

  // check if it's already defined
  const auto it = Pages.find(noun);
  if (it != Pages.end()) {
    LOG(noun + " - already defined");
    return false;
  }

  pageText = ApplyPatterns(noun, pageText);

  Pages[noun].Parse(pageText);

  if (assignPos != string::npos) {
    ++assignPos; // skip the =
    Pages[noun].PageValues = Properties(CutString(text, assignPos, nounPos.Y));
  }

  return true;
}

/** @brief Appends a pattern to the keyword definition, replacing pattern name
  *
  * It will replace occurrences of the pattern keyword with the real keyword
  */
string Story::ApplyPatterns(const string& Keyword,
                            const string& PageText)
{
  string text;
  szt pos = 0;

  while (pos < PageText.size()) {
    // break up the expected [[pattern]] or [noun[pattern]]
    szt_pair nounPos = FindToken(PageText, token::noun, pos); // [] outer
    szt_pair patPos;
    if (nounPos.X != string::npos) {
      patPos = FindToken(PageText, token::noun, nounPos.X + 1, nounPos.Y);
    } else {
      // nothing to apply, no more patterns, paste the rest of text and exit
      text += CutString(PageText, pos);
      break;
    }

    // append text before the pattern
    text += CutString(PageText, pos, nounPos.X);

    // do the keyword substitution on the pattern and append it
    if (patPos.X != string::npos) {
      const string& pattern = CutString(PageText, patPos.X + 1, patPos.Y);
      // try find the pattern in the defined patterns
      map<string, string>::iterator it = Patterns.find(pattern);
      if (it != Patterns.end()) {
        text += PreparePattern(Keyword, it->first, it->second);
      } else {
        LOG(pattern + " - pattern definition missing");
      }
    } else {
      // if it's not a pattern just copy the text
      text += CutString(PageText, nounPos.X, nounPos.Y + 1);
    }

    pos = nounPos.Y+1;
  }

  return text;
}

/** @brief Prepends a Pattern definition to a keyword definition
  *
  * It will replace occurrences of the pattern keyword with the real keyword
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
