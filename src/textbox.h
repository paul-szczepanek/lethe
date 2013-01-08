#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "windowbox.h"
#include "properties.h"

struct PaneState {
  PaneState ()
    : PaneDragY(0), PaneScroll(0), DragTimeout(0.25),
      PaneDown(false), PaneUp(false) { };
  ~PaneState () { };
  size_t PaneDragY;
  int PaneScroll;
  real DragTimeout;
  bool PaneDown;
  bool PaneUp;
};

struct KeywordMap {
  KeywordMap(const string& keyName)
    : Keyword(keyName) { };
  ~KeywordMap() { };
  string Keyword;
  Rect Size;
};

class MouseState;

class TextBox : public WindowBox
{
public:
  TextBox() { };
  virtual ~TextBox() { };

  size_t_pair GetMaxSize();
  void SetText(string NewText);

  void Draw();
  void Reset();

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

private:
  void Scroll();
  bool BreakText();
  void RefreshPage();
  void RefreshHighlights();

public:
  bool HighlightsDirty = true;
  bool PageDirty = true;

  PaneState Pane;

  size_t ValidateKeywords = 0; // how far to check the text keywords
  Properties ValidKeywords;

private:
  string Text;
  Rect PageSize;
  Rect PageClip;

  Surface Highlights;
  Surface PageSurface;

  size_t PageHeight = 0;
  size_t LineHeight = 0;

  vector<string> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
