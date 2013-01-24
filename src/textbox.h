#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "windowbox.h"
#include "properties.h"

struct PaneState {
  PaneState () { };
  size_t PaneDragY = 0;
  size_t Y = 0;
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
  void SetText(const string& NewText);
  void AddText(const string& NewText);

  void Scroll(lint PaneScroll);
  void Draw();
  void Reset();

  bool Select(MouseState& Mouse, real DeltaTime);
  bool Deselect();
  bool GetSelectedKeyword(string& Keyword);

  inline bool Empty() {
    return Text.empty();
  };

private:
  void ResetText();
  bool BreakText();
  void RefreshPage();
  void RefreshHighlights();


public:
  bool HighlightsDirty = true;
  bool PageDirty = true;
  bool Centered = false;
  size_t PageHeight = 0;
  size_t PageWidth = 0;

  size_t ValidateKeywords = 0; // how far to check the text keywords
  Properties ValidKeywords;

  Rect PageSize;

protected:
  string Text;
  PaneState Pane;
  vector<Font*> Fonts;

private:
  Surface Highlights;
  Surface PageSurface;

  size_t SelectedKeyword = 0;

  vector<TextLine> Lines;
  vector<KeywordMap> Keywords;

  Colour TextColour;
};

inline void ShiftPositions(vector<size_t_pair>& Positions,
                           const size_t Beyond,
                           const size_t Offset)
{
  for (size_t_pair& pos : Positions) {
    if (pos.X >= Beyond) {
      pos.X += Offset;
    }
    if (pos.Y >= Beyond) {
      pos.Y += Offset;
    }
  }
}

#endif // TEXTBOX_H
