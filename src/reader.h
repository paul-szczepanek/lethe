#ifndef READER_H
#define READER_H

#include "main.h"
#include "textbox.h"
#include "imagebox.h"
#include "layout.h"

const size_t NUM_LAYOUTS = 2;
const real REDRAW_TIMEOUT = 1.0;

class Book;

enum orientation {
  landscape,
  portrait,
  ORIENTATION_MAX
};

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
  Reader(int ReaderWidth, int ReaderHeight, int ReaderBPP);
  virtual ~Reader();

  bool Init();
  bool Tick(real DeltaTime);

  void RedrawScreen(real DeltaTime);
  void PrintFPS(real DeltaTime);
  void DrawWindows();
  void DrawBackdrop();

  void ProcessInput();

  bool ReadBook();

  size_t SetLayout(size_t LayoutIndex = NUM_LAYOUTS);
  size_t FixLayout();
  Layout& GetCurrentLayout();

  string PageSource;
  string QuickMenuSource;

private:
  bool Quit = false;
  bool RedrawPending = true;

  orientation CurrentOrientation = landscape;
  size_t CurrentLayout = 0;

  Layout Layouts[ORIENTATION_MAX][NUM_LAYOUTS];

  int BPP;
  size_t Width;
  size_t Height;

  MouseState Mouse;

  SDL_Surface* Screen;
  SDL_Surface* Backdrop;

  TTF_Font* FontMain;
  TTF_Font* FontSys;

  Book* MyBook;

  // part of the main layout
  ImageBox MainImage;
  TextBox MainText;
  TextBox QuickMenu;
  WindowBox MainMenu;

  // popups
  TextBox ChoiceMenu;
  TextBox VerbMenu;

  string_pair KeywordAction;

  float RedrawCountdown = REDRAW_TIMEOUT;

#ifdef LOGGER
  TextBox* Logger = NULL;
#endif
};

#endif // READER_H
