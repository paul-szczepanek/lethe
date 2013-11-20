#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include "main.h"
#include "windowbox.h"
#include "surface.h"

class MediaManager;

class ImageBox : public WindowBox
{
public:
  ImageBox() { };
  virtual ~ImageBox() { };

  void Reset();

  bool SetImage(const string& Image);
  inline void SetMediaManager(MediaManager* NewManager);
  void Draw();

private:
  void ResetImage();
  MediaManager* ImageManager = NULL;

private:
  Rect Dst;
  Surface ImageSurface;
};


void ImageBox::SetMediaManager(MediaManager* NewManager)
{
  ImageManager = NewManager;
  Reset();
};

#endif // IMAGEBOX_H
