#ifndef BUTTONBOX_H
#define BUTTONBOX_H

#include "windowbox.h"
#include "properties.h"
#include "tokens.h"

class MouseState;

struct Button {
  void Deselect()
  {
    if (State) {
      --State;
    } else {
      State = ButtonSurfaces.size() - 1;
    }
  };
  void Select()
  {
    ++State;
    State %= ButtonSurfaces.size();
  };
  Button(const buttonType _Function);
  vector<Surface> ButtonSurfaces;
  Rect Size;
  string Name;
  szt State = 0;
  buttonType Function;
  bool Visible = true;
};

class ButtonBox : public WindowBox
{
public:
  ButtonBox() { };
  ~ButtonBox() { };

  void Draw();
  void Reset();
  bool HandleInput(MouseState& Mouse);

  bool GetSelectedButton(buttonType& Selected, szt& State);
  bool SetButtonState(const buttonType Selected, cszt State);
  void AddButton(buttonType NewButton);


private:
  vector<Button> Buttons;
  szt SelectedButton;
};

#endif // BUTTONBOX_H
