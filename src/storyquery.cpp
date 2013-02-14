#include "storyquery.h"
#include "tokens.h"
#include "story.h"
#include "session.h"
#include "book.h"
#include "page.h"

/** \brief Will recursively evaluate the Expression in each child block
  * starting with current block and fill in Text resulting from the evaluation
  * \return how many parents up do we go up to with !<<
  */
szt StoryQuery::ExecuteBlock(const string& Noun,
                             const Block& CurBlock,
                             cszt Level)
{
  const string& expression = CurBlock.Expression;
  if (Level > 100) {
    LOG("Excessive call stack " + IntoString(Level) + " levels deep at:"
        + expression);
    if (Level > 1000) {
      LOG("infinite (1000 levels deep) loop at: " + expression + ". Aborting.");
      return 2000;
    }
  }
  cszt length = expression.size();
#ifdef DEVBUILD
  if (GTrace[GTrace.size() - 1] != '\n') {
    // don't add double breaklines (caused by empty expressions)
    GTrace += "\n";
  }
  for (szt i = 0; i < Level + GTraceIndent; ++i) {
    GTrace += "  ";
  }
#endif
  if (CurBlock.Blocks.empty()) {
    if (length) {
      if (CurBlock.Execute) {
        // if there are no child blocks it must be an !instruction
#ifdef DEVBUILD
        GTrace += "!" + expression;
#endif
        // this is a !<<
        if (FindTokenStart(expression, token::stop) != string::npos) {
          // we know that at least two < exist already
          szt breakUp = 3;
          while (breakUp < length
                 && expression[breakUp] == token::Start[token::stop]) {
            ++breakUp;
          }
          return breakUp; // this breaks out of n loops above
        } else {
#ifdef DEVBUILD
          ++GTraceIndent;
#endif
          ExecuteExpression(Noun, expression);
#ifdef DEVBUILD
          --GTraceIndent;
#endif
        }
      } else {
#ifdef DEVBUILD
        cszt sampleEnd = min((szt)40, FindCharacter(expression, '\n'));
        GTrace += "\"" + CutString(expression, 0, sampleEnd);
        if (expression.size() > sampleEnd) {
          GTrace += "...\"";
        } else {
          GTrace += '\"';
        }
#endif
        // handle plain text Expression by copying to the display page
        if ((Text.empty() || Text[Text.size() - 1] != ' ')
            && expression[0] != ',' && expression[0] != ' '
            && expression[0] != '.') {
          Text += " ";
        }

        Text += expression;
      }
    }
  } else {
    // this block has children so this must be a ?condition
#ifdef DEVBUILD
    if (length) {
      GTrace += " ?" + expression;
    }
#endif
    if (length == 0 || ExecuteExpression(Noun, expression, true)) {
      // if the condition is true, execute all the child blocks
      bool executeElse = false;
      for (szt i = 0, fSz = CurBlock.Blocks.size(); i < fSz; ++i) {
        // if !<< used to escape parent break from loop
        szt backUp = 0; // this will carry the escape up the call stack
        if (!CurBlock.Blocks[i].Else || executeElse) {
#ifdef DEVBUILD
          if (CurBlock.Blocks[i].Else) {
            GTrace += "else";
          }
#endif
          backUp = ExecuteBlock(Noun, CurBlock.Blocks[i], Level + 1);
        }

        executeElse = false;
        if (backUp > 0) {
          if (backUp == 1) {
            executeElse = true;
          } else {
            return --backUp;
          }
        }
      }
    } else {
#ifdef DEVBUILD
      GTrace += " (failed) ";
#endif
      return 1;
    }
  }

  // all OK, continue parent execution normally
  return 0;
}

/** @brief Get verbs that don't fail their verb expression
  */
Properties StoryQuery::GetVerbs(const string& Noun)
{
  Properties Result;
  const Page& page = QueryStory.FindPage(Noun);
  for (szt i = 0, fSz = page.Verbs.size(); i < fSz; ++i) {
    const VerbBlock& verb = page.Verbs[i];
    if (!verb.VisualName.empty()) {
      if (ExecuteExpression(Noun, verb.BlockTree.Expression, true)) {
        Result.AddValue(verb.VisualName);
      }
    }
  }
  return Result;
}

/** @brief Parses the instruction or condition, checks for user values
  * and modifies them if needed
  */
bool StoryQuery::ExecuteExpression(const string& Noun,
                                   const string& Expression,
                                   bool Condition)
{
  szt length = Expression.size();
  szt pos = 0;
  bool shortAnd = false;
  bool shortOr = false;
  bool result = true;

  // might contain a chain of expressions so loop through them
  while (pos < length) {
    result = true;
    token::tokenName op = token::add;
    cszt aPos = FindCharacter(Expression, token::Start[token::logicalAnd], pos);
    cszt oPos = FindCharacter(Expression, token::Start[token::logicalOr], pos);
    cszt endPos = min(length, min(aPos, oPos));

    if (endPos < length) {
      if (aPos < oPos) {
        shortAnd = true;
        shortOr = false;
      } else {
        shortAnd = false;
        shortOr = true;
      }
    }

    if (Condition) {
      // check which condition it is
      // TODO: do this in one scan
      // Keep calm, this is just an unrolled loop
      op = token::contains;
      szt_pair opPos = FindToken(Expression, op, pos, endPos);
      if (opPos.X == string::npos) {
        op = token::notContains;
        opPos = FindToken(Expression, op, pos, endPos);
        if (opPos.X == string::npos) {
          op = token::notEquals;
          opPos = FindToken(Expression, op, pos, endPos);
          if (opPos.X == string::npos) {
            op = token::equalsOrLess;
            opPos = FindToken(Expression, op, pos, endPos);
            if (opPos.X == string::npos) {
              op = token::equalsOrMore;
              opPos = FindToken(Expression, op, pos, endPos);
              if (opPos.X == string::npos) {
                op = token::equals;
                opPos = FindToken(Expression, op, pos, endPos);
                if (opPos.X == string::npos) {
                  op = token::isMore;
                  opPos = FindToken(Expression, op, pos, endPos);
                  if (opPos.X == string::npos) {
                    op = token::isLess;
                    opPos = FindToken(Expression, op, pos, endPos);
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
        const string& left = CutString(Expression, pos, opPos.X);
        const string& right = CutString(Expression, opPos.Y+1, endPos);

        Properties rightValues;
        Properties leftEvalValues;
        EvaluateExpression(rightValues, right, isNum, isText);
        // default to current page noun
        if (!left.empty()) {
          EvaluateExpression(leftEvalValues, left, isNum, isText);
#ifdef DEVBUILD
          if (FindTokenStart(left, token::number) == string::npos
              && FindTokenStart(left, token::value) == string::npos
              && FindTokenStart(left, token::function) == string::npos) {
            LOG("warning: ?" + Expression
                + " - did you forget a @ or # on the left?");
          }
#endif
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
    } else { // instruction
      // check which assignment operation it is
      szt_pair opPos = FindToken(Expression, op, pos, endPos);
      if (opPos.X == string::npos) {
        op = token::remove;
        opPos = FindToken(Expression, op, pos, endPos);
        if (opPos.X == string::npos) {
          op = token::assign;
          opPos = FindToken(Expression, op, pos, endPos);
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

        const string& left = CutString(Expression, pos, opPos.X);
        const string& right = CutString(Expression, opPos.Y+1, endPos);

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
    }

    if (op == token::condition || op == token::instruction) {
      const string& evaluate = CutString(Expression, pos, endPos);
      bool isNum, isText;
      Properties values;
      EvaluateExpression(values, evaluate, isNum, isText);

      // execute !noun:verb commands and !noun=value assignments
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

        cszt assignPos = FindTokenStart(text, token::assign);
        if (assignPos != string::npos) {
          result &= ExecuteExpression(Noun, text);
        }
      }

      // function call instructions return the result in the IntValue
      result = values.IntValue + values.TextValues.size();
    }

    pos = endPos;
    ++pos;

    // short circuit logical & and |
    if (shortAnd && !result) {
      return false;
    } else if (shortOr && result) {
      return true;
    }
  } // end while

  return result;
}

struct OperationNode {
  token::operationName Operation = token::plus;
  Properties Operand;
  bool Nested = true; // nested doesn't have operand, copy from result of next
};

/** @brief Evaluates keywords to their contents and does arithmetic
  */
bool StoryQuery::EvaluateExpression(Properties& Result,
                                    const string& Expression,
                                    bool& IsNum,
                                    bool& IsText)
{
  // important hints to determine type of assignment
  IsNum = false;
  IsText = false;
  szt length = Expression.size();
  szt pos = 0;
  szt valuePos = 0;
  szt valueEnd = 0; // needed because function names vary in length
  vector<OperationNode> opStack;
  opStack.reserve(16); // too avoid constant allocations

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
      for (szt i = 0; i < token::OPERATION_NAME_MAX; ++i) {
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
            cszt funcEnd = FindTokenEnd(Expression, token::function,
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

  szt opI = 0; // current operation
  szt nextOp = 0; // next operation after nesting is finished
  cszt stackSize = opStack.size();
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
          if (isdigit(numberText[0]) || numberText[0] == '-') {
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

/** @brief Fills in the dialog with text based on the verb found in the Noun
  */
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

const map<const string, cszt> FunctionNameMap = {
  { "Size", bookFunctionSize },
  { "Play", bookFunctionPlay },
  { "Stop", bookFunctionStop },
  { "Keyword", bookFunctionKeyword },
  { "SelectValue", bookFunctionSelectValue },
  { "Print", bookFunctionPrint },
  { "CloseMenu", bookFunctionCloseMenu },
  { "OpenMenu", bookFunctionOpenMenu },
  { "CloseBook", bookFunctionCloseBook },
  { "OpenBook", bookFunctionOpenBook },
  { "Quit", bookFunctionQuit },
  { "GetBooks", bookFunctionGetBooks },
  { "IsInGame", bookFunctionIsInGame },
  { "GetSessions", bookFunctionGetSessions },
  { "GetSessionName", bookFunctionGetSessionName },
  { "SaveSession", bookFunctionSaveSession },
  { "BranchSession", bookFunctionBranchSession },
  { "LoadSession", bookFunctionLoadSession },
  { "NewSession", bookFunctionNewSession },
  { "Bookmark", bookFunctionBookmark },
  { "UserBookmark", bookFunctionUserBookmark },
  { "LoadSnapshot", bookFunctionLoadSnapshot },
  { "GetSnapshots", bookFunctionGetSnapshots },
  { "GetBookmarks", bookFunctionGetBookmarks },
  { "GetSnapshotIndex", bookFunctionGetSnapshotIndex },
  { "Dialog", bookFunctionDialog },
  { "Input", bookFunctionInput }
};

/** @brief Executes all functions on all argument values
  */
bool StoryQuery::ExecuteFunction(const Properties& FunctionName,
                                 Properties& FunctionArgs)
{
  lint& intArg = FunctionArgs.IntValue;
  vector<string>& textArgs = FunctionArgs.TextValues;
  for (const string& func : FunctionName.TextValues) {
    // translate string into an enum
    szt functionIndex = BOOK_FUNCTION_MAX;
    const auto it = FunctionNameMap.find(func);
    if (it != FunctionNameMap.end()) {
      functionIndex = it->second;
    }
    switch (functionIndex) {
      case bookFunctionSize:
        // return number of values
        intArg = 0;
        for (const string& arg : textArgs) {
          const Properties nounValues = GetValues(arg);
          intArg += nounValues.TextValues.size();
        }
        textArgs.clear();
        break;
      case bookFunctionPlay:
        // activate asset
        for (const string& arg : textArgs) {
          if (!QueryBook.GetAssetState(arg)) {
            QueryBook.SetAssetState(arg, true);
          }
        }
        textArgs.clear();
        intArg = 1;
        break;
      case bookFunctionStop:
        // deactivate asset
        for (const string& arg : textArgs) {
          if (QueryBook.GetAssetState(arg)) {
            QueryBook.SetAssetState(arg, false);
          }
        }
        textArgs.clear();
        intArg = 1;
        break;
      case bookFunctionKeyword:
        // print a list of values
        Text += FunctionArgs.PrintKeywordList();
        textArgs.clear();
        intArg = 1;
        break;
      case bookFunctionSelectValue:
        // print a list of <value[noun=value:verb]>
        for (const string& arg : textArgs) {
          cszt scopePos = FindTokenEnd(arg, token::scope);
          if (scopePos != string::npos) {
            // rearrange the noun:verb into proper places
            const string& noun = CutString(arg, 0, scopePos);
            const string& verb = CutString(arg, scopePos + 1);
            const Properties& values = GetValues(noun);
            Text += values.PrintValueSelectList(noun, verb, "\n");
          }
        }
        textArgs.clear();
        intArg = 1;
        break;
      case bookFunctionPrint:
        // print a list of values as keywords
        Text += FunctionArgs.PrintPlainList();
        textArgs.clear();
        intArg = 1;
        break;
      case bookFunctionCloseMenu:
        // close main menu, show book
        intArg = (lint)QueryBook.HideMenu();
        textArgs.clear();
        break;
      case bookFunctionOpenMenu:
        // show main menu
        intArg = (lint)QueryBook.ShowMenu();
        textArgs.clear();
        break;
      case bookFunctionCloseBook:
        // saves the sessions and closes the book
        QueryBook.CloseBook();
        intArg = 1;
        textArgs.clear();
        break;
      case bookFunctionOpenBook:
        // try values until you open a book
        intArg = 0;
        for (const string& arg : textArgs) {
          if (QueryBook.OpenBook(arg)) {
            intArg = 1;
            break;
          }
        }
        textArgs.clear();
        break;
      case bookFunctionQuit:
        // Exit the reader
        textArgs.clear();
        intArg = 1;
        QueryBook.CloseBook();
        QueryBook.Quit();
        break;
      case bookFunctionGetBooks:
        // return book names
        FunctionArgs = QueryBook.GetBooks();
        intArg = 1;
        break;
      case bookFunctionIsInGame:
        // return 1 if a book is open
        textArgs.clear();
        intArg = (lint)QueryBook.SessionOpen;
        break;
      case bookFunctionGetSessions:
        // return session names
        FunctionArgs = QueryBook.GetSessions();
        intArg = 1;
        break;
      case bookFunctionGetSessionName:
        // return session names
        textArgs.clear();
        FunctionArgs.AddValue(QueryBook.GetSessionName());
        intArg = 1;
        break;
      case bookFunctionSaveSession:
        // saves the session, doesn't close it
        if (textArgs.empty()) {
          intArg = (lint)QueryBook.SaveSession();
        } else {
          const string& name = textArgs[0];
          intArg = (lint)QueryBook.SaveSession(name);
        }
        textArgs.clear();
        break;
      case bookFunctionBranchSession:
        // saves the session and creates a new one from current place in time
        if (textArgs.empty()) {
          intArg = (lint)QueryBook.BranchSession();
        } else {
          const string& name = textArgs[0];
          intArg = (lint)QueryBook.BranchSession(name);
        }
        textArgs.clear();
        break;
      case bookFunctionLoadSession:
        // load session or continue last played session if no name given
        if (textArgs.empty()) {
          intArg = (lint)QueryBook.LoadSession();
        } else {
          const string& name = textArgs[0];
          intArg = (lint)QueryBook.LoadSession(name);
          textArgs.clear();
        }
        break;
      case bookFunctionNewSession:
        // start new session
        textArgs.clear();
        intArg = (lint)QueryBook.NewSession();
        break;
      case bookFunctionBookmark:
        // create a bookmark using the argument to produce the text
        QueryBook.SetBookmark(FunctionArgs);
        intArg = 1;
        textArgs.clear();
        break;
      case bookFunctionUserBookmark:
        // create a bookmark using the argument to produce the text
        // overwrites any previous bookmark at that place
        {
          string bookmarkDesc;
          for (const string& arg : textArgs) {
            bookmarkDesc += arg;
          }
          QueryBook.SetBookmark(bookmarkDesc);
        }
        intArg = 1;
        textArgs.clear();
        break;
      case bookFunctionLoadSnapshot:
        // Load the given snapshot
        intArg = 0;
        if (!textArgs.empty()) {
          intArg = (lint)QueryBook.LoadSnapshot(textArgs[0]);
          textArgs.clear();
        }
        break;
      case bookFunctionGetSnapshots:
        // return snapshots with descriptions, range based on the int value
        textArgs.clear();
        QueryBook.GetSnapshots(FunctionArgs);
        break;
      case bookFunctionGetBookmarks:
        // return snapshots with bookmarks only
        textArgs.clear();
        QueryBook.GetBookmarks(FunctionArgs);
        break;
      case bookFunctionGetSnapshotIndex:
        // return current snapshot index
        intArg = QueryBook.GetCurrentSnapshot();
        textArgs.clear();
        break;
      case bookFunctionDialog:
        // create a dialog
        intArg = 1;
        for (const string& arg : textArgs) {
          Dialog dialog;
          if (CreateDialog(arg, dialog)) {
            QueryBook.AddDialog(dialog);
          } else {
            intArg = 0;
          }
        }
        textArgs.clear();
        break;
      case bookFunctionInput:
        // create an input dialog that saves user response in the noun
        intArg = 1;
        for (const string& arg : textArgs) {
          Dialog dialog;
          dialog.InputBox = true;
          if (CreateDialog(arg, dialog)) {
            QueryBook.AddDialog(dialog);
          } else {
            intArg = 0;
          }
        }
        textArgs.clear();
        break;
      default:
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

/** @brief add user values of the noun to the passed in Properties
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

/** @brief add user integer value of the noun to the passed in Properties
  * \return true if value was found in the user values
  */
bool StoryQuery::GetUserInteger(const string& Noun,
                                Properties& Result)
{
  if (QuerySession.GetUserInteger(Noun, Result)) {
    return true;
  }
  // fall back to values as defined in the story
  Result.IntValue += QueryStory.FindPage(Noun).PageValues.IntValue;
  return false;
}
