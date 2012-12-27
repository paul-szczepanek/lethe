#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "textbox.h"
#include "reader.h"
#include "tokens.h"

const Uint16 RIGHT_MARGIN = BLOCK_SIZE;
const real DRAG_TIMEOUT = 0.1;

TextBox::~TextBox()
{
  ResetPage();
}

/** @brief Create all the assets required by the text box
  *
  * builds the image slice array for the frame building
  */
void TextBox::Init(TTF_Font* Font,
                   const string& Frame,
                   int Bpp)
{
  if (Font) {
    FontMain = Font;
  }

  if (!Frame.empty()) {
    FrameName = Frame;
  }

  BPP = Bpp;

  ResetPage();
}


/** @brief Set Size of the textbox
  *
  * resets the page as well
  */
void TextBox::SetSize(Rect& NewSize)
{
  NewSize.Blockify();
  if (Size != NewSize) {
    Size = NewSize;
    FrameDst.w = Size.W;
    FrameDst.h = Size.H;
    FrameDst.x = Size.X;
    FrameDst.y = Size.Y;
    TextClip.w = Size.W - BLOCK_SIZE;
    TextClip.h = Size.H - BLOCK_SIZE;
    TextDst.x = Size.X + BLOCK_SIZE / (uint)2;
    TextDst.y = Size.Y + BLOCK_SIZE / (uint)2;
    TextDst.w = TextClip.w;
    TextDst.h = TextClip.h;

    ResetPage();
  }
}

/** @brief Set page text
  *
  * resets the cached pages
  */
void TextBox::SetText(string NewText)
{
  if (FontMain) {
    Text = NewText;
    ResetPage();
  } else {
    LOG("Font not set in textbox!");
  }
}

/** @brief Reset Page
  *
  * clears cached surfaces
  */
void TextBox::ResetPage()
{
  TextClip.x = TextClip.y = PageHeight = 0;

  if (PageSurface) {
    SDL_FreeSurface(PageSurface);
    PageSurface = NULL;
  }

  if (PageTextSurface) {
    SDL_FreeSurface(PageTextSurface);
    PageTextSurface = NULL;
  }

  if (FrameSurface) {
    SDL_FreeSurface(FrameSurface);
    FrameSurface = NULL;
  }

  if (FrameDown) {
    SDL_FreeSurface(FrameDown);
    FrameDown = NULL;
  }

  if (FrameUp) {
    SDL_FreeSurface(FrameUp);
    FrameUp = NULL;
  }

  if (FrameIcon) {
    SDL_FreeSurface(FrameIcon);
    FrameIcon = NULL;
  }
}

/** @brief Draw page onto passed in surface
  */
void TextBox::Draw(SDL_Surface* Screen)
{
  if (Screen) {
    SDL_Surface* page = GetPageSurface();
    SDL_Surface* text = GetPageTextSurface();
    DrawFrame(Screen);
    if (page && text) {
      SDL_BlitSurface(page, &TextClip, Screen, &TextDst);
      SDL_BlitSurface(text, &TextClip, Screen, &TextDst);
    }
  }
}

bool TextBox::DrawFrame(SDL_Surface* Screen)
{
  // only bother if we have a frame template
  if (FrameName.empty()) {
    return false;
  }

  if (!FrameSurface) {
    FrameSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, Size.W, Size.H, BPP,
                                        MASK_R, MASK_G, MASK_B, MASK_A);

    SDL_Surface* spritePage = IMG_Load((FRAMES_DIR+FrameName+".png").c_str());

    if (!spritePage) {
      LOG(FrameName+" - frame image missing");
      FrameName.clear();
      return false;
    } else if (spritePage->w != BLOCK_SIZE * 4) {
      //TODO: stretch SpritePage if needed
      LOG("frame image size incorrect");
      return false;
    }

    // sprite image layout
    // 00C 01M 02E 03C   corner middle edge corner
    // 04M 05M 06I 07M   middle middle icon middle
    // 08E 09D 10U 11E   edge    down   up   edge
    // 12C 13M 14E 15C   corner middle edge corner

    SDL_Rect src = { 0, 0, BLOCK_SIZE, BLOCK_SIZE };
    SDL_Rect dst = { 0, 0, BLOCK_SIZE, BLOCK_SIZE };

    for (size_t i = 0, colS = Size.W / BLOCK_SIZE; i < colS; ++i) {
      for (size_t j = 0, rowS = Size.H / BLOCK_SIZE; j < rowS; ++j) {
        size_t index;
        if (j == 0) {
          if (i == 0) {
            index = 0;
          } else if (i == colS / 2) {
            index = 1;
          } else if (i < colS - 1) {
            index = 2;
          } else {
            index = 3;
          }
        } else if (j < rowS - 1) {
          if (i == 0) {
            if (j == rowS / 2) {
              index = 4;
            } else {
              index = 8;
            }
          } else if (i < colS - 1) {
            index = 5;
          } else {
            if (j == rowS / 2) {
              index = 7;
            } else {
              index = 11;
            }
          }
        } else {
          if (i == 0) {
            index = 12;
          } else if (i == colS / 2) {
            index = 13;
          } else if (i < colS - 1) {
            index = 14;
          } else {
            index = 15;
          }
        }

        src.x = BLOCK_SIZE * (index % 4);
        src.y = BLOCK_SIZE * (index / 4);
        dst.x = BLOCK_SIZE * i;
        dst.y = BLOCK_SIZE * j;
        SDL_BlitSurface(spritePage, &src, FrameSurface, &dst);
      }
    }

    FrameUp = SDL_CreateRGBSurface(SDL_HWSURFACE, BLOCK_SIZE, BLOCK_SIZE, BPP,
                                   MASK_R, MASK_G, MASK_B, MASK_A);
    FrameIcon = SDL_CreateRGBSurface(SDL_HWSURFACE, BLOCK_SIZE, BLOCK_SIZE, BPP,
                                     MASK_R, MASK_G, MASK_B, MASK_A);
    FrameDown = SDL_CreateRGBSurface(SDL_HWSURFACE, BLOCK_SIZE, BLOCK_SIZE, BPP,
                                     MASK_R, MASK_G, MASK_B, MASK_A);

    src.x = BLOCK_SIZE * 2;
    src.y = BLOCK_SIZE * 2;
    SDL_BlitSurface(spritePage, &src, FrameUp, 0);
    src.x = BLOCK_SIZE * 2;
    src.y = BLOCK_SIZE * 1;
    SDL_BlitSurface(spritePage, &src, FrameIcon, 0);
    src.x = BLOCK_SIZE * 1;
    src.y = BLOCK_SIZE * 2;
    SDL_BlitSurface(spritePage, &src, FrameDown, 0);

    UpDst = { 0, 0, 0, 0 };
    UpDst.x = FrameDst.x + FrameDst.w - 2 * BLOCK_SIZE;
    UpDst.y = FrameDst.y;
    DownDst = { 0, 0, 0, 0 };
    DownDst.x = UpDst.x;
    DownDst.y = FrameDst.y + FrameDst.h - BLOCK_SIZE;

    SDL_FreeSurface(spritePage);
  }

  if (FrameSurface) {
    SDL_BlitSurface(FrameSurface, 0, Screen, &FrameDst);
    if (TextClip.y > 1) {
      SDL_BlitSurface(FrameUp, 0, Screen, &UpDst);
    }
    if ((usint)TextClip.y + 1u + TextClip.h < PageHeight ) {
      SDL_BlitSurface(FrameDown, 0, Screen, &DownDst);
    }
    return true;
  }

  return false;
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
  PageDirty = true;
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
  const Uint16 X = Mouse.X - TextDst.x;
  const Uint16 Y = Mouse.Y - (TextDst.y - TextClip.y );

  for (size_t i = 0, forSize = keywordsNum; i < forSize; ++i) {
    if ((Keywords[i].X < X)
        && (Keywords[i].Y < Y)
        && (X < Keywords[i].X + Keywords[i].W)
        && (Y < Keywords[i].Y + Keywords[i].H)) {
      SelectedKeyword = i;
      break;
    }
  }

  if (oldSelectedKeyword != SelectedKeyword) {
    PageDirty = true;
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
    if ((Pane.PaneScroll != 0) && (PageHeight > TextClip.h)) {
      int pagePosition = Pane.PaneScroll + TextClip.y;

      if (pagePosition < 0) {
        TextClip.y = 0;
      } else {
        if ((usint)pagePosition > PageHeight - TextClip.h) {
          pagePosition = PageHeight - TextClip.h;
        }

        TextClip.y = pagePosition;
      }
    }

    Pane.PaneScroll = 0;
  }
}

/** @brief word wrap and find keywords
  *
  * fills in Lines and Keywords
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
      //TODO we can't replace with space because of possible rollback
      //Text[pos] = ' '; // replace with a space
      flush = true;
    } else {
      pos = space;
    }

    ++pos;

    // test print the line
    string line = cutString(Text, lastLineEnd, pos);
    TTF_SizeText(FontMain, line.c_str(), &width ,&height);

    // did we fit in?
    if (width < TextClip.w || firstWord) {
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

  SelectedKeyword = Keywords.size();

  // scroll to end
  TextClip.y = PageHeight > TextClip.h? (PageHeight - TextClip.h) : 0;

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

  if (PageHeight == 0) {
    return NULL;
  }

  PageTextSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, TextClip.w,
                                         max(PageHeight, uint(TextClip.h)),
                                         BPP, MASK_R, MASK_G, MASK_B, MASK_A);

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

    PageSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, TextDst.w,
                                       max(PageHeight, uint(TextDst.h)),
                                       BPP, MASK_R, MASK_G, MASK_B, MASK_A);
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
