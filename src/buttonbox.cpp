#include "buttonbox.h"
#include "input.h"
#include "disk.h"

Button::Button(const buttonType aFunction) : Function(aFunction)
{
  const string& stem = ButtonTypeNames[Function];
  vector<string> surfaceNames = Disk::GetFileSeries(BUTTONS_DIR, stem);

  ButtonSurfaces.resize(surfaceNames.size());
  for (szt i = 0, fSz = surfaceNames.size(); i < fSz; ++i) {
    const string& name = surfaceNames[i];
    Surface& newSurface = ButtonSurfaces[i];
    newSurface.LoadImage(BUTTONS_DIR + SLASH + name);
    newSurface.Resize(GRID + GRID);
    Size.W = newSurface.W;
    Size.H = newSurface.H;
  }
};

/** @brief Select keywords
  * \return true if we hit a keyword
  */
bool ButtonBox::HandleInput(MouseState& Mouse)
{
  bool inside = (Mouse.Left || Mouse.LeftUp) && Visible;
  szt newSelectedButton = Buttons.size();
  if (inside) {
    if ((Mouse.X < Size.X) || (Mouse.X > Size.X + Size.W)
        || (Mouse.Y < Size.Y) || (Mouse.Y > Size.Y + Size.H)) {
      inside = false;
    } else {
      // find the clicked button
      for (szt i = 0, fSz = Buttons.size(); i < fSz; ++i) {
        const Button& button = Buttons[i];
        if (button.Visible && (button.Size.X < Mouse.X)
            && (button.Size.Y < Mouse.Y)
            && (Mouse.X < button.Size.X + button.Size.W)
            && (Mouse.Y < button.Size.Y + button.Size.H)) {
          newSelectedButton = i;
          break;
        }
      }
    }
  }

  // cycle the clicked button and revert the old button
  if (newSelectedButton != SelectedButton) {
    // revert last button state change
    if (SelectedButton < Buttons.size()) {
      Buttons[SelectedButton].Deselect();
    }
    SelectedButton = newSelectedButton;
    // change state of selected button
    if (SelectedButton < Buttons.size()) {
      Buttons[SelectedButton].Select();
    }
  }

  return inside;
}

bool ButtonBox::SetButtonState(const buttonType Selected,
                               cszt State)
{
  for (Button& button : Buttons) {
    if (button.Function == Selected) {
      button.State = State;
      return true;
    }
  }
  return false;
}

/** @brief Return the currently selected button and make the state change hold
  */
bool ButtonBox::GetSelectedButton(buttonType& Selected,
                                  szt& State)
{
  if (SelectedButton < Buttons.size()) {
    Selected = Buttons[SelectedButton].Function;
    State = Buttons[SelectedButton].State;
    SelectedButton = Buttons.size();
    return true;
  }
  return false;
}

/** @brief Add the button to the box, this box will now change
  * the state of the button if it's clicked
  */
void ButtonBox::AddButton(buttonType NewButton)
{
  Buttons.push_back(NewButton);
}

void ButtonBox::Draw()
{
  if (Visible) {
    DrawFrame();
    for (szt i = 0, fSz = Buttons.size(); i < fSz; ++i) {
      Button& button = Buttons[i];
      if (button.Visible) {
        button.ButtonSurfaces[button.State].Draw(button.Size);
      }
    }
  }
}

void ButtonBox::Reset()
{
  // how to arrange buttons - row or column
  bool vertical = (Size.W < Size.H);
  // measure how much space we need for buttons
  lint allW = 0;
  lint allH = 0;
  lint maxW = 0;
  lint maxH = 0;
  for (szt i = 0, fSz = Buttons.size(); i < fSz; ++i) {
    Button& button = Buttons[i];
    // hide the ones we can't fit
    if ((vertical && allH + button.Size.H > Size.H)
        || allW + button.Size.W > Size.W) {
      button.Visible = false;
    } else {
      button.Visible = true;
      allW += button.Size.W;
      allH += button.Size.H;
      maxW = max(maxW, button.Size.W);
      maxH = max(maxH, button.Size.H);
    }
  }

  lint X = Size.X;
  lint Y = Size.Y;
  const real scaleW = vertical?
                      (real)Size.W / (real)maxW
                      : (real)Size.W / (real)allW;
  const real scaleH = vertical?
                      (real)Size.H / (real)allH
                      : (real)Size.H / (real)maxH;
  for (szt i = 0, fSz = Buttons.size(); i < fSz; ++i) {
    Button& button = Buttons[i];
    button.Size.X = X;
    button.Size.Y = Y;
    button.Size.W = button.Size.W * scaleW;
    button.Size.H = button.Size.H * scaleH;
    if (vertical) {
      Y += button.Size.H;
    } else {
      X += button.Size.W;
    }
    if (!button.ButtonSurfaces.empty()) {
      button.Size.X += Abs(button.Size.W - button.ButtonSurfaces.back().W) / 2;
      button.Size.Y += Abs(button.Size.H - button.ButtonSurfaces.back().H) / 2;
    }
    button.Size.W = button.Size.H = min(button.Size.W, button.Size.H);
  }
}
