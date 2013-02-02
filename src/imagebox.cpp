#include "imagebox.h"

/** @brief Draw page onto passed in surface
  */
void ImageBox::Draw()
{
  if (Visible) {
    DrawFrame();
    ImageSurface.Draw(Dst);
  }
}

/** @brief Draw page onto passed in surface
  */
bool ImageBox::SetImage(const string& Image)
{
  if (Image.empty()) {
    return false;
  } else {
    Dst.W = Size.W - BLOCK_SIZE;
    Dst.H = Size.H - BLOCK_SIZE;
    Dst.X = Size.X + BLOCK_SIZE / (sz)2;
    Dst.Y = Size.Y + BLOCK_SIZE / (sz)2;
    if (!ImageSurface.LoadImage(Image)) {
      ImageSurface.Init(Dst.W, Dst.H);
      return false;
    }
  }
  return true;
}
