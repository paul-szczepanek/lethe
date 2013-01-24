#include "textbox.h"
#include "reader.h"
#include "tokens.h"

const Uint16 RIGHT_MARGIN = BLOCK_SIZE;
const real DRAG_TIMEOUT = 0.1;

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

/** @brief Resets the cached pages and breaks new text
  */
void TextBox::SetText(const string NewText)
{
  Text = NewText;
  Reset();
  BreakText();
}

/** @brief recalculates size and marks surfaces dirty
  */
void TextBox::Reset()
{
  HighlightsDirty = PageDirty = true;
  Keywords.clear();
  PageClip.X = PageClip.Y = PageHeight = 0;
  PageClip.W = Size.W - BLOCK_SIZE;
  PageClip.H = Size.H - BLOCK_SIZE;
  PageSize.X = Size.X + BLOCK_SIZE / (size_t)2;
  PageSize.Y = Size.Y + BLOCK_SIZE / (size_t)2;
  PageSize.W = PageClip.W;
  PageSize.H = PageClip.H;
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
    return true;
  }
  return false;
}

/** @brief Deselect keywords
  * reset the currently selected keyword
  */
bool TextBox::Deselect()
{
  const size_t oldSelectedKeyword = SelectedKeyword;
  SelectedKeyword = Keywords.size();
  HighlightsDirty = true;
  Pane.DragTimeout = DRAG_TIMEOUT;

  return (oldSelectedKeyword != SelectedKeyword);
}

/** @brief Select keywords
  * \return true if we hit a keyword
  */
bool TextBox::Select(MouseState& Mouse,
                     real DeltaTime)
{
  if ((Mouse.X < Size.X) || (Mouse.X > Size.X + Size.W)
      || (Mouse.Y < Size.Y) || (Mouse.Y > Size.Y + Size.H)) {
    Pane.DragTimeout = DRAG_TIMEOUT;
    return false;
  }

  // translate coords to text surface coords
  const Uint16 X = Mouse.X - PageSize.X;
  const Uint16 Y = Mouse.Y - (PageSize.Y - PageClip.Y );
  // find selected keyword
  size_t oldSelectedKeyword = SelectedKeyword;
  size_t keywordsNum = Keywords.size();
  SelectedKeyword = keywordsNum;
  for (size_t i = 0, forSize = keywordsNum; i < forSize; ++i) {
    if ((Keywords[i].Size.X < X)
        && (Keywords[i].Size.Y < Y)
        && (X < Keywords[i].Size.X + Keywords[i].Size.W)
        && (Y < Keywords[i].Size.Y + Keywords[i].Size.H)) {
      SelectedKeyword = i;
      break;
    }
  }
  if (oldSelectedKeyword != SelectedKeyword) {
    // selected keyword has changed
    HighlightsDirty = true;
    oldSelectedKeyword = SelectedKeyword;
  }

  // nothing selected, try and drag the page instead
  if (SelectedKeyword < keywordsNum) {
    Pane.DragTimeout = DRAG_TIMEOUT;
  } else if (Pane.DragTimeout <= 0.f) {
    Pane.PaneScroll = Pane.PaneDragY - Mouse.Y;
    Pane.PaneDragY = Mouse.Y;
    Scroll();
  } else {
    Pane.PaneDragY = Mouse.Y;
    Pane.DragTimeout -= DeltaTime;
  }

  return true;
}

/** @brief Scroll the page by moving the text clipping rectangle
  */
void TextBox::Scroll()
{
  if (Pane.PaneScroll != 0) {
    // only scroll if the page is bigger than box
    if ((Pane.PaneScroll != 0) && (PageHeight > PageClip.H)) {
      int pagePosition = Pane.PaneScroll + PageClip.Y;
      if (pagePosition < 0) {
        PageClip.Y = 0;
      } else {
        if ((usint)pagePosition > PageHeight - PageClip.H) {
          pagePosition = PageHeight - PageClip.H;
        }
        PageClip.Y = pagePosition;
      }
    }
    // adjust the surfaces to the new scroll position
    ShowUp = (PageClip.Y > 1);
    ShowDown = ((usint)PageClip.Y + 1u + PageClip.H < PageHeight);
    PageSurface.SetClip(PageClip);
    Highlights.SetClip(PageClip);
    Pane.PaneScroll = 0;
  }
}

size_t_pair TextBox::GetMaxSize()
{
  size_t_pair maxSize;
  size_t lineSkip = Fonts[styleMain]->GetLineSkip();
  string text;
  size_t newline = 0;
  size_t pos = 0;
  maxSize.Y += lineSkip;
  LineHeight = Fonts[styleMain]->GetHeight() + lineSkip;

  // check sizes for all ines of text to make sure we can fit them unwrapped
  while (newline != string::npos) {
    newline = FindCharacter(Text, '\n', pos);
    text = CutString(Text, pos, newline);
    maxSize.X = max(maxSize.X, Fonts[styleMain]->GetWidth(text));
    maxSize.Y += LineHeight;
    pos = newline;
    ++pos;
  }

  maxSize.X += BLOCK_SIZE;
  maxSize.Y += BLOCK_SIZE;
  return maxSize;
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
  // record font changes and strip the formatting
  size_t pos = 0;
  size_t length = Text.size();
  vector<FontChange> fontChanges;
  string plain;
  while(pos < length) {
    const size_t_pair& titlePos = FindToken(Text, token::styleTitle, pos);
    const size_t_pair& quotePos = FindToken(Text, token::styleQuote, pos);
    const size_t_pair& monoPos = FindToken(Text, token::styleMono, pos);
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
      plain += CutString(Text, pos);
      pos = length;
    } else if (changePos.X != changePos.Y - 3) {
      plain += CutString(Text, pos, changePos.X);
      // insert a newline in front to make sure it breaks
      if (!plain.empty()) {
        if (plain[plain.size()-1] == ' ') {
          plain[plain.size()-1] = '\n';
        } else if (plain[plain.size()-1] != '\n') {
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
      plain += CutString(Text, changePos.X + 2, changePos.Y - 1);
      // and another newline at the end of formatting
      if (Text.size() > changePos.Y + 1) {
        if (Text[changePos.Y + 1] == ' ') {
          plain += '\n';
          ++pos; // skip the space
        } else if (Text[changePos.Y + 1] != '\n') {
          plain += '\n';
        }
      }
      fontChanges.push_back(FontChange(Fonts[styleMain], plain.size()));
    }
  }

  // record keywords and their positions and remove syntax symbols
  vector<size_t_pair> keywordPos;
  vector<string> keywordNames;
  string cleaned;
  string realName;
  pos = 0;
  length = plain.size();
  size_t skip = 0; // characters skipped
  while(pos < length) {
    // keywordPos store the positions in the cleaned string and need fixing
    size_t_pair namePos = FindToken(plain, token::keyword, pos);
    // quit if no more keywords
    if (namePos.X == string::npos) {
      break;
    }
    const size_t_pair realNamePos = FindToken(plain, token::expression,
                                    pos, namePos.Y);
    // copy up to the keyword
    cleaned += CutString(plain, pos, namePos.X);
    pos = namePos.Y + 1;
    // don't print the real name, but record it
    if (realNamePos.X != string::npos) {
      realName = CutString(plain, realNamePos.X+1, realNamePos.Y);
      namePos.Y = realNamePos.X;
    } else {
      realName = CutString(plain, namePos.X+1, namePos.Y);
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
    cleaned += CutString(plain, namePos.X+1, namePos.Y);
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
    cleaned += CutString(plain, pos);
  }

  length = cleaned.size();
  skip = pos = 0;
  // remove escape characters
  while(pos < length) {
    const char c = cleaned[pos];
    if (c == '\\') {
      ++skip;
      if (++pos < length) {
        cleaned[pos - skip] = cleaned[pos];
      }
      // fix the  keyword positions that are beyond this pos
      for (size_t i = 0, fSz = keywordPos.size(); i < fSz; ++i) {
        if (keywordPos[i].X > pos) {
          --keywordPos[i].X;
        }
        if (keywordPos[i].Y > pos) {
          --keywordPos[i].Y;
        }
      }
    } else {
      cleaned[pos - skip] = c;
    }
    ++pos;
  }

  if (skip > 0) {
    cleaned = CutString(cleaned, 0, length - skip);
  }

  Lines.clear();
  Keywords.clear();
  size_t lineSkip = Fonts[styleMain]->GetLineSkip();
  size_t fontHeight = Fonts[styleMain]->GetHeight();
  LineHeight = fontHeight + lineSkip;
  size_t lastLineEnd = 0;
  size_t lastPos = 0;
  bool firstWord = true;

  length = cleaned.size();
  pos = 0;

  // break the text into lines that fit within the textbox width
  size_t fontChangeI = 0;
  Font* currentFont = Fonts[styleMain];
  PageHeight = 0;
  while(pos < length) {
    bool flush = false;
    //find space
    size_t space = cleaned.find(' ', pos);
    // if end of text
    if (space == string::npos) {
      space = length;
      flush = true;
    }
    // find newline
    size_t newline = cleaned.find('\n', pos);
    if (newline == string::npos) {
      newline = length;
    }
    // what's first, newline or space?
    if (newline < space) {
      pos = newline;
      //TODO we can't replace with space because of possible rollback
      //CleanText[pos] = ' '; // replace with a space
      flush = true;
    } else {
      pos = space;
    }

    // test print the line
    string line = CutString(cleaned, lastLineEnd, pos);
    // did we fit in?
    if (Fonts[styleMain]->GetWidth(line) < PageClip.W || firstWord) {
      lastPos = pos;
    } else {
      // overflow, revert to last position and print that
      flush = true;
      pos = lastPos; // include the character so the sizes match
      line = CutString(cleaned, lastLineEnd, pos);
    }

    ++pos;
    firstWord = false;
    if (flush || !(pos < length)) { // || in case there's a space at the end
      if (fontChangeI < fontChanges.size()
          && fontChanges[fontChangeI].Position <= lastLineEnd) {
        // change font and ready for next change
        currentFont = fontChanges[fontChangeI].LineFont;
        ++fontChangeI;
      }
      // size up the line
      Rect lineSize;
      lineSize.W = currentFont->GetWidth(line);
      lineSize.H = currentFont->GetHeight();
      lineSize.X = Centered ? (PageSize.W - lineSize.W) / 2 : 0;
      lineSize.Y = PageHeight;
      // ready for the next line
      PageHeight += lineSize.H + currentFont->GetLineSkip();
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

  if (PageHeight) {
    PageHeight -= lineSkip;
  }
  SelectedKeyword = Keywords.size();
  // scroll to end
  PageClip.Y = PageHeight > PageClip.H? (PageHeight - PageClip.H) : 0;

  return (PageHeight > 0);
}

/** @brief returns the surface with word wrapped text of the current page
  * updates the page if needed, otherwise returns the cached page
  */
void TextBox::RefreshPage()
{
  if (PageDirty) {
    PageDirty = false;
    if (!PageHeight) {
      BreakText();
    }
    if (PageHeight == 0) {
      // don't bother if there is no text
      return;
    }
    PageSurface.Init(PageClip.W, max(PageHeight, PageClip.H));
    PageSurface.SetClip(PageClip);
    for (size_t i = 0, fSz = Lines.size(); i < fSz; ++i) {
      PageSurface.PrintText(Lines[i].Size, *Lines[i].LineFont,
                            Lines[i].Text, 255, 255, 255);
    }
  }
}

/** @brief returns the surface with the backdrop and highligths for the keywords
  * only updates if required, otherwise returns cached
  */
void TextBox::RefreshHighlights()
{
  if (HighlightsDirty) {
    HighlightsDirty = false;
    if (!PageHeight) {
      BreakText();
    }
    Highlights.Init(PageSize.W, max(PageHeight, PageSize.H));
    Highlights.SetClip(PageClip);
    // paint a rectangle behind each keyword
    for (size_t i = 0, forSize = Keywords.size(); i < forSize; ++i) {
      // change colour for the selected keyword
      uint highlightColour = (i == SelectedKeyword) ? 150 : 50;
      Highlights.DrawRectangle(Keywords[i].Size, highlightColour, 150, 150);
    }
  }
}
