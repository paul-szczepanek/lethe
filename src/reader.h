#ifndef READER_H
#define READER_H

#include "main.h"
#include "textbox.h"
#include "book.h"
#include "imagebox.h"
#include "layout.h"
#include "mediamanager.h"
#include "surface.h"
#include "font.h"

const size_t NUM_LAYOUTS = 2;
const real REDRAW_TIMEOUT = 1.0;
const real MIN_TIMEOUT = 0.1;

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

  void ReadBook();

  size_t SetLayout(size_t LayoutIndex = NUM_LAYOUTS);
  size_t FixLayout();
  Layout& GetCurrentLayout();

  string PageSource;
  string QuickMenuSource;

private:
  int BPP;
  size_t Width;
  size_t Height;
  float RedrawCountdown = REDRAW_TIMEOUT;
  bool RedrawPending = true;
  // screen
  Surface Screen;
  Surface Backdrop;

  // media
  MediaManager Media;
  Font FontMain;
  Font FontSmall;
  Font FontTitle;
  Font FontSys;

  // logic
  Book MyBook;
  string_pair KeywordAction;
  MouseState Mouse;
  bool Quit = false;

  // layout
  orientation CurrentOrientation = landscape;
  size_t CurrentLayout = 0;
  Layout Layouts[ORIENTATION_MAX][NUM_LAYOUTS];

  // main windows
  ImageBox MainImage;
  TextBox MainText;
  TextBox QuickMenu;
  WindowBox MainMenu;

  // popups
  TextBox ChoiceMenu;
  TextBox VerbMenu;

  real Timeout = MIN_TIMEOUT;
  real TimeoutTimer = 0;

#ifdef LOGGER
  TextBox* Logger = NULL;
#endif
};

#endif // READER_H
