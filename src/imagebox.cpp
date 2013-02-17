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
    Dst.W = Size.W - GRID;
    Dst.H = Size.H - GRID;
    Dst.X = Size.X + GRID / (szt)2;
    Dst.Y = Size.Y + GRID / (szt)2;
    if (!ImageSurface.LoadImage(Image)) {
      ImageSurface.Init(Dst.W, Dst.H);
      return false;
    }
  }
  return true;
}
