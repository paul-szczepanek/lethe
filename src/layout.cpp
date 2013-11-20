#include "layout.h"
#include "reader.h"



/** @brief create layout based on string
 */
bool Layout::Init(const string& Text)
{
  if (Text.size() != (BOX_TYPE_MAX * 3 + 7)) {
    LOG("mangled layout string");
    return false;
  }
  // read this: 0132 3111 1111 0 00
  //            ----
  szt offset = 0;
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Order[i] = (boxType)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 00
  //                 ----
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Side[i] = (side)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 00
  //                      ----
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Active[i] = IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 00
  //                           -
  SizeSpan = IntoInt(Text[offset]);
  offset += 2;
  // read this: 0132 3111 1111 0 00
  //                             --
  Split = IntoReal(CutString(Text, offset, offset+2)) / (real)100;
  return true;
};

void Layout::ChangeSplit(real Change)
{
  Split += Change;
  if (Split < 0.1) {
    Split = 0.1;
  } else if (Split > 0.9) {
    Split = 0.9;
  }
}

/** @brief return the string used to create this layout
 */
const string Layout::GetDefinition() const
{
  string result;
  // write this: 0132 3111 1111 0 00
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    result += IntoString(Order[i]);
  }
  result += ' ';
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    result += IntoString(Side[i]);
  }
  result += ' ';
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    result += IntoString(Active[i]);
  }
  result += ' ';
  result += IntoString(SizeSpan);
  result += ' ';
  const int intSplit = Split * 100;
  // padding to 00
  if (intSplit < 10) {
    result += "0";
  }
  result += IntoString(intSplit);
  return result;
};
