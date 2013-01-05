#include "storyquery.h"
#include "tokens.h"
#include "properties.h"
#include "page.h"
#include "story.h"
#include "session.h"

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
      for (size_t i = 0, forSz = CurBlock.Blocks.size(); i < forSz; ++i) {
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

/** @brief Executes all functions on all argument values
  *
  */
bool StoryQuery::ExecuteFunction(const Properties& FunctionName,
                                 Properties& FunctionArgs)
{
  for (const string& func : FunctionName.TextValues) {
    // execute function
    if (func == "Size") { // return number of values
      FunctionArgs.IntValue = 0;
      for (const string& arg : FunctionArgs.TextValues) {
        const Properties& nounValues = Progress.IsUserValues(arg)?
                                       Progress.UserValues[arg]
                                       : StoryDef.FindPage(arg).PageValues;
        FunctionArgs.IntValue += nounValues.TextValues.size();
      }
      FunctionArgs.TextValues.clear();
    } else if (func == "Play") {
      for (const string& arg : FunctionArgs.TextValues) {
        LOG(arg + " - play");
      }
    } else if (func == "Stop") {
      for (const string& arg : FunctionArgs.TextValues) {
        LOG(arg + " - stop");
      }
    } else if (func == "List") { // print a list of values
      FunctionArgs.IntValue = 0;
      for (const string& arg : FunctionArgs.TextValues) {
        const Properties& nounValues = Progress.IsUserValues(arg)?
                                       Progress.UserValues[arg]
                                       : StoryDef.FindPage(arg).PageValues;
        bool first = true;
        for (const string& keyword : nounValues.TextValues) {
          if (first) {
            first = false;
            Text += "<";
          } else {
            Text += ", <";
          }
          Text += keyword;
          Text += ">";
          ++FunctionArgs.IntValue;
        }
      }
    } else {
      LOG(func + " - function doesn't exist!");
      return false;
    }
  }
  return true;
}

struct OperationNode {
  token::operationName Operation = token::plus;
  Properties Operand;
  bool Nested = true; //doesn't have operand, copy from result of next
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
  size_t valueEnd = 0; //needed because function names vary in length

  vector<OperationNode> opStack;
  opStack.reserve(16); // too avoid constant allocs

  // parse expressions like a+@b-func(arg)-#1
  // into an array of pairs like this
  // +,a  +,_ <- @,b  -,func_ <- (,arg  -,_ <- #,1
  // _ <- means that the next pair will write its result there
  // in case of functions it will replace the values that are the func names

  // it's more efficient to scan char by char
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
              const string& funcArgs = cutString(Expression, pos + 1, funcEnd);
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
      const string& valueText = cutString(Expression, valuePos, valueEnd);

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
        break;
      case token::evaluate:
        // evaluate all text into noun values, add them up and reuse the value
        for (string text : operand.TextValues) {
          // try to find the Values of this noun, user values first
          GetUserValues(text, target);
        }
        break;
      case token::integer:
        // evaluate all text into int values, add them up and reuse the value
        for (string numberText : operand.TextValues) {
          // check for a plain text number (keywords can't start with a digit)
          if (isdigit(numberText[0])) {
            target.IntValue += intoInt(numberText);
          } else { // this a noun, find its value
            GetUserInteger(numberText, operand);
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

  if (opStack.empty()) {
    return false;
  }

  return true;
}

/** @brief add user values of the noun to the passed in Properites
  * \return true if value was found in the user values
  */
bool StoryQuery::GetUserValues(const string& Noun,
                               Properties& Result)
{
  if (Progress.GetUserValues(Noun, Result)) {
    return true;
  }
  // fall back to values as defined in the story
  Result.AddValues(StoryDef.FindPage(Noun).PageValues);
  return false;
}

/** @brief add user integer value of the noun to the passed in Properites
  * \return true if value was found in the user values
  */
bool StoryQuery::GetUserInteger(const string& Noun,
                                Properties& Result)
{
  if (Progress.GetUserInteger(Noun, Result)) {
    return true;
  }
  // fall back to values as defined in the story
  Result.IntValue = StoryDef.FindPage(Noun).PageValues.IntValue;
  return false;
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

        const string& left = cutString(Expression, tokenPos.X+2, opPos.X);
        const string& right = cutString(Expression, opPos.Y+1, tokenPos.Y);

        // default to current page noun
        if (left.empty()) {
          leftValues.AddValue(Noun);
        } else {
          EvaluateExpression(leftValues, left, isNum, isText);
        }

        EvaluateExpression(rightValues, right, isNum, isText);

        // assign right values to all nouns whose name is in left values
        for (const string& text : leftValues.TextValues) {
          // copy the story values if they haven't been created for the user yet
          if (!Progress.IsUserValues(text)) {
            Progress.UserValues[text] = StoryDef.FindPage(text).PageValues;
          }
          Properties& userValues = Progress.UserValues[text];

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
        Properties leftValues;
        Properties rightValues;

        const string& left = cutString(Expression, tokenPos.X+2, opPos.X);
        const string& right = cutString(Expression, opPos.Y+1, tokenPos.Y);

        // default to current page noun
        if (left.empty()) {
          leftValues = Progress.IsUserValues(Noun)?
                       Progress.UserValues[Noun]
                       : StoryDef.FindPage(Noun).PageValues;
        } else {
          EvaluateExpression(leftValues, left, isNum, isText);
        }

        EvaluateExpression(rightValues, right, isNum, isText);

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
      const string evaluate = cutString(Expression, tokenPos.X+2, tokenPos.Y);
      bool isNum, isText;
      Properties values;
      EvaluateExpression(values, evaluate, isNum, isText);

      // executy [!noun:verb] commands
      for (const string& text : values.TextValues) {
        const size_t scopePos = FindTokenStart(text, token::scope);
        if (scopePos != string::npos) { // recurse with the new block
          string noun = cutString(text, 0, scopePos);
          const string verbName = cutString(text, scopePos+1);

          // if no noun, use the current page noun by default
          if (noun.empty()) {
            noun = Noun;
          }
          const Page& page = StoryDef.FindPage(noun);
          const Block& block = page.GetVerb(verbName).BlockTree;

          result &= ExecuteBlock(noun, block);
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
