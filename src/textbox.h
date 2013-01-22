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

struct TextLine {
  TextLine(const string& _Text, size_t _X = 0) : Text(_Text), X(_X) { };
  string Text;
  size_t X;
};

class MouseState;

class TextBox : public WindowBox
{
public:
  TextBox() { };
  virtual ~TextBox() { };

  size_t_pair GetMaxSize();
  void SetText(const string NewText);

  void Draw();
  void Reset();

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

  inline bool Empty() { return Text.empty(); };

private:
  void Scroll();
  bool BreakText();
  void RefreshPage();
  void RefreshHighlights();


public:
  bool HighlightsDirty = true;
  bool PageDirty = true;
  bool Centered = false;

  PaneState Pane;

  size_t ValidateKeywords = 0; // how far to check the text keywords
  Properties ValidKeywords;

  size_t PageHeight = 0;
  size_t LineHeight = 0;

protected:
  string Text;
  Rect PageSize;
  Rect PageClip;

private:
  Surface Highlights;
  Surface PageSurface;

  vector<TextLine> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
