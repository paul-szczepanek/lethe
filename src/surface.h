#ifndef SURFACE_H
#define SURFACE_H

#include "main.h"

class SDL_Surface;
class SDL_Rect;
class Font;

class Surface
{
public:
  static bool SystemInit();
  static bool SystemDraw();

  Surface() { };
  Surface(size_t Width, size_t Height);
  Surface(const string& Filename);
  virtual ~Surface();

  bool InitScreen(size_t ScreenWidth, size_t ScreenHeight, int ScreenBPP);
  bool Init(size_t Width, size_t Height);
  bool LoadImage(const string& Filename);
  bool Zoom(real X, real Y);
  bool SetAlpha(size_t Alpha);
  bool Draw(Surface& Destination, const Rect& Position);
  bool Draw(Surface& Destination);
  bool Draw(const Rect& Position);
  bool Draw(const size_t X, const size_t Y);
  bool Draw();
  bool DrawRectangle(const Rect& Rectangle, usint R, usint G, usint B);
  void SetClip(const Rect& NewClip);
  bool Blank();
  bool CreateText(const Font& TextFont, const string& Text,
                  usint R, usint G, usint B);
  bool PrintText(const Rect& Position, const Font& TextFont, const string& Text,
                 usint R, usint G, usint B);

private:
  bool OnInit();

public:
  size_t W = 0;
  size_t H = 0;

  static SDL_Surface* Screen;
  static int BPP;

private:
  Rect Clip = { 0, 0, 0, 0 };
  SDL_Surface* SDLSurface = NULL;
};

inline void Blit(SDL_Surface* SDLSurface, Rect& Clip, SDL_Surface* Destination,
                 SDL_Rect* Dst);

#endif // SURFACE_H
