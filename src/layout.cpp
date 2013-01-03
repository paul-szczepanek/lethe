#include "layout.h"

/** @brief create layout based on string
 */
bool Layout::Init(const string& Text)
{
  if (Text.size() != (BOX_TYPE_MAX * 3 + 9)) {
    LOG("mangled layout string");
    return false;
  }

  size_t offset = 0;

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    Order[i] = (boxType)intoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    Side[i] = (side)intoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    Active[i] = intoInt(Text[offset+i]);
  }
  offset += BOX_TYPE_MAX;
  ++offset;

  SizeSpan = intoInt(Text[offset]);
  ++offset;

  Split = intoInt(Text[offset]);
  ++offset;

  Split = intoInt(cutString(Text, offset+1, offset+3));

  if (Text[offset] == '-') {
    Split = -Split;
  }

  return true;
};

/** @brief return the string used to create this layout
 */
string Layout::GetDefinition()
{
  string result;

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Order[i];
  }
  result += "";

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Side[i];
  }
  result += "";

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    result += Active[i];
  }
  result += "";

  result += SizeSpan;
  result += "";

  result += Split < 0? '-' : '+';

  // padding 000
  if (Split < 10) {
    result += "00";
  } else if (Split < 100) {
    result += "0";
  }

  result += abs(Split);

  return result;
};
