#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "main.h"

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

class TextBox
{
public:
  TextBox() { };
  virtual ~TextBox();

  void Init(TTF_Font* Font = NULL, const string& Frame = "", int Bpp = 32);
  void SetSize(Rect& NewSize);
  uint_pair GetMaxSize();
  void SetText(string NewText);

  void Draw(SDL_Surface* Screen);

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

private:
  bool DrawFrame(SDL_Surface* Screen);
  void Scroll();
  bool BreakText();
  void ResetPage();
  SDL_Surface* GetPageSurface();
  SDL_Surface* GetPageTextSurface();

public:
  bool Active = false;
  bool Visible = false;
  bool PageDirty = false;

  PaneState Pane;

private:
  Rect Size;
  int BPP;

  string FrameName;
  string Text;
  TTF_Font* FontMain = NULL;

  SDL_Surface* PageSurface = NULL;
  SDL_Surface* PageTextSurface = NULL;
  SDL_Surface* FrameSurface = NULL;
  SDL_Surface* FrameDown = NULL;
  SDL_Surface* FrameUp = NULL;
  SDL_Surface* FrameIcon = NULL;

  SDL_Rect FrameDst;
  SDL_Rect TextClip;
  SDL_Rect TextDst;
  SDL_Rect UpDst;
  SDL_Rect DownDst;

  uint PageHeight = 0;
  uint LineHeight = 0;

  vector<string> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
