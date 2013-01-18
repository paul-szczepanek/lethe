#ifndef IMAGE_H
#define IMAGE_H

#include "asset.h"
#include "surface.h"

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

Rect GetCrop(const Rect& Size, const Rect& Frame);
void CentreWithin(Rect& Size, const Rect& Frame,
                  const real X = 0, const real Y = 0);

#endif // IMAGE_H
