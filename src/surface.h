#ifndef SURFACE_H
#define SURFACE_H

#include "main.h"

class SDL_Surface;
class SDL_Rect;
class Font;

class Surface
{
public:
  Surface() { };
  Surface(size_t Width, size_t Height);
  Surface(const string& Filename);
  ~Surface();

  static bool SystemInit();
  static bool SystemDraw();

  bool InitScreen(size_t ScreenWidth, size_t ScreenHeight, int ScreenBPP);
  bool Init();
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
  void Trim(size_t ClipW, size_t ClipH);
  void SetClip(const Rect& NewClip);
  bool Blank();
  bool Unload();
  bool CreateText(const Font& TextFont, const string& Text,
                  usint R = 255, usint G = 255, usint B = 255);
  bool PrintText(const Rect& Position, const Font& TextFont, const string& Text,
                 usint R = 255, usint G = 255, usint B = 255);

private:
  bool OnInit();


public:
  size_t W = 0;
  size_t H = 0;
  static int BPP;

  static size_t ScreenW;
  static size_t ScreenH;

private:
  static SDL_Surface* Screen;

  Rect Clip = { 0, 0, 0, 0 };
  SDL_Surface* SDLSurface = NULL;
};

inline void Blit(SDL_Surface* SDLSurface, Rect& Clip, SDL_Surface* Destination,
                 SDL_Rect* Dst);

#endif // SURFACE_H
