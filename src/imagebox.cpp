#include "imagebox.h"
#include <SDL_image.h>
#include <SDL_rotozoom.h>

/** @brief Reset Page
  *
  * clears cached surfaces
  */
void ImageBox::ResetImage()
{
  if (ImageSurface) {
    SDL_FreeSurface(ImageSurface);
    ImageSurface = NULL;
  }
}

/** @brief free the SDL surfaces
  */
void ImageBox::Reset()
{
  ResetImage();
  ResetFrame();
}

/** @brief Draw page onto passed in surface
  */
void ImageBox::Draw(SDL_Surface* Screen)
{
  if (Visible && Screen) {
    DrawFrame(Screen);
    if (ImageSurface) {
      SDL_BlitSurface(ImageSurface, 0, Screen, &ImageDst);
    }
  }
}

/** @brief Draw page onto passed in surface
  */
bool ImageBox::SetImage(const string& Image)
{
  if (!Image.empty()) {
    ResetImage();

    ImageDst.w = Size.W - BLOCK_SIZE;
    ImageDst.h = Size.H - BLOCK_SIZE;
    ImageDst.x = Size.X + BLOCK_SIZE / (uint)2;
    ImageDst.y = Size.Y + BLOCK_SIZE / (uint)2;

    ImageSurface = IMG_Load(Image.c_str());

    if (!ImageSurface) {
      ImageSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, ImageDst.w, ImageDst.h,
                                          BPP, MASK_R, MASK_G, MASK_B, MASK_A);
      LOG(Image + " - image missing");
      return false;
    }
  }

  return true;
}
