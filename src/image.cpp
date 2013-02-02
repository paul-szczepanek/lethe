#include "image.h"
#include "tokens.h"
#include "mediamanager.h"

Image::Image(MediaManager& Manager,
             const string& AssetName,
             const string& Params)
  : Asset(Manager, AssetName)
{
  string params = GetCleanWhitespace(Params);
  sz xPos = FindCharacter(params, ',');
  Filename = Media.AssetDir + CutString(params, 0, xPos);
  if (xPos != string::npos) {
    Priority = 1;
    sz yPos = FindCharacter(params, ',', ++xPos);
    const string& x = CutString(params, xPos, yPos);
    X = IntoReal(x);
    if (yPos != string::npos) {
      sz zoomPos = FindCharacter(params, ',', ++yPos);
      const string& y = CutString(params, yPos, zoomPos);
      Y = IntoReal(y);
      if (zoomPos != string::npos) {
        sz orderPos = FindCharacter(params, ',', ++zoomPos);
        const string& zoom = CutString(params, zoomPos, orderPos);
        Zoom = IntoReal(zoom);
        if (orderPos != string::npos) {
          const string& order = CutString(params, orderPos);
          Priority = abs(IntoInt(order)) + 1; //0 reserved for BG
        }
      }
    }
  } else {
    BG = true;
  }
}

/** @brief Load the image if required, and put in correct order on the stack
  */
bool Image::Play()
{
  if (!Playing && Media.Visible) {
    Playing = true;
    Size = WindowSize = Media.ImageWindowSize;
    if (ImageSurface.LoadImage(Filename)) {
      Size.W = ImageSurface.W;
      Size.H = ImageSurface.H;
      if (BG) {
        Zoom = max((real)WindowSize.W / (real)ImageSurface.W,
                   (real)WindowSize.H / (real)ImageSurface.H);
        Media.BGZoom = Zoom;
        ImageSurface.Zoom(Zoom, Zoom);
        ImageSurface.Trim(WindowSize.W, WindowSize.H);
      } else {
        const real zoom = Zoom * Media.BGZoom;
        ImageSurface.Zoom(zoom, zoom);
        CentreWithin(Size, WindowSize, X, Y);
        //ImageSurface.SetClip(GetCrop(Size, WindowSize));
      }
      ImageSurface.Draw(Media.ImageWindow, Size);
    }
    return true;
  }
  return false;
}

/** @brief Draw on screen if the image window is visible in the reader
  */
bool Image::Draw()
{
  if (Playing && Media.Visible) {
    ImageSurface.Draw(Media.ImageWindow, Size);
    return true;
  }
  return false;
}

Rect GetCrop(const Rect& Size,
             const Rect& Frame)
{
  Rect result;
  //todo
  return result;
}

void CentreWithin(Rect& Size,
                  const Rect& Frame,
                  const real X,
                  const real Y)
{
  Size.X = Frame.X + Frame.W / 2 + X * (Frame.W / 2) - Size.W / 2;
  Size.Y = Frame.Y + Frame.H / 2 + Y * (Frame.H / 2) - Size.H / 2;
}

/** @brief Make sure the image is the correct size for the viewport
  * If required reload the image
  */
bool Image::Tick(real DeltaTime)
{
  if (Playing && Media.Visible) {
    // reload if the size of the window changed
    if (WindowSize != Media.ImageWindowSize) {
      Playing = false;
      return Play();
    }
  }
  return false;
}

/** @brief Unload image
  */
bool Image::Stop()
{
  if (Playing) {
    Playing = false;
    ImageSurface.Unload();
    return true;
  }
  return false;
}
