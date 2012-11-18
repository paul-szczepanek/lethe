#include <SDL.h>
#include <SDL_ttf.h>

#include "textbox.h"
#include "reader.h"
#include "tokens.h"

const Uint16 RIGHT_MARGIN = 20;
const real DRAG_TIMEOUT = 0.25;

TextBox::TextBox()
  : Active(false), Visible(false), PageDirty(false), PageHeight(0)
{
  PageSurface = NULL;
  PageTextSurface = NULL;
  FontMain = NULL;
}

TextBox::~TextBox()
{
  if (PageSurface) {
    SDL_FreeSurface(PageSurface);
  }

  if (PageTextSurface) {
    SDL_FreeSurface(PageTextSurface);
  }
}


/** @brief Scroll
  *
  * @todo: document this function
  */
void TextBox::Scroll()
{
  if (Pane.PaneScroll != 0) {
    // only scroll if the page is bigger than box
    if ((Pane.PaneScroll != 0) && (PageHeight > ClipRect.h)) {
      int pagePosition = Pane.PaneScroll + ClipRect.y;

      if (pagePosition < 0) {
        ClipRect.y = 0;

      } else {
        if ((usint)pagePosition > PageHeight - ClipRect.h) {
          pagePosition = PageHeight - ClipRect.h;
        }

        ClipRect.y = pagePosition;
      }
    }

    Pane.PaneScroll = 0;
  }
}

/** @brief GetKeyword
  *
  * @todo: document this function
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
  PageDirty = true;
  Pane.PaneDrag = false;

  return (oldSelectedKeyword != SelectedKeyword);
}

/** @brief Select keywords
  *
  * returns true if we hit a keyword
  */
bool TextBox::Select(MouseState& Mouse,
                     real DeltaTime)
{
  if (Pane.DragTimeout > 0.f) {
    Pane.DragTimeout -= DeltaTime;
  }

  if (Mouse.X > Size.X && Mouse.X < Size.X + Size.W
      && Mouse.Y > Size.Y && Mouse.Y < Size.Y + Size.Y) {
    Pane.DragTimeout = DRAG_TIMEOUT;
    return false;
  }

  size_t oldSelectedKeyword = SelectedKeyword;
  size_t keywordsNum = Keywords.size();

  SelectedKeyword = keywordsNum;

  const Uint16 X = Mouse.X - DstRect.x;
  const Uint16 Y = Mouse.Y - (DstRect.y - ClipRect.y);

  for (size_t i = 0, forSize = keywordsNum; i < forSize; ++i) {
    if ((Keywords[i].X < X) &&
        (Keywords[i].Y < Y) &&
        (X < Keywords[i].X + Keywords[i].W) &&
        (Y < Keywords[i].Y + Keywords[i].H)) {
      SelectedKeyword = i;
    }
  }

  if (oldSelectedKeyword != SelectedKeyword) {
    PageDirty = true;
    oldSelectedKeyword = SelectedKeyword;
  }

  if (SelectedKeyword < keywordsNum) {
    Pane.DragTimeout = DRAG_TIMEOUT;
  } else if (Pane.PaneDrag && (Pane.DragTimeout<=0.f)) {
    Pane.PaneScroll = Pane.PaneDragY - Mouse.Y;
    Pane.PaneDragY = Mouse.Y;
  } else {
    Pane.PaneDrag = true;
    Pane.PaneDragY = Mouse.Y;
  }

  Scroll();

  return true;
}

/** @brief Draw page onto passed in surface
  *
  * draws the backdrop and text onto the screen passed into the function
  */
void TextBox::Draw(SDL_Surface* Screen)
{
  SDL_Surface* page = GetPageSurface();
  SDL_Surface* text = GetPageTextSurface();

  if (page && text && Screen) {
    SDL_BlitSurface(page, &ClipRect, Screen, &DstRect);
    SDL_BlitSurface(text, &ClipRect, Screen, &DstRect);
  }
}

/** @brief word wrap and find keywords
  *
  * fills in Lines and Keywords in a single pass
  */
bool TextBox::BreakText()
{
  vector<size_t_pair> keywordPos;
  vector<string> keywordNames;

  size_t pos = 0;
  size_t length = Text.size();
  size_t skip = 0; //characters skipped

  string cleaned;

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
      keywordNames.push_back(cutString(Text, realNamePos.X+1, realNamePos.Y));
      namePos.Y = realNamePos.X;
    } else {
      keywordNames.push_back(cutString(Text, namePos.X+1, namePos.Y));
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

    keywordPos.push_back(namePos);
  }

  // add remaining characters after the last keyword
  if (pos < length) {
    cleaned += cutString(Text, pos);
  }

  Text.swap(cleaned);
  cleaned = Text;

  length = Text.size();
  pos = 0;
  skip = 0;

  // remove escape characters
  while(pos < length) {
    const char c = Text[pos];
    if (c == '\\') {
      ++skip;
      if (++pos < length) {
        cleaned[pos - skip] = Text[pos];
      }
      // ifx the  keyword positions that are beyond this pos
      for (size_t i = 0, for_size = keywordPos.size(); i < for_size; ++i) {
        if (keywordPos[i].X > pos) {
          --keywordPos[i].X;
        }
        if (keywordPos[i].Y > pos) {
          --keywordPos[i].Y;
        }
      }
    } else {
      cleaned[pos] = c;
    }
    ++pos;
  }

  if (skip > 0) {
    Text = cutString(cleaned, 0, length - skip);
  }

  Lines.clear();
  Keywords.clear();
  uint lineSkip = TTF_FontLineSkip(FontMain);
  LineHeight = TTF_FontHeight(FontMain) + lineSkip;
  size_t lastLineEnd = 0;
  size_t lastPos = 0;
  int width;
  int height;
  bool firstWord = true;

  length = Text.size();
  pos = 0;

  // break the text into lines that fit within the textbox width
  while(pos < length) {
    bool flush = false;

    //find space
    size_t space = Text.find(' ', pos);
    // if end of text
    if (space == string::npos) {
      space = length;
      flush = true;
    }

    // find newline
    size_t newline = Text.find('\n', pos);
    if (newline == string::npos) {
      newline = length;
    }

    // what's first, newline or space?
    if (newline < space) {
      pos = newline;
      Text[pos] = ' '; // replace with a space
      flush = true;
    } else {
      pos = space;
    }

    ++pos;

    // test print the line
    string line = cutString(Text, lastLineEnd, pos);
    TTF_SizeText(FontMain, line.c_str(), &width ,&height);

    // did we fit in?
    if (width < ClipRect.w - RIGHT_MARGIN || firstWord) {
      lastPos = pos;
    } else {
      // overflow, revert to last position and print that
      flush = true;
      pos = lastPos; // include the character so the sizes match
      line = cutString(Text, lastLineEnd, pos);
    }

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
    const size_t lineLength = Lines[i].size();
    const size_t lineEnd = lastLineEnd + lineLength;

    for (size_t j = 0, for_size = keywordPos.size(); j < for_size; ++j) {
      const size_t_pair& keyPos = keywordPos[j];
      // check if this keyword is on this line
      if (keyPos.X > lineEnd) {
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
          const string& keywordStart = cutString(Text, 0, beg);
          TTF_SizeText(FontMain, keywordStart.c_str(), &width ,&height);
          newKey.X = width;
        }
        newKey.Y = PageHeight;

        // find where the keyword ends
        const string& keywordEnd = cutString(Text, beg, end);
        TTF_SizeText(FontMain, keywordEnd.c_str(), &width ,&height);
        newKey.W = width;
        newKey.H = height;

        Keywords.push_back(newKey);
      }
    }

    PageHeight += LineHeight;
    lastLineEnd = lineEnd;
  }

  return (PageHeight > 0);
}

/** @brief returns the surface with word wrapped text of the current page
  *
  * updates the page if needed, otherwise returns the cached page
  */
SDL_Surface* TextBox::GetPageTextSurface()
{
  if (PageTextSurface) {
    return PageTextSurface;
  }

  if (!PageHeight) {
    BreakText();
  }

  assert(PageHeight > 0);

  PageTextSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, ClipRect.w,
                                         max(PageHeight, uint(ClipRect.h)),
                                         32, 0x000000FF, 0x0000FF00,
                                         0x00FF0000, 0xFF000000);

  SDL_Color white;
  white.r = white.g = white.b = 255;
  SDL_Rect dstRect;
  dstRect.x = 0;

  for (size_t i = 0, for_size = Lines.size(); i < for_size; ++i) {
    SDL_Surface* lineSurface;
    lineSurface = TTF_RenderText_Solid(FontMain, Lines[i].c_str(), white);

    dstRect.y = LineHeight * i;

    SDL_BlitSurface(lineSurface, 0, PageTextSurface, &dstRect);
    SDL_FreeSurface(lineSurface);
  }

  return PageTextSurface;
}

/** @brief returns the surface with the backdrop and highligths for the keywords
  *
  * only updates if required, otherwise returns cached
  */
SDL_Surface* TextBox::GetPageSurface()
{
  if (PageSurface && !PageDirty) {
    return PageSurface;
  }

  PageDirty = false;

  if (!PageSurface) {
    if (!PageHeight) {
      BreakText();
    }

    PageSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, ClipRect.w,
                                       max(PageHeight, uint(ClipRect.h)),
                                       32, 0x000000FF, 0x0000FF00,
                                       0x00FF0000, 0xFF000000);
  }

  for (size_t i = 0, forSize = Keywords.size(); i < forSize; ++i) {
    SDL_Rect highlight;
    highlight.x = Keywords[i].X;
    highlight.y = Keywords[i].Y;
    highlight.w = Keywords[i].W;
    highlight.h = Keywords[i].H;

    Uint8 highlightColour = (i == SelectedKeyword) ? 150 : 50;

    SDL_FillRect(PageSurface, &highlight, SDL_MapRGB(PageSurface->format,
                 highlightColour, 150, 150));
  }

  return PageSurface;
}

/** @brief Set Size of the textbox
  *
  * resets the page as well
  */
void TextBox::SetSize(Rect& NewSize)
{
  Size = NewSize;

  ClipRect.w = Size.W;
  ClipRect.h = Size.H;
  DstRect.x = Size.X;
  DstRect.y = Size.Y;

  ResetPage();
}

/** @brief Reset Page
  *
  * clears cached surfaces
  */
void TextBox::ResetPage()
{
  ClipRect.x = ClipRect.y = PageHeight = 0;

  if (PageSurface) {
    SDL_FreeSurface(PageSurface);
    PageSurface = NULL;
  }

  if (PageTextSurface) {
    SDL_FreeSurface(PageTextSurface);
    PageTextSurface = NULL;
  }
}

/** @brief Set page text
  *
  * resets the cached pages
  */
void TextBox::SetText(string NewText, TTF_Font* Font)
{
  if (Font) {
    FontMain = Font;
  }

  if (FontMain) {
    Text = NewText;
    ResetPage();
  } else {
    LOG("Font not set in textbox!");
  }
}
