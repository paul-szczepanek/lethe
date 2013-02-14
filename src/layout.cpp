#include "layout.h"

/** @brief create layout based on string
 */
bool Layout::Init(const string& Text)
{
  if (Text.size() != (BOX_TYPE_MAX * 3 + 9)) {
    LOG("mangled layout string");
    return false;
  }
  // read this: 0132 3111 1111 0 +000
  //            ----
  szt offset = 0;
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Order[i] = (boxType)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                 ----
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Side[i] = (side)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                      ----
  for (szt i = 0, fSz = BOX_TYPE_MAX; i < fSz; ++i) {
    Active[i] = IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                           -
  SizeSpan = IntoInt(Text[offset]);
  offset += 2;
  // read this: 0132 3111 1111 0 +000
  //                             -
  const char sign = Text[offset];
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                              ---
  Split = IntoInt(CutString(Text, offset+1, offset+3));
  // fix the negative value
  if (sign == '-') {
    Split = -Split;
  }
  return true;
};

/** @brief return the string used to create this layout
 */
const string Layout::GetDefinition() const
{
  string result;
  // write this: 0132 3111 1111 0 +000
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
  result += Split < 0? '-' : '+';
  // padding to 000
  if (Split < 10) {
    result += "00";
  } else if (Split < 100) {
    result += "0";
  }
  result += IntoString(Abs(Split));
  return result;
};
