#include "textbox.h"
#include "reader.h"
#include "tokens.h"

const Uint16 RIGHT_MARGIN = BLOCK_SIZE;
const real DRAG_TIMEOUT = 0.1;
const size_t MAX_TEXT_SIZE = 65536 / 2;

void TextBox::Init(vector<Font>& TextBoxFonts,
                   const string& Frame,
                   const int Bpp)
{
  WindowBox::Init(Frame, Bpp);
  for (size_t i = 0, fSz = TEXT_STYLE_MAX; i < fSz; ++i) {
    Font& font = TextBoxFonts[min(i, TextBoxFonts.size())];
    Fonts.push_back(&font);
  }
}

void TextBox::Init(Font& TextBoxFont,
                   const string& Frame,
                   const int Bpp)
{
  WindowBox::Init(Frame, Bpp);
  Fonts.resize(TEXT_STYLE_MAX, &TextBoxFont);
}

/** @brief Use the broken text as guide to shrink the box
  */
Rect TextBox::GetTextSize()
{
  Rect fitSize = Size;
  const lint newW = PageWidth + Size.W - PageSize.W + BLOCK_SIZE;
  const lint newH = PageHeight + Size.H - PageSize.H + BLOCK_SIZE;
  // resize if at least one dimension shrunk (but not if the other grew)
  if (newW < Size.W) {
    fitSize.W = newW;
  }
  if (newH < Size.H) {
    fitSize.H = newH;
  }
  return fitSize;
}

/** @brief Resets the cached pages and breaks new text
  */
void TextBox::SetText(const string& NewText)
{
  Text = NewText;
  ResetText();
}

void TextBox::ResetText()
{
  // trim text far off the screen
  if (Text.size() > MAX_TEXT_SIZE) {
    // to avoid constant resets
    const size_t limit = Text.size() - (MAX_TEXT_SIZE / 2);
    const size_t cut = FindCharacter(Text, '\n', limit);
    Text = CutString(Text, cut+1);
  }

  Pane.Y = PageHeight = PageWidth = 0;
  HighlightsDirty = PageDirty = true;
  Keywords.clear();
  Lines.clear();
  BreakText();
}

/** @brief Keeps the text already broken into lines
  */
void TextBox::AddText(const string& NewText)
{
  if (!Text.empty() && Text[Text.size() - 1] != '\n') {
    Text += '\n';
  }
  Text += NewText;
  ResetText();
}

/** @brief recalculates size and marks surfaces dirty
  */
void TextBox::Reset()
{
  PageSize.X = Size.X + BLOCK_SIZE / 2;
  PageSize.Y = Size.Y + BLOCK_SIZE / 2;
  PageSize.W = Size.W - BLOCK_SIZE;
  PageSize.H = Size.H - BLOCK_SIZE;
  ResetText();
}

/** @brief Draw page onto passed in surface
  */
void TextBox::Draw()
{
  if (Visible) {
    RefreshPage();
    RefreshHighlights();
    DrawFrame();
    Highlights.Draw(PageSize);
    PageSurface.Draw(PageSize);
  }
}

/** @brief Get selected keyword
  */
bool TextBox::GetSelectedKeyword(string& Keyword)
{
  if (SelectedKeyword < Keywords.size()) {
    Keyword = Keywords[SelectedKeyword].Keyword;
    SelectedKeyword = Keywords.size();
    HighlightsDirty = true;
    return true;
  }
  return false;
}

/** @brief Select keywords
  * \return true if the mouse is clicked inside of the box
  */
bool TextBox::HandleInput(MouseState& Mouse,
                          const real DeltaTime)
{
  bool inside = (Mouse.Left || Mouse.LeftUp) && Visible;
  size_t newSelected = Keywords.size();
  // no click has the same effect as being outside
  if (inside) {
    if ((Mouse.X < Size.X) || (Mouse.X > Size.X + Size.W)
        || (Mouse.Y < Size.Y) || (Mouse.Y > Size.Y + Size.H)) {
      Pane.DragTimeout = DRAG_TIMEOUT;
      inside = false;
    } else {
      // translate screen position to text surface position
      const lint X = Mouse.X - PageSize.X;
      const lint Y = Mouse.Y - (PageSize.Y - Pane.Y );
      // find selected keyword
      for (size_t i = 0, fSz = Keywords.size(); i < fSz; ++i) {
        if ((Keywords[i].Size.X < X)
            && (Keywords[i].Size.Y < Y)
            && (X < Keywords[i].Size.X + Keywords[i].Size.W)
            && (Y < Keywords[i].Size.Y + Keywords[i].Size.H)) {
          newSelected = i;
          break;
        }
      }

      // if nothing selected try and drag the page instead
      if (newSelected < Keywords.size()) {
        Pane.DragTimeout = DRAG_TIMEOUT;
      } else if (Pane.DragTimeout <= 0.f) {
        Scroll(Pane.PaneDragY - Mouse.Y);
        Pane.PaneDragY = Mouse.Y;
      } else {
        Pane.PaneDragY = Mouse.Y;
        Pane.DragTimeout -= DeltaTime;
      }
    }
  } else {
    Pane.DragTimeout = DRAG_TIMEOUT;
  }

  if (newSelected != SelectedKeyword) {
    // selected keyword has changed, repaint the highlights
    HighlightsDirty = true;
    SelectedKeyword = newSelected;
  }

  return inside;
}

/** @brief Scroll the page by moving the text clipping rectangle
  */
void TextBox::Scroll(lint PaneScroll)
{
  if (PaneScroll != 0) {
    // only scroll if the page is bigger than box
    if ((PaneScroll != 0) && (PageHeight > PageSize.H)) {
      int pagePosition = PaneScroll + Pane.Y;
      if (pagePosition < 0) {
        Pane.Y = 0;
      } else {
        if ((usint)pagePosition > PageHeight - PageSize.H) {
          pagePosition = PageHeight - PageSize.H;
        }
        Pane.Y = pagePosition;
      }
    }

    // adjust the surfaces to the new scroll position
    ShowUp = (Pane.Y > 1);
    ShowDown = ((usint)Pane.Y + 1u + PageSize.H < PageHeight);
    PaneScroll = 0;
    // page moved so need to refresh surfaces
    HighlightsDirty = PageDirty = true;
  }
}

struct FontChange {
  FontChange(Font* _LineFont, size_t _Position)
    : LineFont(_LineFont), Position(_Position) {}
  Font* LineFont;
  size_t Position;
};

/** @brief Word wrap and find keywords, fills in Lines and Keywords
  */
bool TextBox::BreakText()
{
  size_t pos = 0;
  size_t length = Text.size();
  vector<size_t_pair> keywordPos;
  vector<string> keywordNames;
  string cleaned;
  string realName;
  size_t skip = 0; // characters skipped
  // record keywords and their positions and remove syntax symbols
  while(pos < length) {
    // keywordPos store the positions in the cleaned string and need fixing
    size_t_pair namePos = FindToken(Text, token::keyword, pos);
    // quit if no more keywords
    if (namePos.X == string::npos) {
      break;
    }
    const size_t_pair realNamePos = FindToken(Text, token::expression,
                                    pos, namePos.Y);
    // copy up to the keyword
    cleaned += CutString(Text, pos, namePos.X);
    pos = namePos.Y + 1;
    // don't print the real name, but record it
    if (realNamePos.X != string::npos) {
      realName = CutString(Text, realNamePos.X+1, realNamePos.Y);
      namePos.Y = realNamePos.X;
    } else {
      realName = CutString(Text, namePos.X+1, namePos.Y);
    }

    // check keywords found in a position earlier than set threshold to see
    // if they're still valid
    const bool validKeyword = pos < ValidateKeywords ?
                              ValidKeywords.ContainsValue(realName)
                              : true;
    if (validKeyword) {
      keywordNames.push_back(realName);
    }

    // copy the keyword
    cleaned += CutString(Text, namePos.X+1, namePos.Y);
    // fix up the position for after removing the tokens and real name
    namePos.X -= skip;
    namePos.Y -= ++skip; // +1 for opening <
    if (realNamePos.X != string::npos) {
      skip += realNamePos.Y - realNamePos.X + 1;
    }
    ++skip; // +1 for closing >
    if (validKeyword) {
      keywordPos.push_back(namePos);
    }
  }

  // add remaining characters after the last keyword
  if (pos < length) {
    cleaned += CutString(Text, pos);
  }

  vector<FontChange> fontChanges;
  string plain;
  length = cleaned.size();
  pos = 0;
  // record font changes and strip the formatting
  while(pos < length) {
    const size_t_pair& titlePos = FindToken(cleaned, token::styleTitle, pos);
    const size_t_pair& quotePos = FindToken(cleaned, token::styleQuote, pos);
    const size_t_pair& monoPos = FindToken(cleaned, token::styleMono, pos);
    size_t_pair changePos;
    Font* lineFont;

    // find which formatting is closest
    if (titlePos.X < quotePos.X) {
      if (titlePos.X < monoPos.X) {
        changePos = titlePos;
        lineFont = Fonts[styleTitle];
      } else {
        changePos = monoPos;
        lineFont = Fonts[styleMono];
      }
    } else {
      if (quotePos.X < monoPos.X) {
        changePos = quotePos;
        lineFont = Fonts[styleQuote];
      } else {
        changePos = monoPos;
        lineFont = Fonts[styleMono];
      }
    }

    //only bother if there's a non-zero length text
    if (changePos.X == string::npos) {
      // we're done, no more formatting
      plain += CutString(cleaned, pos);
      pos = length;
    } else if (changePos.X != changePos.Y - 3) {
      plain += CutString(cleaned, pos, changePos.X);
      // insert a newline in front to make sure it breaks
      if (!plain.empty()) {
        if (plain[plain.size()-1] == ' ') {
          plain[plain.size()-1] = '\n';
        } else if (plain[plain.size()-1] != '\n') {
          ShiftPositions(keywordPos, plain.size(), 1);
          plain += '\n';
        }
      }
      // move beyond the formatting
      pos = changePos.Y + 1;
      // reuse last entry if it's at the same position
      // this happens if two styles come one after another
      if (!fontChanges.empty() && fontChanges.back().Position == plain.size()) {
        fontChanges.back().LineFont = lineFont;
      } else {
        fontChanges.push_back(FontChange(lineFont, plain.size()));
      }
      // remove the formatting from text
      ShiftPositions(keywordPos, plain.size(), -2);
      plain += CutString(cleaned, changePos.X + 2, changePos.Y - 1);
      ShiftPositions(keywordPos, plain.size(), -2);
      // and another newline at the end of formatting
      if (cleaned.size() > changePos.Y + 1) {
        if (cleaned[changePos.Y + 1] == ' ') {
          plain += '\n';
          ++pos; // newline overwrites the space
        } else if (cleaned[changePos.Y + 1] != '\n') {
          ShiftPositions(keywordPos, plain.size(), -2);
          plain += '\n';
        }
      }
      fontChanges.push_back(FontChange(Fonts[styleMain], plain.size()));
    }
  }

  length = plain.size();
  skip = pos = 0;
  // remove escape characters
  while(pos < length) {
    const char c = plain[pos];
    if (c == '\\') {
      ++skip;
      if (++pos < length) {
        plain[pos - skip] = plain[pos];
      }
      // fix the  keyword positions that are beyond this position
      for (size_t i = 0, fSz = keywordPos.size(); i < fSz; ++i) {
        if (keywordPos[i].X > pos) {
          --keywordPos[i].X;
        }
        if (keywordPos[i].Y > pos) {
          --keywordPos[i].Y;
        }
      }
    } else {
      plain[pos - skip] = c;
    }
    ++pos;
  }

  if (skip > 0) {
    plain = CutString(plain, 0, length - skip);
  }

  bool firstWord = true;
  size_t fontChangeI = 0;
  size_t lastLineEnd = 0;
  size_t lastPos = 0;
  Font* currentFont = Fonts[styleMain];
  size_t oldLineSkip = currentFont->GetLineSkip();
  pos = 0;
  length = plain.size();

  // break the text into lines that fit within the text box width
  while(pos < length) {
    // did we hit a font change?
    if (fontChangeI < fontChanges.size()
        && fontChanges[fontChangeI].Position <= pos) {
      // change font and ready for next change
      currentFont = fontChanges[fontChangeI].LineFont;
      ++fontChangeI;
    }
    bool flush = false;
    //find space
    size_t space = plain.find(' ', pos);
    // if end of text
    if (space == string::npos) {
      space = length;
      flush = true;
    }
    // find newline
    size_t newline = plain.find('\n', pos);
    if (newline == string::npos) {
      newline = length;
    }
    // what's first, newline or space?
    if (newline < space) {
      pos = newline;
      //TODO we can't replace with space because of possible roll-back
      //CleanText[pos] = ' '; // replace with a space
      flush = true;
    } else {
      pos = space;
    }

    // test print the line
    string line = CutString(plain, lastLineEnd, pos);
    // did we fit in?
    if (currentFont->GetWidth(line) < PageSize.W || firstWord) {
      lastPos = pos;
    } else {
      // overflow, revert to last position and print that
      flush = true;
      pos = lastPos; // include the character so the sizes match
      line = CutString(plain, lastLineEnd, pos);
    }

    ++pos;
    firstWord = false;
    if (flush || !(pos < length)) { // || in case there's a space at the end
      // use the larger line skip
      const size_t lineSkip = currentFont->GetLineSkip();
      PageHeight += max(oldLineSkip, lineSkip);
      oldLineSkip = lineSkip;
      // size up the line
      Rect lineSize;
      lineSize.W = currentFont->GetWidth(line);
      lineSize.H = currentFont->GetHeight();
      lineSize.X = Centered ? (PageSize.W - lineSize.W) / 2 : 0;
      lineSize.Y = PageHeight;
      PageWidth = max(PageWidth, lineSize.W);
      // ready for the next line
      PageHeight += lineSize.H;
      firstWord = true;
      lastLineEnd = pos;
      // create new line ready for printing
      Lines.push_back(TextLine(line, currentFont, lineSize));
    }
  }

  lastLineEnd = 0;
  // record visual keyword positions
  for (size_t i = 0, fSz = Lines.size(); i < fSz; ++i) {
    const string& lineText = Lines[i].Text;
    const Font& lineFont = *(Lines[i].LineFont);
    const Rect& lineSize = Lines[i].Size;
    const size_t lineLength = lineText.size() + 1;
    const size_t lineEnd = lastLineEnd + lineLength;

    for (size_t j = 0, fSz = keywordPos.size(); j < fSz; ++j) {
      const size_t_pair& keyPos = keywordPos[j];
      // check if this keyword is on this line
      if (keyPos.X >= lineEnd) {
        // since this keyword begins after the current line we can stop looking
        // since the next keyword will be even farther
        break;
      } else if (keyPos.Y > lastLineEnd) {
        // there may be multiple keywords referencing the same noun
        // they can also be split across multiple lines
        KeywordMap newKey(keywordNames[j]);
        // get positions relative to line start
        const size_t beg = keyPos.X < lastLineEnd? 0 : keyPos.X - lastLineEnd;
        const size_t end = keyPos.Y > lineEnd?
                           lineLength : keyPos.Y - lastLineEnd;
        // find where the keyword starts
        newKey.Size.X = lineSize.X;
        if (beg) {
          const string& keywordStart = CutString(lineText, 0, beg);
          newKey.Size.X += lineFont.GetWidth(keywordStart);
        }
        newKey.Size.Y = lineSize.Y;
        // find where the keyword ends
        const string& keywordEnd = CutString(lineText, beg, end);
        newKey.Size.W = lineFont.GetWidth(keywordEnd);
        newKey.Size.H = lineSize.H;
        Keywords.push_back(newKey);
      }
    }
    lastLineEnd = lineEnd;
  }

  SelectedKeyword = Keywords.size();
  HighlightsDirty = PageDirty = true;
  // scroll to end
  Pane.Y = PageHeight > PageSize.H? (PageHeight - PageSize.H) : 0;

  return (PageHeight > 0);
}

/** @brief returns the surface with word wrapped text of the current page
  * updates the page if needed, otherwise returns the cached page
  */
void TextBox::RefreshPage()
{
  if (PageDirty) {
    PageDirty = false;
    if (PageHeight) {
      PageSurface.Init(PageSize.W, PageSize.H);
      for (size_t i = 0, fSz = Lines.size(); i < fSz; ++i) {
        TextLine& line =  Lines[i];
        Rect offsetLocation = line.Size;
        offsetLocation.Y -= Pane.Y;
        if (offsetLocation.Y > -(lint)offsetLocation.H
            && offsetLocation.Y < (lint)PageSize.H) {
          PageSurface.PrintText(offsetLocation, *line.LineFont, line.Text,
                                TextColour.R, TextColour.G, TextColour.B);
        }
      }
    }
  }
}

/** @brief returns the surface with the backdrop and highlights for the keywords
  * only updates if required, otherwise returns cached
  */
void TextBox::RefreshHighlights()
{
  if (HighlightsDirty) {
    HighlightsDirty = false;
    if (PageHeight) {
      Highlights.Init(PageSize.W, PageSize.H);
      // paint a rectangle behind each keyword
      for (size_t i = 0, forSize = Keywords.size(); i < forSize; ++i) {
        // change colour for the selected keyword
        usint highlightColour = (i == SelectedKeyword) ? 150 : 50;
        Rect offsetLocation = Keywords[i].Size;
        offsetLocation.Y -= Pane.Y;
        if (offsetLocation.Y > -(lint)offsetLocation.H
            && offsetLocation.Y < (lint)PageSize.H) {
          Highlights.DrawRectangle(offsetLocation, highlightColour, 150, 150);
        }
      }
    }
  }
}
