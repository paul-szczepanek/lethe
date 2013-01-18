#ifndef LAYOUT_H
#define LAYOUT_H

#include "main.h"

enum boxType {
  boxMain,
  boxBG,
  boxQuick,
  boxMenu,
  BOX_TYPE_MAX
};

enum side {
  top,
  right,
  bottom,
  left,
  SIDE_MAX
};

class Layout
{
public:
  Layout() { };
  ~Layout() { };

  bool Init(const string& Text);
  string GetDefinition();


public:
  boxType Order[BOX_TYPE_MAX] = { boxMain, boxBG, boxMenu, boxQuick };
  side Side[BOX_TYPE_MAX] = { left, right, right, right };
  bool Active[BOX_TYPE_MAX] = { true, true, true, true };
  size_t SizeSpan = 0;
  int Split = 0;
};

#endif // LAYOUT_H
