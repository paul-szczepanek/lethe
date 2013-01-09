#include "textbox.h"
#include "reader.h"
#include "tokens.h"

const Uint16 RIGHT_MARGIN = BLOCK_SIZE;
const real DRAG_TIMEOUT = 0.1;

/** @brief Set page text
  *
  * resets the cached pages
  */
void TextBox::SetText(string NewText)
{
  Text = NewText;
  Reset();
}

/** @brief Reset Page
  *
  * clears cached surfaces
  */
void TextBox::Reset()
{
  PageClip.X = PageClip.Y = PageHeight = 0;
  HighlightsDirty = PageDirty = true;
  Keywords.clear();
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
  *
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
  *
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

  // keyword selection
  size_t oldSelectedKeyword = SelectedKeyword;
  size_t keywordsNum = Keywords.size();

  SelectedKeyword = keywordsNum;

  // translate coords to text surface coords
  const Uint16 X = Mouse.X - PageSize.X;
  const Uint16 Y = Mouse.Y - (PageSize.Y - PageClip.Y );

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
    HighlightsDirty = true;
    oldSelectedKeyword = SelectedKeyword;
  }

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
  size_t lineSkip = FontMain->GetLineSkip();
  string text;
  size_t newline = 0;
  size_t pos = 0;

  LineHeight = FontMain->GetHeight() + lineSkip;

  while (newline != string::npos) {
    newline = FindCharacter(Text, '\n', pos);
    text = cutString(Text, pos, newline);
    maxSize.X = max(maxSize.X, FontMain->GetWidth(text));
    maxSize.Y += LineHeight;
    pos = newline;
    ++pos;
  }

  maxSize.X += BLOCK_SIZE * 1.5;
  maxSize.Y += BLOCK_SIZE + lineSkip;

  return maxSize;
}

/** @brief word wrap and find keywords
  *
  * fills in Lines and Keywords
  */
bool TextBox::BreakText()
{
  vector<size_t_pair> keywordPos;
  vector<string> keywordNames;

  PageClip.W = Size.W - BLOCK_SIZE;
  PageClip.H = Size.H - BLOCK_SIZE;
  PageSize.X = Size.X + BLOCK_SIZE / (size_t)2;
  PageSize.Y = Size.Y + BLOCK_SIZE / (size_t)2;
  PageSize.W = PageClip.W;
  PageSize.H = PageClip.H;

  size_t pos = 0;
  size_t length = Text.size();
  size_t skip = 0; // characters skipped

  string cleaned;
  string realName;

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
    cleaned += cutString(Text, pos, namePos.X);
    pos = namePos.Y + 1;

    // don't print the real name, but record it
    if (realNamePos.X != string::npos) {
      realName = cutString(Text, realNamePos.X+1, realNamePos.Y);
      namePos.Y = realNamePos.X;
    } else {
      realName = cutString(Text, namePos.X+1, namePos.Y);
    }

    // check keywords found in a position earlier than set threshold to see
    // if they're still valid
    const bool validKeyword = (pos < ValidateKeywords) ?
                              ValidKeywords.ContainsValue(realName)
                              : true;

    if (validKeyword) {
      keywordNames.push_back(realName);
    }

    // copy the keyword
    cleaned += cutString(Text, namePos.X+1, namePos.Y);
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
    cleaned += cutString(Text, pos);
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
      for (size_t i = 0, for_size = keywordPos.size(); i < for_size; ++i) {
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
    cleaned = cutString(cleaned, 0, length - skip);
  }

  Lines.clear();
  Keywords.clear();
  size_t lineSkip = FontMain->GetLineSkip();
  size_t fontHeight = FontMain->GetHeight();
  LineHeight = fontHeight + lineSkip;
  size_t lastLineEnd = 0;
  size_t lastPos = 0;
  bool firstWord = true;

  length = cleaned.size();
  pos = 0;

  // break the text into lines that fit within the textbox width
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
    string line = cutString(cleaned, lastLineEnd, pos);

    // did we fit in?
    if (FontMain->GetWidth(line) < PageClip.W || firstWord) {
      lastPos = pos;
    } else {
      // overflow, revert to last position and print that
      flush = true;
      pos = lastPos; // include the character so the sizes match
      line = cutString(cleaned, lastLineEnd, pos);
    }

    ++pos;

    firstWord = false;

    if (flush || !(pos < length)) { // || in case there's a space at the end
      lastLineEnd = pos;
      Lines.push_back(line);
      firstWord = true;
    }
  }

  PageHeight = 0;
  lastLineEnd = 0;

  // record visual keyword positions
  for (size_t i = 0, for_size = Lines.size(); i < for_size; ++i) {
    const string& line = Lines[i];
    const size_t lineLength = line.size() + 1;
    const size_t lineEnd = lastLineEnd + lineLength;

    for (size_t j = 0, for_size = keywordPos.size(); j < for_size; ++j) {
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
        if (beg) {
          const string& keywordStart = cutString(line, 0, beg);
          newKey.Size.X = FontMain->GetWidth(keywordStart);
        }
        newKey.Size.Y = PageHeight;

        // find where the keyword ends
        const string& keywordEnd = cutString(line, beg, end);
        newKey.Size.W = FontMain->GetWidth(keywordEnd);
        newKey.Size.H = fontHeight;

        Keywords.push_back(newKey);
      }
    }

    PageHeight += LineHeight;
    lastLineEnd = lineEnd;
  }

  SelectedKeyword = Keywords.size();

  // scroll to end
  PageClip.Y = PageHeight > PageClip.H? (PageHeight - PageClip.H) : 0;

  return (PageHeight > 0);
}

/** @brief returns the surface with word wrapped text of the current page
  *
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
      return;
    }

    PageSurface.Init(PageClip.W, max(PageHeight, PageClip.H));

    Rect dst;
    dst.X = 0;

    for (size_t i = 0, for_size = Lines.size(); i < for_size; ++i) {
      dst.Y = LineHeight * i;
      PageSurface.PrintText(dst, *FontMain, Lines[i], 255, 255, 255);
    }

    PageSurface.SetClip(PageClip);
  }
}

/** @brief returns the surface with the backdrop and highligths for the keywords
  *
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

    for (size_t i = 0, forSize = Keywords.size(); i < forSize; ++i) {
      uint highlightColour = (i == SelectedKeyword) ? 150 : 50;
      Highlights.DrawRectangle(Keywords[i].Size, highlightColour, 150, 150);
    }

    Highlights.SetClip(PageClip);
  }
}
