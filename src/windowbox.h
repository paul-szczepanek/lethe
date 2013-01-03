#ifndef WINDOWBOX_H
#define WINDOWBOX_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "main.h"

class SDL_Surface;

class WindowBox
{
public:
  WindowBox() { };
  virtual ~WindowBox() {
    ResetFrame();
  };

  void Init(TTF_Font* Font = NULL, const string& Frame = "", int Bpp = 32);
  void SetSize(Rect NewSize);

  virtual void Draw(SDL_Surface* Screen);
  bool DrawFrame(SDL_Surface* Screen);

  virtual void Reset();
  void ResetFrame();

public:
  int BPP;
  Rect Size;

  bool Active = false;
  bool Visible = false;

  bool ShowUp = false;
  bool ShowDown = false;
  bool ShowIcon = false;

  TTF_Font* FontMain = NULL;

  size_t AspectW = 0;
  size_t AspectH = 0;

private:
  string FrameName;

  SDL_Rect FrameDst;
  SDL_Rect UpDst;
  SDL_Rect DownDst;
  SDL_Rect IconDst;

  SDL_Surface* FrameSurface = NULL;
  SDL_Surface* FrameDown = NULL;
  SDL_Surface* FrameUp = NULL;
  SDL_Surface* FrameIcon = NULL;
};

#endif // WINDOWBOX_H
