#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include "textbox.h"

class DialogBox : public TextBox
{
public:
  DialogBox() { };
  ~DialogBox() { };

  size_t_pair GetMaxSize();
  void Draw();
  void DrawInput();
  void AddCharacter(const char NewChar);

  void EnableInput(const string& Noun, const string& Value = "");

public:
  string InputNoun;
  string InputText;

  Surface InputSurface;

  bool InputDirty = false;
  bool Input = false;
};

#endif // DIALOGBOX_H
