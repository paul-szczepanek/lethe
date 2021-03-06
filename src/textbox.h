#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "windowbox.h"
#include "properties.h"

struct PaneState {
  PaneState() { };
  lint PaneDragY = 0;
  lint Y = 0;
  real DragTimeout = 0.25;
  bool PaneDown = false;
  bool PaneUp = false;
};

struct KeywordMap {
  KeywordMap(const string& keyName)
    : Keyword(keyName) { };
  const string Keyword;
  Rect Size;
  bool Active;
};

struct TextLine {
  TextLine(const string& aText, const Font* aLineFont, const Rect aSize)
    : Text(aText), LineFont(aLineFont), Size(aSize) { };
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

  void SetText(const string& NewText);
  void AddText(const string& NewText);
  Rect GetTextSize();

  void Scroll(lint PaneScroll);
  virtual void Draw();
  void Reset();

  bool HandleInput(MouseState& Mouse, const real DeltaTime);
  bool GetSelectedKeyword(string& Keyword);

  inline bool Empty()
  {
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
  bool CentreMain = false;
  lint PageHeight = 0;
  lint PageWidth = 0;

  szt ValidateKeywords = 0; // how far to check the text keywords
  Properties ValidKeywords;

  Rect PageSize;
  bool RawMode = false;

protected:
  string Text;
  PaneState Pane;
  vector<Font*> Fonts;

private:
  Surface Highlights;
  Surface PageSurface;

  szt SelectedKeyword = 0;

  vector<TextLine> Lines;
  vector<KeywordMap> Keywords;

  Colour TextColour;
};

inline void ShiftPositions(vector<szt_pair>& Positions,
                           cszt Beyond,
                           cszt Offset)
{
  for (szt_pair& pos : Positions) {
    if (pos.X > Beyond) {
      pos.X += Offset;
    }
    if (pos.Y > Beyond) {
      pos.Y += Offset;
    }
  }
}

#endif // TEXTBOX_H
