#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include "main.h"
#include "windowbox.h"
#include "surface.h"

class ImageBox : public WindowBox
{
public:
  ImageBox() { };
  virtual ~ImageBox() { };

  bool SetImage(const string& Image);
  void Draw();

private:
  void ResetImage();


private:
  Rect Dst;
  Surface ImageSurface;
};

#endif // IMAGEBOX_H
