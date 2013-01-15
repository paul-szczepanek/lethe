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
  const size_t length = Text.size();

  char expStart = token::Start[token::expression];
  char expEnd = token::End[token::expression];
  bool ignoreWhite = false;

  while (pos < length) {
    bool escaped = false;
    char c = Text[pos];

    // handle the special meaning the character may have
    if (c == '\t') {
      // ignore illegal chars
      c = ' ';
    } else if (c == '\\') {
      ignoreWhite = false;
      // anything after the escape character is just copied over and ignored
      if (pos + 1 < length) {
        escaped = true;
      } else {
        break;
      }
    } else if (c == '\n') {
      // collapse multiple newlines
      while ((pos + 1 < length) && ('\n' == Text[pos + 1])) {
        ++pos;
      }
      // if the newline is after plain text, treat it as space
      if (!ignoreWhite) {
        clean += " ";
      }
      ignoreWhite = true;
    } else if (c == expStart) { // [
      ++expCount;
      ignoreWhite = true;
      clean += c;
    } else if (c == expEnd) { // ]
      --expCount;
      ignoreWhite = true;
      clean += c;
    } else {
      // we have hit a regular character
      ignoreWhite = false;
      // unless it's a whitespace remover
      if (expCount) {
        // should we remove all whitespace around this character?
        for (size_t i = 0; i < token::NUM_EXPRESSION_SPACE_REMOVERS; ++i) {
          if (c == token::ExpressionWhitespaceRemovers[i]) {
            ignoreWhite = true;
            clean += c;
            break;
          }
        }
      } else {
        for (size_t i = 0; i < token::NUM_SPACE_REMOVERS; ++i) {
          if (c == token::WhitespaceRemovers[i]) {
            ignoreWhite = true;
            clean += c;
            break;
          }
        }
      }
    }

    // deal with whitespace and finally copy the character to the clean text
    if (ignoreWhite) {
      whiteCount = 0;
      while (++pos < length && (Text[pos] == ' ' || Text[pos] == '\t'));
      --pos;
    } else if (c == ' ') {
      // merge whitespaces
      ++whiteCount;
      while (++pos < length && (Text[pos] == ' ' || Text[pos] == '\t')) {
        ++whiteCount;
      }
      --pos;
    } else {
      // deal with accumulated whitespaces
      if (whiteCount) { // restore whitespace
        do { // becuase whiteCount is unsigned
          clean += ' ';
        } while (--whiteCount);
      }

      if (escaped) {
        ++pos;
        // check for a newline
        if (Text[pos] == 'n') {
          clean += "\n";
        } else {
          // copy the escaped character ignoring any special meaning
          clean += "\\";
          clean += Text[pos];
        }
      } else {
        clean += c;
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
  size_t type = token::Type[TokenName];

  while (pos < End) {
    if (IsSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        // if it's two char token check for other char and return if found
        if (IsSpecial(Text, pos+1, tokenB)) {
          return size_t_pair(pos, pos+1);
        }
      } else if (type & token::isPaired) {
        // return if matching pair found like [ ]
        size_t match = pos;
        size_t matchCount = 1;

        while (++match <= End) {
          if (IsSpecial(Text, match, tokenA)) {
            ++matchCount;
          }
          if (IsSpecial(Text, match, tokenB) && (--matchCount == 0)) {
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
  size_t type = token::Type[TokenName];

  while (pos < End) {
    if (IsSpecial(Text, pos, tokenA)) {
      if (type & token::isWide) {
        // if it's two char token check for other char and return if found
        if (IsSpecial(Text, pos+1, tokenB)) {
          return pos;
        }
      } else if (type & token::isPaired) {
        // return if matching pair found like [ ]
        size_t match = pos;
        size_t matchCount = 1;

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
  size_t type = token::Type[TokenName];

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
        size_t matchCount = 1;
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
    if (IsSpecial(Text, pos, Char)) {
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
    if (IsSpecial(Text, pos, token::Start[token::comment]) &&
        (pos+1 < length) && Text[pos+1] == token::End[token::comment]) {
      // ignore the rest of the line
      Text = Text.substr(0, pos);
      break;
    }
    ++pos;
  }
}
