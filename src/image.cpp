#include "image.h"
#include "tokens.h"
#include "mediamanager.h"

Image::Image(MediaManager& Manager,
             const string& AssetName,
             const string& Params)
  : Asset(Manager, AssetName)
{
  const string& params = CleanWhitespace(Params);
  size_t xPos = FindCharacter(params, ',');

  Filename = Media.AssetDir + cutString(params, 0, xPos);

  if (xPos != string::npos) {
    Priority = 1;
    size_t yPos = FindCharacter(params, ',', ++xPos);
    const string& x = cutString(params, xPos, yPos);
    X = intoReal(x);
    if (yPos != string::npos) {
      size_t zoomPos = FindCharacter(params, ',', ++yPos);
      const string& y = cutString(params, yPos, zoomPos);
      Y = intoReal(y);
      if (zoomPos != string::npos) {
        size_t orderPos = FindCharacter(params, ',', ++zoomPos);
        const string& zoom = cutString(params, zoomPos, orderPos);
        Zoom = intoReal(zoom);
        if (orderPos != string::npos) {
          const string& order = cutString(params, orderPos);
          Priority = abs(intoInt(order)) + 1; //0 reserved for BG
        }
      }
    }
  } else {
    BG = true;
  }
}

Image::~Image()
{
}

/** @brief Play
  *
  * @todo: document this function
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

/** @brief Play
  *
  * @todo: document this function
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

/** @brief Play
  *
  * @todo: document this function
  */
bool Image::Tick(real DeltaTime)
{
  if (Playing && Media.Visible) {
    //  reload if the size of the window chaged
    if (WindowSize != Media.ImageWindowSize) {
      Playing = false;
      return Play();
    }
  }
  return false;
}

/** @brief Stop
  *
  * @todo: document this function
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
