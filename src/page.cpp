#include "page.h"
#include "tokens.h"
#include "pageparser.h"

VerbBlock Page::MissingVerb = { "", Block("You can't do that.", false), { } };

void Page::Parse(const string& SourceText)
{
  PageParser(SourceText, *this);
}

/** @brief Get the reference to the top Block node of the verb
  *
  * \return the block belonging to a verb by the name passed in
  */
const VerbBlock& Page::GetVerb(const string& Verb) const
{
  for (szt i = 0, fSz = Verbs.size(); i < fSz; ++i) {
    for (szt j = 0, fSzj = Verbs[i].Names.size(); j < fSzj; ++j) {
      if (Verbs[i].Names[j] == Verb) {
        return Verbs[i];
      }
    }
  }
  LOG("Verb: " + Verb + " missing.")
  MissingVerb.BlockTree.Expression = "You can't " + Verb + " that.";
  return MissingVerb;
}
