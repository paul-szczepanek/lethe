#ifndef INPUT_H
#define INPUT_H

#include "main.h"

struct MouseState {
  uint X = 0;
  uint Y = 0;
  bool Left = false;
  bool Middle = false;
  bool Right = false;
  bool LeftUp = false;
  bool MiddleUp = false;
  bool RightUp = false;
};

struct KeysState {
  char Letter = '?';
  bool InputMode = false;
  bool KeyPressed = false;
  bool PgUp = false;
  bool PgDown = false;
  bool Menu = false;
  bool SplitGrow = false;
  bool SplitShrink = false;
  bool LayoutToggle = false;
  bool Escape = false;
  bool ImageZoom = false;
  bool Bookmark = false;
  bool Undo = false;
  bool Redo = false;
  bool Quit = false;
};

class Input
{
public:
  Input() { };
  ~Input() { };

  static bool Tick(MouseState& Mouse, KeysState& Keys);
  static real limitFPS(ulint& LastTime);
};

#endif // INPUT_H
