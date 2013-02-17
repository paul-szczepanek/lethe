#include "windowbox.h"

#ifdef __ANDROID__
lint WindowBox::Grid = 16;
#else
lint WindowBox::Grid = 32;
#endif

void WindowBox::Init(const string& Frame,
                     const int Bpp)
{
  if (!Frame.empty()) {
    FrameVisible = true;
    FrameName = Frame;
  }
  BPP = Bpp;
}

/** @brief Draw page onto passed in surface
  */
void WindowBox::Draw()
{
  if (Visible) {
    DrawFrame();
  }
}


/** @brief Enforce aspect ratio or minimum size
  */
void WindowBox::FixAspectRatio(Rect& NewSize)
{
  if (AspectW) {
    if (AspectH) {
      lint maxH = NewSize.W * ((real)AspectH / (real)AspectW);
      if (maxH < AspectH * GRID) {
        maxH = AspectH * GRID;
      }
      if (NewSize.H > maxH) {
        NewSize.H = maxH;
      }
    } else {
      if (NewSize.W < AspectW * GRID) {
        NewSize.W = AspectW * GRID;
      }
    }
  } else if (AspectH) {
    if (NewSize.H < AspectH * GRID) {
      NewSize.H = AspectH * GRID;
    }
  }
}

/** @brief Set Size (resets the page as well)
  */
void WindowBox::SetSize(Rect NewSize)
{
  FixAspectRatio(NewSize);

  // align size to block size
  NewSize.W = max(NewSize.W, 2*GRID);
  NewSize.H = max(NewSize.H, 2*GRID);
  const lint newW = NewSize.W - (NewSize.W % GRID);
  const lint newH = NewSize.H - (NewSize.H % GRID);
  NewSize.X += (NewSize.W - newW) / 2;
  NewSize.Y += (NewSize.H - newH) / 2;
  NewSize.W = newW;
  NewSize.H = newH;

  if (Size != NewSize) {
    Size = NewSize;
    BuildFrame();
    Reset();
  }
}

/** @brief Open the frame file and construct the image of the frame
  */
bool WindowBox::BuildFrame()
{
  // only bother if we have a frame template
  if (!FrameVisible) {
    return false;
  }
  FrameSurface.Init(Size.W, Size.H);

  Surface spritePage;
  if (!spritePage.LoadImage(FRAMES_DIR+SLASH+FrameName+".png")) {
    LOG(FrameName+" - frame image missing");
    FrameVisible = false;
    return false;
  } else if (spritePage.W != GRID * 4) {
    const real scale = (real)GRID * (real)4 / (real)spritePage.W;
    spritePage.Zoom(scale, scale);
    LOG("frame image size incorrect, scaling to fit");
  }

  // sprite image layout
  // 00C 01M 02E 03C   corner middle edge corner
  // 04M 05M 06I 07M   middle middle icon middle
  // 08E 09D 10U 11E   edge    down   up   edge
  // 12C 13M 14E 15C   corner middle edge corner

  Rect src(GRID, GRID);
  Rect dst(GRID, GRID);

  // create the base frame
  for (szt i = 0, colS = Size.W / GRID; i < colS; ++i) {
    for (szt j = 0, rowS = Size.H / GRID; j < rowS; ++j) {
      szt index;
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
      src.X = GRID * (index % 4);
      src.Y = GRID * (index / 4);
      dst.X = GRID * i;
      dst.Y = GRID * j;
      spritePage.SetClip(src);
      spritePage.Draw(FrameSurface, dst);
    }
  }

  // create icons
  FrameUp.Init(GRID, GRID);
  FrameIcon.Init(GRID, GRID);
  FrameDown.Init(GRID, GRID);
  src.X = GRID * 2;
  src.Y = GRID * 2;
  spritePage.SetClip(src);
  spritePage.Draw(FrameUp);
  src.X = GRID * 2;
  src.Y = GRID * 1;
  spritePage.SetClip(src);
  spritePage.Draw(FrameIcon);
  src.X = GRID * 1;
  src.Y = GRID * 2;
  spritePage.SetClip(src);
  spritePage.Draw(FrameDown);

  // set up icon locations
  UpDst.W = UpDst.H = GRID;
  UpDst.X = Size.X + Size.W - 2 * GRID;
  UpDst.Y = Size.Y;
  DownDst.W = DownDst.H = GRID;
  DownDst.X = UpDst.X;
  DownDst.Y = Size.Y + Size.H - GRID;
  IconDst.W = IconDst.H = GRID;
  IconDst.X = Size.X + Size.W - GRID;
  IconDst.Y = Size.Y + GRID;

  return true;
}

bool WindowBox::DrawFrame()
{
  if (FrameVisible) {
    FrameSurface.Draw(Size);

    if (ShowUp) {
      FrameUp.Draw(UpDst);
    }
    if (ShowDown) {
      FrameDown.Draw(DownDst);
    }
    if (ShowIcon) {
      FrameIcon.Draw(IconDst);
    }
    return true;
  }
  return false;
}
