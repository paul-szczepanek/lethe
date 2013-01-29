#include "storyquery.h"
#include "tokens.h"
#include "story.h"
#include "session.h"
#include "book.h"
#include "page.h"

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
size_t StoryQuery::ExecuteBlock(const string& Noun,
                                const Block& CurBlock)
{
  const string& expression = CurBlock.Expression;
  const size_t length = expression.size();

  // handle plain text Expression by copying to the display page
  if (length && expression[0] != token::Start[token::expression]) {
    //Text += " ";
    Text += expression;
    return 0;
  }

  // if there are no child blocks it must be an [!instruction]
  if (length && CurBlock.Blocks.empty()) {
    const size_t_pair tokenPos = FindToken(expression, token::expression);
    const char c = expression[tokenPos.X+1];

    assert(tokenPos.Y != string::npos);

    if (c == token::Start[token::instruction]) {
      // this is a [!<<] (this will work for for [!anything<<])
      if (FindTokenStart(expression, token::stop) != string::npos) {
        const size_t breakUp = tokenPos.Y - tokenPos.X - 2;
        return breakUp; // this breaks out of n loops above
      } else {
        ExecuteExpression(Noun, expression);
      }
    } else if (c == token::Start[token::condition]) { // [?condition]
      LOG(expression + " - childless condition");
    } else { // illegal
      LOG(expression + " - illegal character: " + c +", use ? or ! after [");
    }
  } else { // we have children so this must be a [?condition]
    if (length == 0 || ExecuteExpression(Noun, expression)) {
      // if the condition is true, execute all the child blocks
      for (size_t i = 0, fSz = CurBlock.Blocks.size(); i < fSz; ++i) {
        // if [!<<] used to escape parent break from loop
        size_t backUp = ExecuteBlock(Noun, CurBlock.Blocks[i]);
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

Properties StoryQuery::GetVerbs(const string& Noun)
{
  Properties Result;

  const Page& page = QueryStory.FindPage(Noun);
  for (size_t i = 0, fSz = page.Verbs.size(); i < fSz; ++i) {
    const VerbBlock& verb = page.Verbs[i];
    if (!verb.VisualName.empty()) {
      if (ExecuteExpression(Noun, verb.BlockTree.Expression)) {
        Result.AddValue(verb.VisualName);
      }
    }
  }

  return Result;
}

/** @brief Execute Expression
  *
  * Parses the instruction or condition, checks for user values
  * and modifies them if needed
  */
bool StoryQuery::ExecuteExpression(const string& Noun,
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
    token::tokenName op = token::add;

    if (c == token::Start[token::instruction]) { // [!var=value]
      // check which assignment operation it is
      size_t_pair opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
      if (opPos.X == string::npos) {
        op = token::remove;
        opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
        if (opPos.X == string::npos) {
          op = token::assign;
          opPos = FindToken(Expression, op, tokenPos.X+2, tokenPos.Y);
          if (opPos.X == string::npos) {
            op = token::instruction; // [!value]
            // no assignment, just a bare instruction
          }
        }
      } // TODO: do this in one scan

      // bare instructions have different syntax because no assignment
      if (op != token::instruction) {
        bool isNum = false, isText = false;
        Properties leftValues;
        Properties rightValues;

        const string& left = CutString(Expression, tokenPos.X+2, opPos.X);
        const string& right = CutString(Expression, opPos.Y+1, tokenPos.Y);

        // default to current page noun
        if (left.empty()) {
          leftValues.AddValue(Noun);
        } else {
          EvaluateExpression(leftValues, left, isNum, isText);
        }

        EvaluateExpression(rightValues, right, isNum, isText);

        // assign right values to all nouns whose name is in left values
        for (const string& text : leftValues.TextValues) {
          Properties& userValues = GetUserValues(text);

          switch (op) {
            case token::assign:
              if (isText || right.empty()) {
                userValues.SetValues(rightValues);
              }
              if (isNum) {
                userValues.IntValue = rightValues.IntValue;
              }
              break;
            case token::add:
              if (isText) {
                userValues.AddValues(rightValues);
              }
              if (isNum) {
                userValues.IntValue += rightValues.IntValue;
              }
              break;
            case token::remove:
              if (isText) {
                userValues.RemoveValues(rightValues);
              }
              if (isNum) {
                userValues.IntValue -= rightValues.IntValue;
              }
              break;
            default:
              break;
          }
        }
      }
    } else if (c == token::Start[token::condition]) { // [?var=value]
      // check which condition it is
      // TODO: do this in one scan
      // Keep calm, this is just an unrolled loop
      op = token::contains;
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
                      op = token::condition; // [?value]
                      // no comparison, just a bare condition
                    }
                  }
                }
              }
            }
          }
        }
      } // Keep calm, this was just an unrolled loop

      // bare conditions have different syntax because no comparison
      if (op != token::condition) {
        bool isNum = false, isText = false;
        const string& left = CutString(Expression, tokenPos.X+2, opPos.X);
        const string& right = CutString(Expression, opPos.Y+1, tokenPos.Y);

        Properties rightValues;
        Properties leftEvalValues;
        EvaluateExpression(rightValues, right, isNum, isText);
        // default to current page noun
        if (!left.empty()) {
          EvaluateExpression(leftEvalValues, left, isNum, isText);
        }
        const Properties& leftValues = left.empty()?
                                       GetValues(Noun)
                                       : leftEvalValues;

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
      }
    } else { // illegal
      LOG(Expression + " - illegal character: " + c +", use ? or ! after [");
    }

    if (op == token::condition || op == token::instruction) {
      const string& evaluate = CutString(Expression, tokenPos.X+2, tokenPos.Y);
      bool isNum, isText;
      Properties values;
      EvaluateExpression(values, evaluate, isNum, isText);

      // execute [!noun:verb] commands and [!noun=value] assignments
      for (const string& text : values.TextValues) {
        string noun, verb;
        if (ExtractNounVerb(text, noun, verb)) {
          // if no noun, use the current page noun by default
          if (noun.empty()) {
            noun = Noun;
          }
          const Page& page = QueryStory.FindPage(noun);
          const Block& block = page.GetVerb(verb).BlockTree;
          result &= ExecuteBlock(noun, block);
        }

        const size_t assignPos = FindTokenStart(text, token::assign);
        if (assignPos != string::npos) {
          result &= ExecuteExpression(Noun, "[!" + text + "]");
        }
      }

      // function call instructions return the result in the IntValue
      result = values.IntValue + values.TextValues.size();
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

struct OperationNode {
  token::operationName Operation = token::plus;
  Properties Operand;
  bool Nested = true; // nested doesn't have operand, copy from result of next
};

/** @brief Evaluate Expression
  *
  * Evaluates keywords to their contents and does arithmetic
  * \param context
  * \param expresion to be evaluated
  * \param returns true if there is a number in the expression
  * \param returns true if there is a string in the expression
  * \return Values that the expression results in
  */
bool StoryQuery::EvaluateExpression(Properties& Result,
                                    const string& Expression,
                                    bool& IsNum,
                                    bool& IsText)
{
  // important hints to determine type of assignment
  IsNum = false;
  IsText = false;
  size_t length = Expression.size();
  size_t pos = 0;
  size_t valuePos = 0;
  size_t valueEnd = 0; // needed because function names vary in length
  vector<OperationNode> opStack;
  opStack.reserve(16); // too avoid constant allocs

  // parse expressions like a+@b-func(arg)-#1
  // into an array of pairs like this
  // +,a  +,_ <- @,b  -,func_ <- (,arg  -,_ <- #,1
  // _ <- means that the next pair will write its result there
  // in case of functions it will replace the values that are the func names
  while (pos < length) {
    const char c = Expression[pos];
    bool valueFound = false;
    bool opFound = false;
    bool funcFound = false;

    if (c == '\\') {
      // ignore escaped characters
      ++pos;
    } else if (pos + 1 >= length) {
      valueFound = true;
      valueEnd = ++pos;
      // at least one op needed to store the value
      if (opStack.empty()) {
        opStack.resize(1);
      }
    } else {
      // look for operators that delimate values
      for (size_t i = 0; i < token::OPERATION_NAME_MAX; ++i) {
        if (token::Operations[i] == c) {
          opFound = true;
          valueEnd = pos;
          valueFound = true;
          if (pos > 0) {
            // unless the expression starts with an operator we need
            // a previous op to store the value in
            if (opStack.empty()) {
              opStack.resize(1);
            }
          }
          opStack.resize(opStack.size()+1);

          // grouping and function calls recurse with the (contents)
          if (i == token::parens) {
            const size_t funcEnd = FindTokenEnd(Expression, token::function,
                                                pos);
            if (funcEnd == string::npos) {
              LOG(Expression + " - unmatched \"(\" in expression");
              return false;
            } else {
              funcFound = true;
              // a function need a previous op to store the func names in

              opStack.back().Nested = false; // the arguments are the value
              // prepare function arguments by evaluating recursively
              const string& funcArgs = CutString(Expression, pos + 1, funcEnd);
              EvaluateExpression(opStack.back().Operand, funcArgs,
                                 IsNum, IsText);
            }
            pos = funcEnd;
          }

          opStack.back().Operation = (token::operationName)i;
          break;
        }
      }
    }

    if (valueFound && valueEnd > valuePos) {
      // if an op was found this iteration, assign value to the previous op
      OperationNode& lastOp = opFound? *(opStack.rbegin()+1) : opStack.back();
      const string& valueText = CutString(Expression, valuePos, valueEnd);
      // a value terminates a series of nested operators
      lastOp.Nested = funcFound; // unless it was a function
      lastOp.Operand.AddValue(valueText);
    }

    ++pos;

    if (opFound) {
      valuePos = pos;
    }
  }

  size_t opI = 0; // current operation
  size_t nextOp = 0; // next operation after nesting is finished
  const size_t stackSize = opStack.size();
  // traverse the tree, do math operations and accumulate the result
  while (opI < stackSize) {
    bool nested = false;
    // go to the end of nested ops
    while (opI < stackSize && opStack[opI].Nested) {
      ++opI;
      nested = true;
    }

    if (nested) {
      // remember the end of nested ops so we can pick it up later
      if (nextOp < opI) {
        nextOp = opI + 1;
      }
      opStack[opI - 1].Nested = false; // the result will be copied there
    } else if (opI > 0 && opStack[opI - 1].Nested) {
      // we have a nested value behind us
      // we're in the middle of collapsing a nesting
      nested = true;
      opStack[opI - 1].Nested = false; // the result will be copied there
    }

    Properties& operand = opStack[opI].Operand;
    // when dealing with nested values copy values to previous operand
    Properties& target = nested? opStack[opI - 1].Operand : Result;

    switch (opStack[opI].Operation) {
      case token::plus:
        if (!operand.TextValues.empty()) {
          IsText = true;
          target.AddValues(operand);
        }
        target.IntValue += operand.IntValue;
        break;
      case token::minus:
        if (!operand.TextValues.empty()) {
          IsText = true;
          target.RemoveValues(operand);
        }
        target.IntValue -= operand.IntValue;
        break;
      case token::divide:
        if (!operand.TextValues.empty()) {
          IsText = true;
          target.CommonValues(operand);
        }
        target.IntValue /= operand.IntValue;
        break;
      case token::multiply:
        if (!operand.TextValues.empty()) {
          IsText = true;
          target.ConcatValues(operand);
        }
        target.IntValue *= operand.IntValue;
        break;
      case token::parens:
        // all parens are preceded by nested
        // target is the previous op containing function names
        ExecuteFunction(target, operand);
        // replace the function names with the results
        target.SetValues(operand);
        target.IntValue = operand.IntValue;
        IsNum = true;
        break;
      case token::evaluate:
        IsText = true;
        // evaluate all text into noun values, add them up and reuse the value
        for (string text : operand.TextValues) {
          // try to find the Values of this noun, user values first
          GetUserTextValues(text, target);
        }
        break;
      case token::integer:
        // evaluate all text into int values, add them up and reuse the value
        for (string numberText : operand.TextValues) {
          // check for a plain text number (keywords can't start with a digit)
          if (isdigit(numberText[0])) {
            target.IntValue += IntoInt(numberText);
          } else { // this a noun, find its value
            GetUserInteger(numberText, target);
          }
        }
        IsNum = true;
        break;
      default:
        break;
    }

    // if nested go back to previous op to which the values have been copied
    if (nested) {
      --opI;
    } else { // move to next op
      ++opI;
      // jump over the nested ops we've already evaluated
      if (opI < nextOp) {
        opI = nextOp;
      }
    }
  }

  return !opStack.empty();
}

bool StoryQuery::CreateDialog(const string& Noun, Dialog& NewDialog)
{
  NewDialog.Noun = Noun;
  // verbs as buttons
  const Properties& verbs = GetVerbs(Noun);
  if (verbs.IsEmpty()) {
    // you can't have a dialog with no buttons
    return false;
  }
  NewDialog.Buttons = verbs.TextValues;
  // contents of the noun used as the message
  Properties values;
  GetUserTextValues(Noun, values);
  for (const string& text : values.TextValues) {
    NewDialog.Message += text;
  }
  return true;
}

/** @brief Executes all functions on all argument values
  */
bool StoryQuery::ExecuteFunction(const Properties& FunctionName,
                                 Properties& FunctionArgs)
{
  for (const string& func : FunctionName.TextValues) {
    // execute function
    // TODO: hash the function names
    lint& intArg = FunctionArgs.IntValue;
    vector<string>& textArgs = FunctionArgs.TextValues;
    if (func == "Size") {
      // return number of values
      intArg = 0;
      for (const string& arg : textArgs) {
        const Properties nounValues = GetValues(arg);
        intArg += nounValues.TextValues.size();
      }
      textArgs.clear();
    } else if (func == "Play") {
      // activate asset
      for (const string& arg : textArgs) {
        if (!QueryBook.GetAssetState(arg)) {
          QueryBook.SetAssetState(arg, true);
        }
        LOG(arg + " - play");
      }
      textArgs.clear();
      intArg = 1;
    } else if (func == "Stop") {
      // deactivate asset
      for (const string& arg : textArgs) {
        if (!QueryBook.GetAssetState(arg)) {
          QueryBook.SetAssetState(arg, false);
        }
        LOG(arg + " - stop");
      }
      textArgs.clear();
      intArg = 1;
    } else if (func == "Keyword") {
      // print a list of values
      Text += FunctionArgs.PrintKeywordList();
      textArgs.clear();
      intArg = 1;
    } else if (func == "SelectValue") {
      // print a list of <value[noun=value:verb]>
      for (const string& arg : textArgs) {
        const size_t scopePos = FindTokenEnd(arg, token::scope);
        if (scopePos != string::npos) {
          // rearrange the noun:verb into proper places
          const string& noun = CutString(arg, 0, scopePos);
          const string& verb = CutString(arg, scopePos + 1);
          const Properties& values = GetValues(noun);
          Text += values.PrintValueSelectList(noun, verb, "\n ");
        }
      }
      textArgs.clear();
      intArg = 1;
    } else if (func == "Print") {
      // print a list of values as keywords
      Text += FunctionArgs.PrintPlainList();
      textArgs.clear();
      intArg = 1;
    } else if (func == "CloseMenu") {
      // close main menu, show book
      intArg = (lint)QueryBook.HideMenu();
      textArgs.clear();
    } else if (func == "OpenMenu") {
      // show main menu
      intArg = (lint)QueryBook.ShowMenu();
      textArgs.clear();
    } else if (func == "CloseBook") {
      // saves the sessions and closes the book
      QueryBook.CloseBook();
      intArg = 1;
      textArgs.clear();
    } else if (func == "OpenBook") {
      // try values until you open a book
      intArg = 0;
      for (const string& arg : textArgs) {
        if (QueryBook.OpenBook(arg)) {
          intArg = 1;
          break;
        }
      }
      textArgs.clear();
    } else if (func == "Quit") {
      // Exit the reader
      textArgs.clear();
      intArg = 1;
      QueryBook.CloseBook();
      QueryBook.Quit();
    } else if (func == "GetBooks") {
      // return book names
      FunctionArgs = QueryBook.GetBooks();
      intArg = 1;
    } else if (func == "IsInGame") {
      // return 1 if a book is open
      textArgs.clear();
      intArg = (lint)QueryBook.SessionOpen;
    } else if (func == "GetSessions") {
      // return session names
      FunctionArgs = QueryBook.GetSessions();
      intArg = 1;
    } else if (func == "GetSessionName") {
      // return session names
      textArgs.clear();
      FunctionArgs.AddValue(QueryBook.GetSessionName());
      intArg = 1;
    } else if (func == "SaveSession") {
      // saves the session, doesn't close it
      if (textArgs.empty()) {
        intArg = (lint)QueryBook.SaveSession();
      } else {
        const string& name = textArgs[0];
        intArg = (lint)QueryBook.SaveSession(name);
      }
      textArgs.clear();
    } else if (func == "BranchSession") {
      // saves the session and creates a new one from current place in time
      if (textArgs.empty()) {
        intArg = (lint)QueryBook.BranchSession();
      } else {
        const string& name = textArgs[0];
        intArg = (lint)QueryBook.BranchSession(name);
      }
      textArgs.clear();
    } else if (func == "LoadSession") {
      // load session or continue last played session if no name given
      if (textArgs.empty()) {
        intArg = (lint)QueryBook.LoadSession();
      } else {
        const string& name = textArgs[0];
        intArg = (lint)QueryBook.LoadSession(name);
        textArgs.clear();
      }
    } else if (func == "NewSession") {
      // start new session
      textArgs.clear();
      intArg = (lint)QueryBook.NewSession();
    } else if (func == "Bookmark") {
      // create a bookmark using the argument to produce the text
      QueryBook.SetBookmark(FunctionArgs);
      intArg = 1;
      textArgs.clear();
    } else if (func == "UserBookmark") {
      // create a bookmark using the argument to produce the text
      // overwrites any previous bookmark at that place
      string bookmarkDesc;
      for (const string& arg : textArgs) {
        bookmarkDesc += arg;
      }
      QueryBook.SetBookmark(bookmarkDesc);
      intArg = 1;
      textArgs.clear();
    } else if (func == "LoadSnapshot") {
      // Load the given snapshot
      intArg = 0;
      if (!textArgs.empty()) {
        intArg = (lint)QueryBook.LoadSnapshot(textArgs[0]);
        textArgs.clear();
      }
    } else if (func == "GetSnapshots") {
      // return snapshots with descriptions, range based on the int value
      textArgs.clear();
      QueryBook.GetSnapshots(FunctionArgs);
    } else if (func == "GetBookmarks") {
      // return snapshots with bookmarks only
      textArgs.clear();
      QueryBook.GetBookmarks(FunctionArgs);
    } else if (func == "GetSnapshotIndex") {
      // return current snapshot index
      intArg = QueryBook.GetCurrentSnapshot();
      textArgs.clear();
    } else if (func == "Dialog") {
      // create a dialog
      intArg = 1;
      Dialog dialog;
      for (const string& arg : textArgs) {
        if (CreateDialog(arg, dialog)) {
          QueryBook.AddDialog(dialog);
        } else {
          intArg = 0;
        }
      }
      textArgs.clear();
    } else if (func == "Input") {
      // create an input dialog that saves user response in the noun
      intArg = 1;
      Dialog dialog;
      dialog.InputBox = true;
      for (const string& arg : textArgs) {
        if (CreateDialog(arg, dialog)) {
          QueryBook.AddDialog(dialog);
        } else {
          intArg = 0;
        }
      }
      textArgs.clear();
    } else {
      LOG(func + " - function doesn't exist!");
      textArgs.clear();
      return false;
    }
  }
  return true;
}

/** @brief return values for assignment
  */
Properties& StoryQuery::GetUserValues(const string& Noun)
{
  if (!QuerySession.IsUserValues(Noun)) {
    QuerySession.UserValues[Noun] = QueryStory.FindPage(Noun).PageValues;
  }
  QuerySession.ValuesChanged = true;
  return QuerySession.UserValues[Noun];
}

/** @brief return values for quick access but only for reading
  */
const Properties& StoryQuery::GetValues(const string& Noun)
{
  if (QuerySession.IsUserValues(Noun)) {
    return QuerySession.UserValues[Noun];
  } else {
    return QueryStory.FindPage(Noun).PageValues;
  }
}

/** @brief add user values of the noun to the passed in Properites
  * \return true if value was found in the user values
  */
bool StoryQuery::GetUserTextValues(const string& Noun,
                                   Properties& Result)
{
  if (QuerySession.GetUserTextValues(Noun, Result)) {
    return true;
  }
  // fall back to values as defined in the story
  Result.AddValues(QueryStory.FindPage(Noun).PageValues);
  return false;
}

/** @brief add user integer value of the noun to the passed in Properites
  * \return true if value was found in the user values
  */
bool StoryQuery::GetUserInteger(const string& Noun,
                                Properties& Result)
{
  if (QuerySession.GetUserInteger(Noun, Result)) {
    return true;
  }
  // fall back to values as defined in the story
  Result.IntValue = QueryStory.FindPage(Noun).PageValues.IntValue;
  return false;
}
