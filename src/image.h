#ifndef IMAGE_H
#define IMAGE_H

#include "asset.h"
#include "surface.h"

class Image : public Asset
{
public:
  Image(MediaManager& Manager, const string& Params);
  virtual ~Image();

  virtual void Play();
  virtual void Stop();

private:
  Surface ImageSurface;
};

#endif // IMAGE_H
