#ifndef READER_H
#define READER_H

#include "main.h"
#include "book.h"
#include "textbox.h"
#include "imagebox.h"
#include "layout.h"
#include "surface.h"
#include "font.h"
#include "input.h"

const size_t NUM_LAYOUTS = 2;
const real REDRAW_TIMEOUT = 5.0;
const real MIN_TIMEOUT = 0.1;

class Book;

enum orientation {
  landscape,
  portrait,
  ORIENTATION_MAX
};

class Reader
{
public:
  Reader(int ReaderWidth, int ReaderHeight, int ReaderBPP);
  virtual ~Reader();

  bool Init();
  bool Tick(real DeltaTime);

private:
  void RedrawScreen(real DeltaTime);
  void DrawWindows();
  void DrawBackdrop();
  void PrintFPS(real DeltaTime);

  bool ProcessInput(real DeltaTime);

  bool ReadBook();
  bool ReadMenu();

  size_t SetLayout(size_t LayoutIndex = NUM_LAYOUTS);
  size_t FixLayout();
  Layout& GetCurrentLayout();


public:
  string MenuSource;
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
  Font FontMain;
  Font FontSmall;
  Font FontTitle;
  Font FontSys;

  // logic
  Book MyBook;
  string_pair KeywordAction;
  MouseState Mouse; // we want to remember mouse position between clicks

  // layout
  orientation CurrentOrientation = landscape;
  size_t CurrentLayout = 0;
  Layout Layouts[ORIENTATION_MAX][NUM_LAYOUTS];

  // main windows
  ImageBox MainImage;
  TextBox MainText;
  TextBox QuickMenu;
  WindowBox ReaderButtons;

  // popups
  TextBox MainMenu;
  TextBox VerbMenu;

  real Timeout = MIN_TIMEOUT;
  real TimeoutTimer = 0;

#ifdef LOGGER
  TextBox* Logger = NULL;
#endif
};

#endif // READER_H
