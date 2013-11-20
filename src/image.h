#ifndef IMAGE_H
#define IMAGE_H

#include "asset.h"
#include "surface.h"

/** @class This controls an asset that shows an image in an imagebox.
 *  This is the high level class and it uses Surface to show the image.
 *
 * It will automatically adapt the zoom to match the pixel scale of the
 * current background image.
 */

class Image : public Asset
{
public:
  Image(MediaManager& Manager, const string& AssetName , const string& Params);
  ~Image() { };

  bool Play();
  bool Stop();
  bool Tick(real DeltaTime);
  bool Draw();


private:
  Surface ImageSurface;
  Rect Size;
  Rect WindowSize;

  real X = 0;
  real Y = 0;
  real Zoom = 1.0;

  bool BG = false;
};

void CentreWithin(Rect& Size, const Rect& Frame,
                  const real X = 0, const real Y = 0);

#endif // IMAGE_H
