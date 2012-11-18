#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "main.h"

struct PaneState {
  PaneState ()
    : PaneDragY(0), PaneScroll(0), DragTimeout(0),
      PaneDown(false), PaneUp(false), PaneDrag(false) { };
  ~PaneState () { };
  Uint16 PaneDragY;
  int PaneScroll;
  real DragTimeout;
  bool PaneDown;
  bool PaneUp;
  bool PaneDrag;
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

class TextBox
{
public:
  TextBox();
  virtual ~TextBox();

  void Draw(SDL_Surface* Screen);

  void SetSize(Rect& NewSize);
  void SetText(string NewText, TTF_Font* Font = NULL);
  void Scroll();

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

  SDL_Surface* GetPageSurface();
  SDL_Surface* GetPageTextSurface();

private:
  bool BreakText();
  void ResetPage();

public:
  bool Active;
  bool Visible;
  bool PageDirty;

  PaneState Pane;

private:
  Rect Size;

  string Text;
  TTF_Font* FontMain;

  SDL_Surface* PageSurface;
  SDL_Surface* PageTextSurface;

  SDL_Rect DstRect;
  SDL_Rect ClipRect;
  uint PageHeight;
  uint LineHeight;

  vector<string> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
