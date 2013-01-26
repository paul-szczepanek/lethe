#include "dialogbox.h"
#include "tokens.h"

/** @brief Draw page onto passed in surface
  */
void DialogBox::DrawInput()
{
  if (InputDirty) {
    string text = InputText + "_";
    // scroll text offscreen if it doesn't fit
    while (!text.empty() && Fonts[styleMain]->GetWidth(text) > PageSize.W) {
      // keep eating a character from the left until we fit
      text = "..." + CutString(text, 4);
    }
    InputSurface.CreateText(*Fonts[styleMain], text, 255, 255, 255);
    InputDirty = false;
  }
  InputSurface.Draw(PageSize);
}

void DialogBox::EnableInput(const string& Noun,
                            const string& Value)
{
  InputNoun = Noun;
  InputText = Value;
  InputBox = InputDirty = true;
}

void DialogBox::AddCharacter(const char NewChar)
{
  if (InputBox) {
    InputDirty = true;
    if (NewChar != BACKSPACE_CHAR) {
      InputText += NewChar;
    } else if (InputText.size()) {
      InputText.resize(InputText.size() - 1);
    }
  }
}

/** @brief Draw page onto passed in surface
  */
void DialogBox::Draw()
{
  if (Visible) {
    TextBox::Draw();
    if (InputBox) {
      DrawInput();
    }
  }
}
