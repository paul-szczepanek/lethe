#ifndef READER_H
#define READER_H

#include "main.h"
#include "book.h"
#include "textbox.h"
#include "imagebox.h"
#include "dialogbox.h"
#include "layout.h"
#include "surface.h"
#include "font.h"
#include "input.h"
#include "valuestore.h"

const size_t NUM_LAYOUTS = 2;

class Book;

enum orientation {
  landscape,
  portrait,
  ORIENTATION_MAX
};

class Reader
{
public:
  Reader(int ReaderWidth, int ReaderHeight, int ReaderBPP, bool Sound = true);
  ~Reader();

  bool Init();
  bool Tick(real DeltaTime);

private:
  bool InitFonts();
  void RedrawScreen(real DeltaTime);
  void DrawWindows();
  void DrawBackdrop();
  void PrintFPS(real DeltaTime);

  bool ProcessInput(real DeltaTime);
  bool ProcessDialogs();

  bool ReadBook();
  bool ReadMenu();

  size_t SetLayout(size_t LayoutIndex = NUM_LAYOUTS);
  size_t FixLayout();
  Layout& GetCurrentLayout();

  void LoadSettings();
  void SaveSettings();


public:
  string MenuSource;
  string PageSource;
  string QuickMenuSource;

private:
  size_t Width;
  size_t Height;
  int BPP;
  bool Silent;
  float RedrawCountdown = 0;
  bool RedrawPending = true;
  ValueStore Settings;
  // screen
  Surface Screen;
  Surface Backdrop;

  // media
  Font FontSys;
  vector<Font> Fonts;
  vector<string> FontNames;
  size_t FontScale = 100;

  // logic
  Book MyBook;
  string_pair KeywordAction;
  MouseState Mouse; // we want to remember mouse position between clicks
  KeysState Keys;

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
  DialogBox GameDialog;
  size_t DialogIndex = 0;

  real Timeout = 0;
  real TimeoutTimer = 0;

#ifdef LOGGER
  TextBox* Logger = NULL;
#endif
};

void OverwriteValues(vector<string>& defaults, const vector<string>& stored);

#endif // READER_H
