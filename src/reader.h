#ifndef READER_H
#define READER_H

#include "main.h"
#include "session.h"
#include "textbox.h"

class Session;
class Book;

struct MouseState {
  MouseState ()
    : X(0), Y(0), Left(false), Middle(false), Right(false),
      LeftUp(false), MiddleUp(false), RightUp(false) { };
  ~MouseState () { };
  Uint16 X;
  Uint16 Y;
  bool Left;
  bool Middle;
  bool Right;
  bool LeftUp;
  bool MiddleUp;
  bool RightUp;
};

class Reader
{
public:
  Reader();
  virtual ~Reader();

  bool Init(int width, int height, int bpp);
  bool Tick(real DeltaTime);

  void RedrawScreen(real DeltaTime);
  void PrintFPS(real DeltaTime);
  void DrawPage();
  void DrawBackdrop();

  void ProcessInput();
  bool ProcessEvents();

  string PageSource;

private:
  bool Quit;

  Session Progress;

  MouseState Mouse;

  SDL_Surface* Screen;
  SDL_Surface* SysBG;
  SDL_Surface* Backdrop;

  TTF_Font* FontMain;
  TTF_Font* FontSys;

  Book* MyBook;

  TextBox MainText;
  TextBox ChoiceMenu;
  TextBox SideMenu;
  TextBox VerbMenu;

  real SplitH;
  real SplitV;

  bool RedrawPending;
  float RedrawCountdown;

#ifdef LOGGER
private:
  TextBox* Logger;
public:
  //static string GLog;
#endif
};

#endif // READER_H
