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
  size_t offset = 0;
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    Order[i] = (boxType)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                 ----
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    Side[i] = (side)IntoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;
  // read this: 0132 3111 1111 0 +000
  //                      ----
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
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
string Layout::GetDefinition()
{
  string result;
  // write this: 0132 3111 1111 0 +000
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Order[i];
  }
  result += ' ';
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Side[i];
  }
  result += ' ';
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Active[i];
  }
  result += ' ';
  result += SizeSpan;
  result += ' ';
  result += Split < 0? '-' : '+';
  // padding to 000
  if (Split < 10) {
    result += "00";
  } else if (Split < 100) {
    result += "0";
  }
  result += abs(Split);
  return result;
};
