#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include "main.h"
#include "windowbox.h"

class ImageBox : public WindowBox
{
public:
  ImageBox() { };
  virtual ~ImageBox() {
    ResetImage();
  };

  bool SetImage(const string& Image);

  void Draw(SDL_Surface* Screen);
  void Reset();

private:
  void ResetImage();

private:
  SDL_Rect ImageDst;
  SDL_Surface* ImageSurface = NULL;
};

#endif // IMAGEBOX_H
