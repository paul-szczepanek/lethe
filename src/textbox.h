#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "windowbox.h"

struct PaneState {
  PaneState ()
    : PaneDragY(0), PaneScroll(0), DragTimeout(0.25),
      PaneDown(false), PaneUp(false) { };
  ~PaneState () { };
  Uint16 PaneDragY;
  int PaneScroll;
  real DragTimeout;
  bool PaneDown;
  bool PaneUp;
};

struct KeywordMap {
  KeywordMap(const string& keyName)
    : Keyword(keyName), X(0), Y(0), W(0), H(0) { };
  ~KeywordMap() { };
  string Keyword;
  uint X;
  uint Y;
  uint W;
  uint H;
};

class MouseState;

class TextBox : public WindowBox
{
public:
  TextBox() { };
  virtual ~TextBox() {
    ResetPage();
  };

  uint_pair GetMaxSize();
  void SetText(string NewText);

  void Draw(SDL_Surface* Screen);
  void Reset();
  void ResetPage();

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

private:
  void Scroll();
  bool BreakText();
  SDL_Surface* GetPageSurface();
  SDL_Surface* GetPageTextSurface();

public:
  bool PageDirty = false;

  PaneState Pane;

private:
  string Text;

  SDL_Surface* PageSurface = NULL;
  SDL_Surface* PageTextSurface = NULL;

  SDL_Rect TextClip;
  SDL_Rect TextDst;

  uint PageHeight = 0;
  uint LineHeight = 0;

  vector<string> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
