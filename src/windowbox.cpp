#include "windowbox.h"

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
  } else if (AspectH) {
    if (NewSize.H < AspectH * BLOCK_SIZE) {
      NewSize.H = AspectH * BLOCK_SIZE;
    }
  }
}

/** @brief Set Size (resets the page as well)
  */
void WindowBox::SetSize(Rect NewSize)
{
  FixAspectRatio(NewSize);

  NewSize.Blockify();
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
  } else if (spritePage.W != BLOCK_SIZE * 4) {
    //TODO: stretch SpritePage if needed
    LOG("frame image size incorrect");
    return false;
  }

  // sprite image layout
  // 00C 01M 02E 03C   corner middle edge corner
  // 04M 05M 06I 07M   middle middle icon middle
  // 08E 09D 10U 11E   edge    down   up   edge
  // 12C 13M 14E 15C   corner middle edge corner

  Rect src(BLOCK_SIZE, BLOCK_SIZE);
  Rect dst(BLOCK_SIZE, BLOCK_SIZE);

  // create the base frame
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
      src.X = BLOCK_SIZE * (index % 4);
      src.Y = BLOCK_SIZE * (index / 4);
      dst.X = BLOCK_SIZE * i;
      dst.Y = BLOCK_SIZE * j;
      spritePage.SetClip(src);
      spritePage.Draw(FrameSurface, dst);
    }
  }

  // create icons
  FrameUp.Init(BLOCK_SIZE, BLOCK_SIZE);
  FrameIcon.Init(BLOCK_SIZE, BLOCK_SIZE);
  FrameDown.Init(BLOCK_SIZE, BLOCK_SIZE);
  src.X = BLOCK_SIZE * 2;
  src.Y = BLOCK_SIZE * 2;
  spritePage.SetClip(src);
  spritePage.Draw(FrameUp);
  src.X = BLOCK_SIZE * 2;
  src.Y = BLOCK_SIZE * 1;
  spritePage.SetClip(src);
  spritePage.Draw(FrameIcon);
  src.X = BLOCK_SIZE * 1;
  src.Y = BLOCK_SIZE * 2;
  spritePage.SetClip(src);
  spritePage.Draw(FrameDown);

  // set up icon locations
  UpDst.X = Size.X + Size.W - 2 * BLOCK_SIZE;
  UpDst.Y = Size.Y;
  DownDst.X = UpDst.X;
  DownDst.Y = Size.Y + Size.H - BLOCK_SIZE;
  IconDst.X = Size.X + Size.W - BLOCK_SIZE;
  IconDst.Y = Size.Y + BLOCK_SIZE;

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
