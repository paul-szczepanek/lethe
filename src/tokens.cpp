#include "tokens.h"

/** @brief removes all whitespace around tokens and collapses multiple newlines
  */
void CleanWhitespace(string& Text)
{
  string clean;
  clean.reserve(Text.size());
  sz pos = 0;
  sz whiteCount = 0;
  csz length = Text.size();
  char textBlockToken = token::Start[token::textBlock];
  char keywordStart = token::Start[token::keyword];
  char keywordEnd = token::End[token::keyword];
  bool inTextBlock = false;
  bool inKeywordBlock = false;
  bool ignoreWhite = false;

  --pos;
  while (++pos < length) {
    char c = Text[pos];

    // handle the special meaning the character may have
    if (c == '\t' || c == ' ') {
      if (inTextBlock && !inKeywordBlock) {
        clean += ' ';
      } else if (!ignoreWhite) {
        // so we can decided later whether to discard it or not
        ++whiteCount;
      }
    } else if (c == '\\') {
      // recover whitespace
      while(whiteCount) {
        clean += ' ';
        --whiteCount;
      }
      // anything after the escape character is just copied over and ignored
      if (++pos == length) {
        break;
      }
      // check for a newline
      if (Text[pos] == 'n') {
        clean += '\n';
      } else {
        // copy the escaped character ignoring any special meaning
        clean += "\\";
        clean += Text[pos];
      }
    } else if (c == '\n') {
      // collapse multiple newlines
      while (++pos < length && Text[pos] == '\n');
      --pos;
      // if the newline is after plain text, treat it as space
      if (inTextBlock && !inKeywordBlock) {
        clean += ' ';
      }
    } else if (c == textBlockToken) { // "
      whiteCount = 0;
      clean += c;
      if (inTextBlock) {
        inTextBlock = ignoreWhite = false;
        // special case where two blocks come after each other
        sz ignore = pos;
        while (++ignore < length) {
          if (Text[ignore] != '\n' && Text[ignore] != ' '
              && Text[ignore] != '\t') {
            if (Text[ignore] == textBlockToken) {
              pos = ignore;
              // we 're back in a textblock
              inTextBlock = ignoreWhite = true;
              // replace the closing " with a newline
              clean[clean.size() - 1] = '\n';
            }
            break;
          }
        }
      } else {
        inTextBlock = ignoreWhite = true;
      }
    } else if (c == keywordStart) { // <
      whiteCount = 0;
      inKeywordBlock = true;
      ignoreWhite = true;
      clean += c;
    } else if (c == keywordEnd) { // >
      whiteCount = 0;
      inKeywordBlock = false;
      ignoreWhite = false;
      clean += c;
    } else {
      // we have hit a regular character
      ignoreWhite = false;
      // unless it's a whitespace remover
      if (!inTextBlock || inKeywordBlock) {
        for (sz i = 0; i < token::NUM_SPACE_REMOVERS; ++i) {
          if (c == token::WhitespaceRemovers[i]) {
            whiteCount = 0;
            ignoreWhite = true;
            break;
          }
        }
      }
      // recover whitespace
      while(whiteCount) {
        clean += ' ';
        --whiteCount;
      }
      clean += c;
    }
  }

  Text.swap(clean);
}

/** @brief Return string with no escape characters in it
  */
string CleanEscapeCharacters(const string& Text)
{
  string clean;
  clean.reserve(Text.size());
  sz pos = 0;
  csz length = Text.size();
  while (pos < length) {
    char c = Text[pos];
    if (c != '\\') {
      clean += c;
    }
    ++pos;
  }
  return clean;
}

/** @brief Find Token depending on its type, paired tokens pretend the text
  * in between is part of the token for returning position
  *
  * \return a pair of begin and end positions of the token
  */
sz_pair FindToken(const string& Text,
                  token::tokenName TokenName,
                  sz Start,
                  sz End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }
  sz pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  sz type = token::Type[TokenName];

  if ((type & token::isPaired) && (type & token::isWide)) {
    // return if matching pair found like ** **
    while (pos < End - 1) {
      if (IsSpecial(Text, pos, tokenA) && IsSpecial(Text, pos+1, tokenB)) {
        sz match = pos;
        while (++match < End - 1) {
          if (IsSpecial(Text, match, tokenB)
              && IsSpecial(Text, match+1, tokenB)) {
            return sz_pair(pos, match + 1);
          }
        }
        break;
      }
      ++pos;
    }
  } else if (type & token::isPaired) {
    // return if matching pair found like [ ]
    while (pos < End) {
      if (IsSpecial(Text, pos, tokenA)) {
        sz match = pos;
        sz matchCount = 1;
        while (++match < End) {
          if (IsSpecial(Text, match, tokenA)) {
            ++matchCount;
          }
          if (IsSpecial(Text, match, tokenB) && (--matchCount == 0)) {
            return sz_pair(pos, match);
          }
        }
        break;
      }
      ++pos;
    }
  } else if (type & token::isWide) {
    // if it's two char token check for other char and return if found
    while (pos < End - 1) {
      if (IsSpecial(Text, pos, tokenA) && IsSpecial(Text, pos+1, tokenB)) {
        return sz_pair(pos, pos+1);
      }
      ++pos;
    }
  } else {
    // if it's a single char token return when found
    while (pos < End) {
      if (IsSpecial(Text, pos, tokenA)) {
        return sz_pair(pos, pos);
      }
      ++pos;
    }
  }

  // nothing found
  return sz_pair(string::npos, string::npos);
}

/** @brief FindToken
  *
  * \return this version only returns the start of the token
  */
sz FindTokenStart(const string& Text,
                  token::tokenName TokenName,
                  sz Start,
                  sz End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }
  sz pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  sz type = token::Type[TokenName];

  while (pos < End) {
    if (IsSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        // if it's two char token check for other char and return if found
        if (IsSpecial(Text, pos+1, tokenB)) {
          return pos;
        }
      } else if (type & token::isPaired) {
        // return if matching pair found like [ ]
        sz match = pos;
        sz matchCount = 1;

        while (++match <= End) {
          if (IsSpecial(Text, match, tokenA)) {
            ++matchCount;
          }
          if (IsSpecial(Text, match, tokenB) && (--matchCount == 0)) {
            return pos;
          }
        }
        break;
      } else {
        // if it's a single char token return when found
        return pos;
      }
    }
    ++pos;
  }

  // nothing found
  return string::npos;
}

/** @brief FindToken
  *
  * \return the end position of a token
  */
sz FindTokenEnd(const string& Text,
                token::tokenName TokenName,
                sz Start,
                sz End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }
  sz pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  sz type = token::Type[TokenName];

  while (pos < End) {
    if (IsSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        ++pos;
        if ((pos < End) && IsSpecial(Text, pos, tokenB)) {
          return pos;
        } else {
          break;
        }
      } else if (type & token::isPaired) {
        sz matchCount = 1;
        while (++pos < End) {
          if (IsSpecial(Text, pos, tokenA)) {
            ++matchCount;
          }
          if (IsSpecial(Text, pos, tokenB) && (--matchCount == 0)) {
            return pos;
          }
        }
        break;
      } else {
        return pos;
      }
    }
    ++pos;
  }

  return string::npos;
}

/** @brief Removes // and everything after that from the passed in string
  */
void StripComments(string& Text)
{
  csz length = Text.size();
  sz pos = 0;

  while (pos < length) {
    if (IsSpecial(Text, pos, token::Start[token::comment]) &&
        (pos+1 < length) && Text[pos+1] == token::End[token::comment]) {
      // ignore the rest of the line
      Text = Text.substr(0, pos);
      break;
    }
    ++pos;
  }
}
