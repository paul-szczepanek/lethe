#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "windowbox.h"
#include "properties.h"

struct PaneState {
  PaneState () { };
  size_t PaneDragY = 0;
  int PaneScroll = 0;
  real DragTimeout = 0.25;
  bool PaneDown = false;
  bool PaneUp = false;
};

struct KeywordMap {
  KeywordMap(const string& keyName)
    : Keyword(keyName) { };
  const string Keyword;
  Rect Size;
};

struct TextLine {
  TextLine(const string& _Text, const Font* _LineFont, const Rect _Size)
    : Text(_Text), LineFont(_LineFont), Size(_Size) { };
  const string Text;
  const Font* LineFont;
  const Rect Size;
  Surface LineSurface;
};

class MouseState;

class TextBox : public WindowBox
{
public:
  TextBox() { };
  virtual ~TextBox() { };

  void Init(vector<Font>& TextBoxFonts, const string& Frame,
            const int Bpp);
  void Init(Font& TextBoxFont, const string& Frame, const int Bpp);

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

  vector<Font*> Fonts;

private:
  Surface Highlights;
  Surface PageSurface;

  vector<TextLine> Lines;
  vector<KeywordMap> Keywords;

  size_t SelectedKeyword;
};

#endif // TEXTBOX_H
