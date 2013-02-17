#ifndef WINDOWBOX_H
#define WINDOWBOX_H

#include "main.h"
#include "font.h"
#include "surface.h"

#define GRID WindowBox::Grid

class WindowBox
{
public:
  WindowBox() { };
  virtual ~WindowBox() { };

  virtual void Init(const string& Frame = "", const int Bpp = 32);
  void SetSize(Rect NewSize);
  void FixAspectRatio(Rect& NewSize);

  virtual void Draw();
  virtual void Reset() { };
  bool DrawFrame();
  bool BuildFrame();


public:
  static lint Grid;

  int BPP;
  Rect Size;

  bool Active = false;
  bool FrameVisible = false;
  bool Visible = false;

  bool ShowUp = false;
  bool ShowDown = false;
  bool ShowIcon = false;

  int AspectW = 0;
  int AspectH = 0;

private:
  string FrameName;

  Rect UpDst;
  Rect DownDst;
  Rect IconDst;

  Surface FrameSurface;
  Surface FrameDown;
  Surface FrameUp;
  Surface FrameIcon;
};

#endif // WINDOWBOX_H
