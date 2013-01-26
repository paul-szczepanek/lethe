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
  Surface(lint Width, lint Height);
  Surface(const string& Filename);
  ~Surface();

  static bool SystemInit();
  static bool SystemDraw();

  bool InitScreen(lint ScreenWidth, lint ScreenHeight, int ScreenBPP);
  bool Init();
  bool Init(lint Width, lint Height);
  bool LoadImage(const string& NewFilename = "");
  bool Zoom(real X, real Y);
  bool SetAlpha(usint Alpha);
  bool Draw(Surface& Destination, const Rect& Position);
  bool Draw(Surface& Destination);
  bool Draw(const Rect& Position);
  bool Draw(const lint X, const lint Y);
  bool Draw();
  bool DrawRectangle(const Rect& Rectangle, usint R, usint G, usint B);
  void Trim(lint ClipW, lint ClipH);
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
  lint W = 0;
  lint H = 0;
  static int BPP;

  static lint ScreenW;
  static lint ScreenH;

private:
  static SDL_Surface* Screen;
  string Filename;

  Rect Clip = { 0, 0, 0, 0 };
  SDL_Surface* SDLSurface = NULL;
};

inline void Blit(SDL_Surface* SDLSurface, Rect& Clip, SDL_Surface* Destination,
                 SDL_Rect* Dst);

#endif // SURFACE_H
