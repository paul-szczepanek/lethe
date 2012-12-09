#include "story.h"
#include "page.h"
#include "tokens.h"
#include "session.h"

Page Story::MissingPage = Page("");

Story::Story()
{
  //ctor
}

Story::~Story()
{
  page_it it = Pages.begin();
  while (it != Pages.end()) {
    delete it->second;
    ++it;
  }
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

/** @brief FindPage
  *
  * @todo: document this function
  */
Page& Story::FindPage(const string& Noun)
{
  page_it it = Pages.find(Noun);

  if (it != Pages.end()) {
    return *(it->second);
  }

  LOG(Noun + " - not defined in the story");

  return MissingPage;
}


string Story::Read(Session& Progress,
                   const string& Noun,
                   const string& VerbName)
{
  string pageText;

  const Page& page = FindPage(Noun);
  const Verb& verb = page.GetVerb(VerbName);

  ExecuteBlock(Noun, verb.BlockTree, Progress, pageText);

  return pageText;
}

/** \brief fills in the Text param with the story to display
 * Will recursively evalute the Expression and fill in Text resulting from the
 * evaluation
 *
 * \param current page we're on, we may visit other pages whilst recursing
 * \param the block containing the expression or text
 * \param all values that are different than the story ones i.e. savedata
 * \param the resulting text of the story
 * \return how many parents up do we break with [!<<]
 */
uint Story::ExecuteBlock(const string& Noun,
                         const Block& CurBlock,
                         Session& Progress,
                         string& Text)
{
  const string& expression = CurBlock.Expression;
  const size_t length = expression.size();

  // handle plain text Expression by copying to the display page
  if (length && expression[0] != token::Start[token::expression]) {
    Text += expression;
    return 0;
  }

  // if there are no child blocks it must be an [!instruction]
  if (length && CurBlock.Blocks.empty()) {
    const size_t_pair tokenPos = FindToken(expression, token::expression);
    const char c = expression[tokenPos.X+1];

    assert(tokenPos.Y != string::npos);

    if (c == token::Start[token::instruction]) {
      // is it a [!noun:verb] command
      const size_t scopePos = FindTokenStart(expression, token::scope);

      // if it's an command, recurse with the new block
      if (scopePos != string::npos) {
        string noun = cutString(expression, tokenPos.X+2, scopePos);
        const string verbName = cutString(expression, scopePos+1, tokenPos.Y);

        // if no noun, use the current page noun by default
        if (noun.empty()) {
          noun = Noun;
        }
        const Page& page = FindPage(noun);
        const Block& block = page.GetVerb(verbName).BlockTree;

        return ExecuteBlock(noun, block, Progress, Text);
      } else if (FindTokenStart(expression, token::stop) != string::npos) {
        // this is a [!<<] (this will work for for [!anything<<])
        const uint breakUp = tokenPos.Y - tokenPos.X - 2;
        return breakUp; // this breaks out of n loops above
      } else {
        //execute the expression
        ExecuteExpression(Noun, Progress, expression);
      }
    } else if (c == token::Start[token::condition]) { // [?condition]
      LOG(expression + " - childless condition");
    } else { // illegal
      LOG(expression + " - illegal character: " + c +", use ? or ! after [");
    }
  } else { // we have children so this must be a [?condition]
    if (length == 0 || ExecuteExpression(Noun, Progress, expression)) {
      // if the condition is true, execute all the child blocks
      for (size_t i = 0, forSz = CurBlock.Blocks.size(); i < forSz; ++i) {
        // if [!<<] used to escape parent break from loop
        uint backUp = ExecuteBlock(Noun, CurBlock.Blocks[i], Progress, Text);
        if (backUp > 0) {
          // so the parent remains unaffected
          return --backUp;
        }
      }
    }
  }

  // all OK, continue parent execution normally
  return 0;
}

/** \brief get list of active verbs
 *
 * \return array of verbs that pass their top level condition
 */
vector<string> Story::GetVerbs(Session& Progress,
                               const string& Noun)
{
  vector<string> verbs;
  const Page& page = FindPage(Noun);

  for (size_t i = 0, for_size = page.Verbs.size(); i < for_size; ++i) {
    const Verb& verb = page.Verbs[i];
    if (!verb.VisualName.empty()) {
      if (ExecuteExpression(Noun, Progress, verb.BlockTree.Expression)) {
        verbs.push_back(verb.VisualName);
      }
    }
  }

  return verbs;
}

/** @brief Evaluate Expression
  *
  * Evaluates keywords to their contents and does arithmetic
  * \param context
  * \param expresion to be evaluated
  * \param returns true if there is a number in the expression
  * \param returns true if there is a string in the expression
  * \return Values that the expression results in
  */
Properties Story::EvaluateExpression(const Session& Progress,
                                     const string& Expression,
                                     bool& IsNum,
                                     bool& IsText)
{
  Properties result;
  IsNum = false;
  IsText = false;

  size_t length = Expression.size();
  size_t pos = 0;
  size_t valuePos = 0;

  vector<Properties*> valueStack;
  vector<token::mathName> opStack;

  // it's more efficient to scan char by char
  while (pos < length) {
    const char c = Expression[pos];
    // ignore escaped characters
    if (c == '\\') {
      pos += 2;
      continue;
    }

    bool valueCharacter = true;
    // look for operators that delimate values
    for (size_t i = 0; i < token::MATH_NAME_MAX; ++i) {
      if (token::Math[i] == c) {
        valueCharacter = false;
        opStack.push_back((token::mathName)i);
        break;
      }
    }

    // not hit an operator
    if (valueCharacter) {
      ++pos;
      // don't continue if it's the last character so we can get the last value
      if (pos < length) {
        continue;
      }
    }

    // create temporary values
    const string& valueText = cutString(Expression, valuePos, pos);
    Properties* newValue = new Properties(valueText);

    if (!valueText.empty()) {
      if (valueText[0] == token::Start[token::number]) { // treat as number
        IsNum = true;
        const string number = cutString(valueText, 1, valueText.size());
        // check for a plain text number (keywords can't start with a digit)
        if (isdigit(valueText[1])) {
          newValue->IntValue = intoInt(number);
        } else {
          // this a noun, find its value
          if (!Progress.GetUserValuesInteger(number, newValue->IntValue)) {
            newValue->IntValue = FindPage(number).PageValues.IntValue;
          }
        }
      } else if (valueText[0] == token::Start[token::value]) { // text values
        IsText = true;
        const string text = cutString(valueText, 1, valueText.size());
        // try to find the Values of this noun, user values first
        if (!Progress.GetUserValuesText(text, newValue->TextValues)) {
          newValue->AddValues(FindPage(text).PageValues);
        }
      } else { // plain text
        IsText = true;
        newValue->AddValue(valueText);
      }
    }

    valueStack.push_back(newValue);

    valuePos = ++pos;
  }

  // execute math operators
  if (!valueStack.empty()) {
    result = *valueStack[0];

    for (size_t i = 1, j = 0, forSz = valueStack.size(); i < forSz; ++i, ++j) {
      const Properties& value = *valueStack[i];

      switch (opStack[j]) {
        case token::plus:
          result.AddValues(value);
          result.IntValue += value.IntValue;
          break;
        case token::minus:
          result.RemoveValues(value);
          result.IntValue -= value.IntValue;
          break;
        case token::divide:
          result.IntValue /= value.IntValue;
          break;
        case token::multiply:
          result.IntValue *= value.IntValue;
          break;
        default:
          break;
      }

      delete valueStack[i];

      ++j;
    }
  }

  return result;
}


/** @brief Execute Expression
  *
  * Parses the instruction or condition, checks for user values
  * and modifies them if needed
  */
bool Story::ExecuteExpression(const string& Noun,
                              Session& Progress,
                              const string& Expression)
{
  size_t length = Expression.size();
  size_t pos = 0;
  bool shortAnd = false;
  bool shortOr = false;
  bool result = true;

  // might contain a chain of expressions so loop through them
  while (pos < length) {
    result = true;
    const size_t_pair tokenPos = FindToken(Expression, token::expression, pos);

    if (tokenPos.Y == string::npos) {
      LOG(Expression + " - expression ended unexpectedly");
      return false;
    }

    const char c = Expression[tokenPos.X+1];

    if (c == token::Start[token::instruction]) { // [!var=value]
      // check which assignment operation it is
      // TODO: do this in one scan
      token::tokenName op = token::remove;
      size_t_pair opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
      if (opPos.X == string::npos) {
        op = token::add;
        opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
        if (opPos.X == string::npos) {
          op = token::assign;
          opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
          if (opPos.X == string::npos) {
            LOG(Expression + " - wrong isntruction");
            return false;
          }
        }
      }

      string left = cutString(Expression, tokenPos.X+2, opPos.X);

      // default to current page noun
      if (left.empty()) {
        left = Noun;
      } else {
        assert(CleanLeftSide(left));
      }

      // copy the story values if they haven't been created for the user yet
      if (!Progress.IsUserValues(left)) {
        Progress.UserValues[left] = FindPage(left).PageValues;
      }

      Properties& userValues = Progress.UserValues[left];

      const string evaluate = cutString(Expression, opPos.Y+1, tokenPos.Y);

      bool isNum, isText;
      const Properties& newValues = EvaluateExpression(Progress, evaluate,
                                    isNum, isText);

      switch (op) {
        case token::assign:
          if (isText || evaluate.empty()) {
            userValues.SetValues(newValues);
          }
          if (isNum) {
            userValues.IntValue = newValues.IntValue;
          }
          break;
        case token::add:
          if (isText) {
            userValues.AddValues(newValues);
          }
          if (isNum) {
            userValues.IntValue += newValues.IntValue;
          }
          break;
        case token::remove:
          if (isText) {
            userValues.RemoveValues(newValues);
          }
          if (isNum) {
            userValues.IntValue -= newValues.IntValue;
          }
          break;
        default:
          break;
      }
    } else if (c == token::Start[token::condition]) { // [?var=value]
      // check which condition it is
      // TODO: do this in one scan
      // Keep calm, this is just an unrolled loop
      token::tokenName op = token::contains;
      size_t_pair opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
      if (opPos.X == string::npos) {
        op = token::notContains;
        opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
        if (opPos.X == string::npos) {
          op = token::notEquals;
          opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
          if (opPos.X == string::npos) {
            op = token::equalsOrLess;
            opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
            if (opPos.X == string::npos) {
              op = token::equalsOrMore;
              opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
              if (opPos.X == string::npos) {
                op = token::equals;
                opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
                if (opPos.X == string::npos) {
                  op = token::isMore;
                  opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
                  if (opPos.X == string::npos) {
                    op = token::isLess;
                    opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
                    if (opPos.X == string::npos) {
                      LOG(Expression + " - illegal condition");
                      return false;
                    }
                  }
                }
              }
            }
          }
        }
      } // Keep calm, this was just an unrolled loop

      string left = cutString(Expression, tokenPos.X+2, opPos.X);

      // default to current page noun
      if (left.empty()) {
        left = Noun;
      } else {
        assert(CleanLeftSide(left));
      }

      // if UserValues don't exist use page values
      const Properties& leftValues = Progress.IsUserValues(left)?
                                     Progress.UserValues[left]
                                     : FindPage(left).PageValues;

      const string& right = cutString(Expression, opPos.Y+1, tokenPos.Y);

      bool isNum, isText;
      const Properties& rightValues = EvaluateExpression(Progress, right,
                                      isNum, isText);

      switch (op) {
        case token::contains:
          if (isText) {
            result = leftValues.ContainsValues(rightValues);
          }
          if (isNum && result) {
            result = leftValues.IntValue == rightValues.IntValue;
          }
          break;
        case token::notContains:
          if (isText) {
            result = !leftValues.ContainsValues(rightValues);
          }
          if (isNum && result) {
            result = leftValues.IntValue != rightValues.IntValue;
          }
          break;
        case token::equals:
          if (isText) {
            result = leftValues.IsEquivalent(rightValues);
          }
          if (isNum && result) {
            result = leftValues.IntValue == rightValues.IntValue;
          }
          break;
        case token::notEquals:
          if (isText) {
            result = !leftValues.IsEquivalent(rightValues);
          }
          if (isNum && result) {
            result = leftValues.IntValue != rightValues.IntValue;
          }
          break;
        case token::equalsOrLess:
          result = leftValues.IntValue <= rightValues.IntValue;
          break;
        case token::equalsOrMore:
          result = leftValues.IntValue >= rightValues.IntValue;
          break;
        case token::isMore:
          result = leftValues.IntValue > rightValues.IntValue;
          break;
        case token::isLess:
          result = leftValues.IntValue < rightValues.IntValue;
          break;
        default:
          break;
      }
    } else { // illegal
      LOG(Expression + " - illegal character: " + c +", use ? or ! after [");
    }

    pos = tokenPos.Y;
    ++pos;

    // short circuit logical & and |
    if (shortAnd && !result) {
      return false;
    } else if (shortOr && result) {
      return true;
    }

    if (pos < length) {
      if (Expression[pos] == token::Start[token::logicalAnd]) {
        if (!result) {
          return false;
        }
        shortAnd = true;
      } else if (Expression[pos] == token::Start[token::logicalOr]) {
        if (result) {
          return true;
        }
        shortOr = true;
      }
    }
  } // end while

  return result;
}

/** @brief removes # and @
  */
bool Story::CleanLeftSide(string& Left)
{
  // cut the @ if used
  if (Left[0] == token::Start[token::value]) {
    Left = cutString(Left, 1, Left.size());
    LOG("do not use @ on the left side");
    return false;
  } else if (Left[0] == token::Start[token::number]) {
    Left = cutString(Left, 1, Left.size());
    LOG("do not use # on the left side");
    return false;
  }
  return true;
}
