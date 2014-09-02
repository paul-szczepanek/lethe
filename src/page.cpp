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
      const string& name = Verbs[i].Names[j];
      if (name == Verb) {
        return Verbs[i];
      }
    }
  }
  LOG("Verb: " + Verb + " missing.")
  MissingVerb.BlockTree.Expression = "You can't " + Verb + " that.";
  return MissingVerb;
}

/** @brief This will add a new verb and remove all previous verbs that share
  * any names with the newly defined one
  */
void Page::AddVerb(const VerbBlock& Verb)
{
  for (szt h = 0, fSzk = Verb.Names.size(); h < fSzk; ++h) {
    for (szt i = 0; i < Verbs.size(); ++i) {
      for (szt j = 0, fSzj = Verbs[i].Names.size(); j < fSzj; ++j) {
        if (Verbs[i].Names[j] == Verb.Names[h]) {
          Verbs.erase(Verbs.begin() + i);
          // evaluate the same position again after erasing it
          --i;
          break;
        }
      }
    }
  }

  Verbs.push_back(Verb);
}

