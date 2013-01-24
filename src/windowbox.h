#ifndef WINDOWBOX_H
#define WINDOWBOX_H

#include "main.h"
#include "font.h"
#include "surface.h"

class WindowBox
{
public:
  WindowBox() { };
  virtual ~WindowBox() { };

  virtual void Init(const string& Frame = "", const int Bpp = 32);
  void SetSize(Rect NewSize);
  void FixAspectRatio(Rect& NewSize);

  virtual void Draw();
  virtual void Reset();
  bool DrawFrame();
  bool BuildFrame();


public:
  int BPP;
  Rect Size;

  bool Active = false;
  bool Visible = false;

  bool ShowUp = false;
  bool ShowDown = false;
  bool ShowIcon = false;

  size_t AspectW = 0;
  size_t AspectH = 0;

private:
  string FrameName;

  Rect UpDst = { BLOCK_SIZE, BLOCK_SIZE, 0, 0 };
  Rect DownDst = { BLOCK_SIZE, BLOCK_SIZE, 0, 0 };
  Rect IconDst = { BLOCK_SIZE, BLOCK_SIZE, 0, 0 };

  Surface FrameSurface;
  Surface FrameDown;
  Surface FrameUp;
  Surface FrameIcon;
};

#endif // WINDOWBOX_H
