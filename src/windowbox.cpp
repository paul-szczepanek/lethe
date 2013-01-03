#include "windowbox.h"
#include <SDL_image.h>
#include <SDL_rotozoom.h>

/** @brief Create all the assets required by the text box
  *
  * builds the image slice array for the frame building
  */
void WindowBox::Init(TTF_Font* Font,
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
}

/** @brief Draw page onto passed in surface
  */
void WindowBox::Draw(SDL_Surface* Screen)
{
  if (Visible && Screen) {
    DrawFrame(Screen);
  }
}

/** @brief Set Size of the textbox
  *
  * resets the page as well
  */
void WindowBox::SetSize(Rect NewSize)
{
  if (AspectW) {
    if (AspectH) {
      size_t maxH = NewSize.W * ((real)AspectH / (real)AspectW);
      if (maxH < AspectH * BLOCK_SIZE) {
        maxH = AspectH * BLOCK_SIZE;
      }
      if (NewSize.H > maxH) {
        NewSize.H = maxH;
      }
    } else {
      if (NewSize.W < AspectW * BLOCK_SIZE) {
        NewSize.W = AspectW * BLOCK_SIZE;
      }
    }
  }

  NewSize.Blockify();

  if (Size != NewSize) {
    Size = NewSize;
    Reset();
  }
}

/** @brief free the SDL surfaces
  */
void WindowBox::Reset()
{
  ResetFrame();
}

/** @brief free the SDL surfaces
  */
void WindowBox::ResetFrame()
{
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

bool WindowBox::DrawFrame(SDL_Surface* Screen)
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
          } else if (i == colS / 2 && colS > 8) {
            index = 1;
          } else if (i < colS - 1) {
            index = 2;
          } else {
            index = 3;
          }
        } else if (j < rowS - 1) {
          if (i == 0) {
            if (j == rowS / 2 && rowS > 8) {
              index = 4;
            } else {
              index = 8;
            }
          } else if (i < colS - 1) {
            index = 5;
          } else {
            if (j == rowS / 2 && rowS > 8) {
              index = 7;
            } else {
              index = 11;
            }
          }
        } else {
          if (i == 0) {
            index = 12;
          } else if (i == colS / 2 && colS > 8) {
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

    FrameDst.w = Size.W;
    FrameDst.h = Size.H;
    FrameDst.x = Size.X;
    FrameDst.y = Size.Y;

    UpDst = { 0, 0, 0, 0 };
    UpDst.x = FrameDst.x + FrameDst.w - 2 * BLOCK_SIZE;
    UpDst.y = FrameDst.y;
    DownDst = { 0, 0, 0, 0 };
    DownDst.x = UpDst.x;
    DownDst.y = FrameDst.y + FrameDst.h - BLOCK_SIZE;
    IconDst = { 0, 0, 0, 0 };
    IconDst.x = FrameDst.x + FrameDst.w - BLOCK_SIZE;
    IconDst.y = FrameDst.y + BLOCK_SIZE;

    SDL_FreeSurface(spritePage);
  }

  if (FrameSurface) {
    SDL_BlitSurface(FrameSurface, 0, Screen, &FrameDst);
    if (ShowUp) {
      SDL_BlitSurface(FrameUp, 0, Screen, &UpDst);
    }
    if (ShowDown) {
      SDL_BlitSurface(FrameDown, 0, Screen, &DownDst);
    }
    if (ShowIcon) {
      SDL_BlitSurface(FrameIcon, 0, Screen, &IconDst);
    }
    return true;
  }

  return false;
}
