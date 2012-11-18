#include "tokens.h"

/** @brief CleanWhitespace
  *
  * removes all whitespace around tokens and collapses multiple newlines
  */
string CleanWhitespace(const string& Text)
{
  string clean;

  clean.reserve(Text.size());

  size_t pos = 0;
  size_t expCount = 0;
  size_t whiteCount = 0;
  size_t newlineCount = 0;
  const size_t length = Text.size();

  char expStart = token::Start[token::expression];
  char expEnd = token::End[token::expression];

  bool ignoreWhiteAhead = true;
  bool ignoreWhiteBehind = false;
  bool escaped = false;

  while (pos < length) {
    char c = Text[pos];

    // ignore illegal chars
    if (c == '\t') {
      c = ' ';
    }

    // anything after the escape character is just copied over and ignored
    // except a newline
    if (c == '\\') {
      if (++pos < length) {
        // check for a newline
        if (Text[pos] == 'n') {
          // add an escaped newline
          clean = clean + '\n';
          ++pos;
          continue;
        }
      } else {
        break;
      }
      escaped = true; // the escape character \ does get copied
    } else { // handle the special meaning the character may have
      escaped = false;

      // should we remove all whitespace around this character?
      if (expCount > 0) {
        for (size_t i = 0; i < token::NUM_EXPRESSION_SPACE_REMOVERS; ++i) {
          if (c == token::ExpressionWhitespaceRemovers[i]) {
            ignoreWhiteAhead = ignoreWhiteBehind = true;
            break;
          }
        }
      } else {
        for (size_t i = 0; i < token::NUM_SPACE_REMOVERS; ++i) {
          if (c == token::WhitespaceRemovers[i]) {
            ignoreWhiteAhead = ignoreWhiteBehind = true;
            break;
          }
        }
      }

      // collapse multiple newlines
      if (c == '\n') {
        while ((pos < length - 1) && ('\n' == Text[pos + 1])) {
          ++pos;
        }
        // if we're already ignoring whitespace ignore this newline
        if (!ignoreWhiteAhead) {
          ++newlineCount;
        }
        // newline are restored later
        c = ' ';
        ignoreWhiteAhead = ignoreWhiteBehind = true;
      }

      if (c == expStart) {
        ++expCount;
      } else if (c == expEnd) {
        --expCount;
      }
    }

    // merge whitespaces
    if (c == ' ') {
      ++whiteCount;

      while ((pos < length - 1) && (Text[pos + 1] == ' ')) {
        ++pos;
        ++whiteCount;
      }
    } else {
      // deal with accumulated whitespaces
      if (ignoreWhiteBehind) {
        ignoreWhiteBehind = false;

        // add a space for the new line of plain text
        if (newlineCount) {
          clean += ' ';
        }
        whiteCount = 0;
        newlineCount = 0;
        // discard trailing whitespaces as well
        while ((pos < length - 1) && (Text[pos + 1] == ' ')) {
          ++pos;
        }
      } else {
        ignoreWhiteAhead = false;
        if (whiteCount) { // restore whitespace
          do {
            clean += ' ';
          } while (--whiteCount);
        } else if (newlineCount) {
          do {
            clean += '\n';
          } while (--newlineCount);
        }
      }

      clean += c;
    }

    // copy the escaped character ignoring any special meaning
    if (escaped) {
      if (++pos < length) {
        clean += Text[pos];
      }
    }

    ++pos;
  }

  return clean;
}

/** @brief FindToken
  *
  * \return a pair of begin and end positions of the token
  */
size_t_pair FindToken(const string& Text,
                      token::tokenName TokenName,
                      size_t Start,
                      size_t End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }

  size_t pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  uint type = token::Type[TokenName];

  while (pos < End) {
    if (isSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        // if it's two char token check for other char and return if found
        if (isSpecial(Text, pos+1, tokenB)) {
          return size_t_pair(pos, pos+1);
        }
      } else if (type & token::isPaired) {
        // return if matching pair found like [ ]
        size_t match = pos;
        size_t matchCount = 1;

        while (++match <= End) {
          if (isSpecial(Text, match, tokenA)) {
            ++matchCount;
          }
          if (isSpecial(Text, match, tokenB) && (--matchCount == 0)) {
            return size_t_pair(pos, match);
          }
        }
        break;
      } else {
        // if it's a single char token return when found
        return size_t_pair(pos, pos);
      }
    }

    ++pos;
  }

  // nothing found
  return size_t_pair(string::npos, string::npos);
}

/** @brief FindToken
  *
  * \return this version only returns the start of the token
  */
size_t FindTokenStart(const string& Text,
                      token::tokenName TokenName,
                      size_t Start,
                      size_t End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }

  size_t pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  uint type = token::Type[TokenName];

  while (pos < End) {
    if (isSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        // if it's two char token check for other char and return if found
        if (isSpecial(Text, pos+1, tokenB)) {
          return pos;
        }
      } else if (type & token::isPaired) {
        // return if matching pair found like [ ]
        size_t match = pos;
        size_t matchCount = 1;

        while (++match <= End) {
          if (isSpecial(Text, match, tokenA)) {
            ++matchCount;
          }
          if (isSpecial(Text, match, tokenB) && (--matchCount == 0)) {
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
size_t FindTokenEnd(const string& Text,
                    token::tokenName TokenName,
                    size_t Start,
                    size_t End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }

  size_t pos = Start;
  char tokenA = token::Start[TokenName];
  char tokenB = token::End[TokenName];
  uint type = token::Type[TokenName];

  while (pos < End) {
    if (isSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        ++pos;
        if ((pos < End) && isSpecial(Text, pos, tokenB)) {
          return pos;
        } else {
          break;
        }
      } else if (type & token::isPaired) {
        size_t matchCount = 1;
        while (++pos < End) {
          if (isSpecial(Text, pos, tokenA)) {
            ++matchCount;
          }
          if (isSpecial(Text, pos, tokenB) && (--matchCount == 0)) {
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

/** @brief Find first unescaped passed in character
  *
  * \return npos if not found
  */
size_t FindCharacter(const string& Text,
                     char Char,
                     size_t Start,
                     size_t End)
{
  if (End == 0 || End > Text.size()) {
    End = Text.size();
  }

  size_t pos = Start;

  while (pos < End) {
    if (isSpecial(Text, pos, Char)) {
      return pos;
    }
    ++pos;
  }

  return string::npos;
}

/** @brief Strips Comments in a passed in string
  *
  * removes // and everything after that from the passed in string
  */
void StripComments(string& Text)
{
  const size_t length = Text.size();
  size_t pos = 0;

  while (pos < length) {
    if (isSpecial(Text, pos, token::Start[token::comment]) &&
        (pos+1 < length) && Text[pos+1] == token::End[token::comment]) {
      // ignore the rest of the line
      Text = Text.substr(0, pos);
      break;
    }
    ++pos;
  }
}
