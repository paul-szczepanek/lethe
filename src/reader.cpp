#include "reader.h"
#include "audio.h"
#include "input.h"

#ifdef DEVBUILD
#include "disk.h"
#endif

const string FRAME_SOLID = "solid";
const string FRAME_TEXT = "text";
const string FRAME_MENU = "menu";
const string FRAME_IMAGE = "image";

const string SKEY_CURRENT_LAYOUT = "current layout";
const string SKEY_FONT_NAMES = "fonts";
const string SKEY_FONT_SCALE = "font scale";
const string SKEY_LAYOUTSH = "layouts horizontal";
const string SKEY_LAYOUTSV = "layouts vertical";
const string SKEY_SCREENW = "screen width";
const string SKEY_SCREENH = "screen height";
const string SKEY_GRID = "grid size";

const string QUICK_BOOKMARK = "Quick bookmark";

const real REDRAW_TIMEOUT = 5.0;
const real MIN_TIMEOUT = 0.1;

Reader::Reader(lint ReaderWidth, lint ReaderHeight, int ReaderBPP, bool Sound)
  : Width(ReaderWidth), Height(ReaderHeight), BPP(ReaderBPP), Silent(!Sound),
    Settings(SETTINGS_FILE)
{
  LoadSettings();
}

Reader::~Reader()
{
  SaveSettings();
#ifdef DEVBUILD
  Disk::Write("log", GLog);
#endif
}

void Reader::LoadSettings()
{
  // default layouts
  vector<string> layoutStringsH = { "0132 3111 1111 0 40",
                                    "1032 3311 1111 0 60",
                                    "0132 3111 1011 0 60"
                                  };
  vector<string> layoutStringsV = { "1032 3311 1111 0 70",
                                    "1032 0311 1111 1 60",
                                    "0132 3111 1011 0 70"
                                  };
  // overwrite with stored ones
  Settings.GetValue(SKEY_LAYOUTSH, layoutStringsH);
  Settings.GetValue(SKEY_LAYOUTSV, layoutStringsV);
  // initialise the layouts with default or stored layouts strings
  for (szt i = 0; i < NUM_LAYOUTS; ++i) {
    Layouts[landscape][i].Init(layoutStringsH[i]);
    Layouts[portrait][i].Init(layoutStringsV[i]);
  }

  Settings.GetValue(SKEY_CURRENT_LAYOUT, CurrentLayout);

  // font defaults
  FontNames = { "main.ttf", "title.ttf", "quote.ttf", "mono.ttf" };
  FontNames.resize(TEXT_STYLE_MAX);
  // replaced defaults with saved ones if present
  Settings.GetValue(SKEY_FONT_NAMES, FontNames);

  Settings.GetValue(SKEY_FONT_SCALE, FontSize);

  if (!Width || !Height) {
    Settings.GetValue(SKEY_SCREENW, Width);
    Settings.GetValue(SKEY_SCREENH, Height);
    if (!Width || !Height) {
      Width = 640;
      Height = 480;
    }
  }
  Settings.GetValue(SKEY_GRID, GRID);

  Timeout = MIN_TIMEOUT;
}

void Reader::SaveSettings()
{
  // save reader layouts
  vector<string> layoutStringsH;
  for (szt j = 0; j < NUM_LAYOUTS; ++j) {
    layoutStringsH.push_back(Layouts[landscape][j].GetDefinition());
  }
  Settings.SetValue(SKEY_LAYOUTSH, layoutStringsH);
  vector<string> layoutStringsV;
  for (szt j = 0; j < NUM_LAYOUTS; ++j) {
    layoutStringsV.push_back(Layouts[portrait][j].GetDefinition());
  }
  Settings.SetValue(SKEY_LAYOUTSV, layoutStringsV);
  // current layout
  Settings.SetValue(SKEY_CURRENT_LAYOUT, IntoString(CurrentLayout));
  // fonts
  Settings.SetValue(SKEY_FONT_NAMES, FontNames);
  Settings.SetValue(SKEY_FONT_SCALE, FontSize);
  // screen
  Settings.SetValue(SKEY_SCREENW, Width);
  Settings.SetValue(SKEY_SCREENH, Height);
  Settings.SetValue(SKEY_GRID, GRID);
}

bool Reader::InitFonts()
{
  szt fonstSizes[TEXT_STYLE_MAX] = { 22, 46, 16, 16 };
  Fonts.resize(TEXT_STYLE_MAX);

#ifdef __ANDROID__
  const lint smallScreen = min(Screen.W, Screen.H);
  // we need some fallback if we're running on a low resolution screen
  real androidFactor = 1;
  // scale the fontScale for android screens that are tiny
  if (smallScreen < 400) {
    GRID = 16;
    androidFactor = (real)400 / (real)smallScreen;
  }
  const real scale = (real)FontSize / (100.f * androidFactor);
#else
  const real scale = (real)FontSize / 100.f;
#endif

  for (szt i = 0; i < TEXT_STYLE_MAX; ++i) {
    if (Fonts[i].Init(FontNames[i], fonstSizes[i] * scale)) {
      Fonts[i].Style = textStyle(i);
    } else {
      return false;
    }
  }
  if (!FontSys.Init("mono.ttf", 12 * scale)) {
    return false;
  }
  return true;
}

void Reader::InitWindows()
{
  // Windows not part of the dynamic layout use the screen size
  const Rect mainWindowSize(Width - 2 * GRID, Height - 2 * GRID,
                            GRID, GRID);

  // popups
  VerbMenu.Init(Fonts, FRAME_SOLID, BPP);
  GameDialog.Init(Fonts, FRAME_SOLID, BPP);
  GameDialog.CentreMain = true;
  GameDialog.SetSize(mainWindowSize);
  // noun menu with minimum width
  QuickMenu.Init(Fonts, FRAME_MENU, BPP);
  QuickMenu.AspectW = 4;
  // main game view with minimum width
  MainText.Init(Fonts, FRAME_TEXT, BPP);
  MainText.AspectW = 4;
  // the game menu with centred text
  MainMenu.Init(Fonts, FRAME_SOLID, BPP);
  MainMenu.CentreMain = true;
  MainMenu.Visible = true;
  MainMenu.SetSize(mainWindowSize);
  // the image window with a fixed aspect ration
  MainImage.Init(FRAME_IMAGE, BPP);
  MainImage.AspectW = 4;
  MainImage.AspectH = 2;
  // the reader buttons with buttons ordered by importance as they
  // disappear when there is not enough space for them
  ReaderButtons.Init(FRAME_MENU, BPP);
  ReaderButtons.AspectW = 100; // keep the buttons on a thin line
  ReaderButtons.AspectH = 2;
  ReaderButtons.AddButton(buttonMenu);
  ReaderButtons.AddButton(buttonLayout);
  ReaderButtons.AddButton(buttonBookmark);
  ReaderButtons.AddButton(buttonHistory);
  ReaderButtons.AddButton(buttonDisk);
  // set buttons to reflect game variables
  ReaderButtons.SetButtonState(buttonLayout, CurrentLayout);

#ifdef DEVBUILD
  GLog = "";
  Logger.Visible = true;
  Logger.Init(FontSys, "", BPP);
  Logger.RawMode = true;
  VarView.Visible = false;
  VarView.Init(FontSys, FRAME_SOLID, BPP);
  VarView.RawMode = true;
#endif
}

bool Reader::Init()
{
  // initialise all the critical systems and assets
  if (!Surface::SystemInit()
      || !Screen.InitScreen(Width, Height, BPP)
      || !Font::SystemInit()
      || !Audio::SystemInit(Silent)
      || !InitFonts()) {
    return false;
  }

  if (!Backdrop.LoadImage("data/bg.png")) {
    Backdrop.Init(Width, Height);
  }
  InitWindows();
  SetLayout();
  MyBook.ShowMenu();
  MainImage.SetMediaManager(MyBook.GetMediaManagerPointer());

  FontSizeSetting = &MyBook.GetMenuValues(FONT_SIZE);
  FontSizeSetting->IntValue = FontSize;

  return true;
}

bool Reader::Tick(real DeltaTime)
{
  if (TimeoutTimer >= 0) {
    TimeoutTimer -= DeltaTime;
  } else {
    ProcessInput(DeltaTime);
    // process the action queue
    if (MyBook.DialogOpen) {
      RedrawPending |= ProcessDialogs();
    } else if (MyBook.MenuOpen) {
      RedrawPending |= ReadMenu();
    } else if (MyBook.BookOpen) {
      RedrawPending |= ReadBook();
    } else {
      // both menu and book are closed, quit
      return false;
    }
    // game logic might have hidden the menu
    MainMenu.Visible = MyBook.MenuOpen;
  }

  RedrawPending |= MyBook.Tick(DeltaTime);

  if (RedrawPending || RedrawCountdown < 0.f) {
    RedrawScreen(DeltaTime);
    RedrawPending = false;
    RedrawCountdown = REDRAW_TIMEOUT;
  } else {
    RedrawCountdown -= DeltaTime;
  }
  return true;
}

/** @brief Show new dialogs in the array until no more left.
  * Builds the text box from the dialog definition
  */
bool Reader::ProcessDialogs()
{
  if (MyBook.DialogOpen && !GameDialog.Visible) {
    if (DialogIndex >= MyBook.Dialogs.size()) {
      // all the dialogs displayed, reset the stack
      MyBook.DialogOpen = false;
      MyBook.Dialogs.clear();
      DialogIndex = 0;
    } else {
      // show the dialog
      string text;
      GameDialog.Visible = true;
      const Dialog& dialog = MyBook.Dialogs[DialogIndex];
      // prepare the dialog text
      if (dialog.InputBox) {
        GameDialog.EnableInput(dialog.Noun, dialog.Message);
        text += "\n\n";
      } else {
        text += dialog.Message + '\n';
      }
      for (const string& button : dialog.Buttons) {
        text += "\n<" + button + '[' + dialog.Noun + ':' + button + "]>\n";
      }
      GameDialog.SetText(text);
      // centre it on the screen
#ifdef __ANDROID__
      GameDialog.Size.W = Width;
      GameDialog.Size.X = 0;
#else
      GameDialog.Size.W = Width - 2 * GRID;
      GameDialog.Size.X = GRID;
#endif
      GameDialog.Size.H = GameDialog.PageHeight + 2 * GRID;
      GameDialog.Size.Y = (Height - GameDialog.Size.H) / 2;
      GameDialog.SetSize(GameDialog.Size);
    }
    return true;
  }
  return false;
}

/** @brief Check the story for pending actions and execute them if needed
  */
bool Reader::ReadBook()
{
  if (MyBook.IsActionQueueEmpty()) {
    return false;
  }
  TimeoutTimer = Timeout;

  // fill the text from the book
  PageSource = MyBook.ProcessStoryQueue();
#ifdef DEVBUILD
  VarViewSource = MyBook.ShowVariables() + GTrace;
#endif
  QuickMenuSource = MyBook.GetQuickMenu();
  // clear out old keywords
  MainText.ValidKeywords.Reset();
  MyBook.GetStoryNouns(MainText.ValidKeywords);
  // actually set the received text to be shown
  MainText.AddText(PageSource);
  QuickMenu.SetText(QuickMenuSource);

  return true;
}

/** @brief Check the menu for pending actions and execute them if needed
  */
bool Reader::ReadMenu()
{
  if (MyBook.IsActionQueueEmpty()) {
    return false;
  }
  TimeoutTimer = Timeout;

  MainMenu.SetText("");
  MenuSource = MyBook.ProcessMenuQueue();
  RefreshMenu();

  return true;
}

void Reader::RefreshMenu()
{
#ifdef __ANDROID__
  Rect maxSize(Width, Height, 0, 0);
#else
  Rect maxSize(Width - 2 * GRID, Height, GRID, 0);
#endif
  // check if we need to resize fonts
  if (FontSizeSetting->Dirty) {
    FontSizeSetting->Dirty = false;
    // enforce minimum
    if (FontSizeSetting->IntValue <= 0) {
      FontSizeSetting->IntValue = 10;
    }
    if (FontSize != (szt)FontSizeSetting->IntValue) {
      FontSize = FontSizeSetting->IntValue;
      InitFonts();
    }
  }
  MainMenu.SetSize(maxSize);
  MainMenu.SetText(MenuSource);
  Rect fitSize = MainMenu.GetTextSize();
  fitSize.X += (maxSize.W - fitSize.W) / 2;
  fitSize.Y += (maxSize.H - fitSize.H) / 2;
  MainMenu.SetSize(fitSize);
}

bool Reader::ProcessInput(real DeltaTime)
{
  Keys.InputMode = MyBook.DialogOpen;
  SystemState System;
  RedrawPending = Input::Tick(Mouse, Keys, System);

  if (System.Resized) {
    // resize all the textboxes and ignore other input this time
    Width = System.W;
    Height = System.H;
    Screen.InitScreen(Width, Height, BPP);
    SetLayout();
    RefreshMenu();
    RedrawPending = true;
  } else if (Mouse.LeftUp) {
    // releasing the button commits to an action
    // verb -> dialog -> menu -> quick -> main text -> buttons
    if (VerbMenu.Visible) {
      if (VerbMenu.GetSelectedKeyword(KeywordAction.Y)) {
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else {
          MyBook.SetStoryAction(KeywordAction);
        }
        // clean up after action
        KeywordAction.clear();
        VerbMenu.Visible = false;
      }
    } else if (GameDialog.Visible) {
      if (GameDialog.GetSelectedKeyword(KeywordAction.X)) {
        // close the dialog and get ready to process the next one
        ++DialogIndex;
        GameDialog.Visible = false;
        // if the dialog had input, send that input to the noun
        if (GameDialog.InputBox) {
          // if it's possibly a number add that as well
          string number;
          for (const char digit : GameDialog.InputText) {
            if (isdigit(digit)) {
              number += digit;
            } else {
              break;
            }
          }
          if (!number.empty()) {
            GameDialog.InputText += "+#" + number;
          }
          KeywordAction.Y = "=" + GameDialog.InputText;
        }
      }
    } else if (MainMenu.Visible) {
      MainMenu.GetSelectedKeyword(KeywordAction.X);
    } else if (QuickMenu.GetSelectedKeyword(KeywordAction.X)
               || MainText.GetSelectedKeyword(KeywordAction.X)) {
      if (!MyBook.ActiveBranch) {
        // show a pop-up to ask to branch the story
        KeywordAction.clear();
      }
    } else if (ReaderButtons.GetSelectedButton(ButtonAction, ButtonState)) {
      switch (ButtonAction) {
        case buttonMenu:
          ReaderButtons.SetButtonState(buttonMenu, 0);
          MyBook.ShowMenu();
          break;
        case buttonLayout:
          SetLayout(ButtonState);
          break;
        case buttonHistory:
          ReaderButtons.SetButtonState(buttonHistory, 0);
          MyBook.ShowHistoryMenu();
          break;
        case buttonDisk:
          ReaderButtons.SetButtonState(buttonDisk, 0);
          MyBook.ShowSaveMenu();
          break;
        case buttonBookmark:
          ReaderButtons.SetButtonState(buttonBookmark, 0);
          MyBook.ShowBookmarkMenu();
          break;

        default:
          break;
      }
      ButtonAction = BUTTON_TYPE_MAX;
    }

    Mouse.LeftUp = false;

    if (!KeywordAction.X.empty()) {
      if (MyBook.GetChoice(KeywordAction)) {
        // the keyword is a choice and has both noun and verb ready
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else {
          MyBook.SetStoryAction(KeywordAction);
        }
        KeywordAction.clear();
      } else if (!VerbMenu.Visible) {
        // we clicked on a keyword, create a menu full of verbs
        if (MainMenu.Visible) {
          const string& verbsText = MyBook.GetMenuVerbs(KeywordAction.X);
          ShowVerbMenu(verbsText);
        } else {
          const string& verbsText = MyBook.GetStoryVerbs(KeywordAction.X);
          ShowVerbMenu(verbsText);
        }
      }
    }
  } else if (Keys.KeyPressed) {
    // keyboard shortcuts
    if (Keys.InputMode) {
      GameDialog.AddCharacter(Keys.Letter);
    } else if (Keys.SplitShrink) {
      GetCurrentLayout().ChangeSplit(-GetGridPercentage());
      SetLayout();
    } else if (Keys.SplitGrow) {
      GetCurrentLayout().ChangeSplit(GetGridPercentage());
      SetLayout();
    } else if (Keys.LayoutToggle) {
      SetLayout((CurrentLayout + 1) % NUM_LAYOUTS);
    } else if (Keys.PgDown) {
      MainText.Scroll(100);
    } else if (Keys.PgUp) {
      MainText.Scroll(-100);
#ifdef DEVBUILD
    } else if (Keys.Console) {
      VarView.Visible = !VarView.Visible;
#endif
    } else if (Keys.Escape) {
      if (MyBook.MenuOpen) {
        MyBook.HideMenu();
      } else {
        MyBook.ShowMenu();
      }
    } else if (Keys.Menu) {
      if (MyBook.MenuOpen) {
        MyBook.HideMenu();
      } else {
        MyBook.ShowMenu();
      }
    } else if (Keys.ImageZoom) {
      //
    } else if (Keys.Undo) {
      MyBook.UndoSnapshot();
    } else if (Keys.Redo) {
      MyBook.RedoSnapshot();
    } else if (Keys.Bookmark) {
      MyBook.SetBookmark(QUICK_BOOKMARK);
    }
    if (Keys.Quit) {
      MyBook.Quit();
    }
    Keys.Reset();
  }

  if (VerbMenu.Visible) {
    // if we click anywhere outside the verb menu, hide it
    if (!VerbMenu.HandleInput(Mouse, DeltaTime) && Mouse.Left) {
      VerbMenu.Visible = false;
      KeywordAction.clear();
    }
  } else if (GameDialog.Visible) {
    GameDialog.HandleInput(Mouse, DeltaTime);
  } else if (MainMenu.Visible) {
    MainMenu.HandleInput(Mouse, DeltaTime);
  } else {
    QuickMenu.HandleInput(Mouse, DeltaTime);
    ReaderButtons.HandleInput(Mouse);
    MainText.HandleInput(Mouse, DeltaTime);
  }

  return true;
}



bool Reader::ShowVerbMenu(const string& VerbsText)
{
  if (VerbsText.empty()) {
    return false;
  }
  Rect verbSize(Width, Height, Mouse.X, Mouse.Y);
  // break text according to maximum size (whole screen)
  VerbMenu.SetSize(verbSize);
  VerbMenu.SetText(VerbsText);
  // check if it's off screen when sized down to minimum size
  Rect fitSize = VerbMenu.GetTextSize();
  if (fitSize.X + fitSize.W > Width) {
    fitSize.X = Width - fitSize.W;
  }
  if (fitSize.Y + fitSize.H > Height) {
    fitSize.Y = Height - fitSize.H;
  }
  VerbMenu.SetSize(fitSize);
  VerbMenu.Visible = true;
  return true;
}

void Reader::RedrawScreen(real DeltaTime)
{
  DrawBackdrop();
  DrawWindows();
  PrintFPS(DeltaTime);
#ifdef DEVBUILD
  cszt maxlog = 102400;
  if (GLog.size() > maxlog) {
    GLog = CutString(GLog, GLog.size() - maxlog);
  }
  Logger.SetText(GLog);
  Logger.Scroll(10000);
  Logger.Draw();
  if (VarView.Visible) {
    VarView.SetSize(Rect(Width, Height));
    VarView.SetText(VarViewSource);
    Rect fitSize = VarView.GetTextSize();
    VarView.SetSize(fitSize);
    VarView.Draw();
  }
#endif
  Surface::SystemDraw();
}

/** @brief Draw the backdrop image, should be called first
  */
void Reader::DrawBackdrop()
{
  double zoom;
  if (Screen.W > Screen.H) {
    zoom = (Screen.W) / double(Backdrop.W);
  } else {
    zoom = (Screen.H) / double(Backdrop.W);
  }
  if (zoom > 1.01d || zoom < 0.99d) {
    Backdrop.Zoom(zoom, zoom);
  }

  Rect dst(Backdrop.W, Backdrop.H,
           (Screen.W - Backdrop.W) / 2, (Screen.H - Backdrop.H) / 2);
  Screen.Blank();
  Backdrop.Draw(dst);
}

/** @brief Draw All visible windows on the screen
  */
void Reader::DrawWindows()
{
  // main layout
  MainImage.Draw();
  MainText.Draw();
  QuickMenu.Draw();
  ReaderButtons.Draw();
  // pop-ups
  if (!MainMenu.Empty()) {
    MainMenu.Draw();
  }
  GameDialog.Draw();
  VerbMenu.Draw();
}

void Reader::PrintFPS(real DeltaTime)
{
  const real fps = min((real)1 / DeltaTime, (real)999);
  const string& text = RealIntoString(fps);
  Surface textSurface;
  textSurface.CreateText(FontSys, text, 255, 255, 255);
  textSurface.SetAlpha(128);
  textSurface.Draw(10, 10);
}

/** @brief Try to fix the layout that failed to fit by evening out the split
  */
szt Reader::FixLayout()
{
  Layout& layout = Layouts[CurrentOrientation][CurrentLayout];
  // can't fit, abort and try a smaller split
  if (layout.Split - GetGridPercentage() > 0.5) {
    layout.ChangeSplit(-GetGridPercentage());
    return SetLayout();
  } else if (layout.Split + GetGridPercentage() < 0.5) {
    layout.ChangeSplit(GetGridPercentage());
    return SetLayout();
  } else {
    LOG("layout impossible on this screen");
    return CurrentLayout;
    // TODO: try a fall-back
  }
}

szt Reader::SetLayout(szt LayoutIndex)
{
  CurrentOrientation = Width > Height ? landscape : portrait;
  if (LayoutIndex < NUM_LAYOUTS) {
    CurrentLayout = LayoutIndex;
  }
  Layout& layout = Layouts[CurrentOrientation][CurrentLayout];
  WindowBox* boxes[BOX_TYPE_MAX];

  for (szt i = 0; i < BOX_TYPE_MAX; ++i) {
    switch (layout.Order[i]) {
      case boxMain:
        boxes[i] = &MainText;
        break;
      case boxBG:
        boxes[i] = &MainImage;
        break;
      case boxQuick:
        boxes[i] = &QuickMenu;
        break;
      case boxMenu:
        boxes[i] = &ReaderButtons;
        break;

      default:
        boxes[i] = &MainText;
        break;
    }
  }

  lint splitH = 0, splitV = 0;
  Rect boxSize; // we retain certain position data in the loop between boxes
  side lastSide = SIDE_MAX;
  lint layoutW = Width;
  lint layoutH = Height;
  for (szt i = 0; i < BOX_TYPE_MAX; ++i) {
    // check if neighbours are on the same side
    const side pos = layout.Side[i];
    bool first = (pos != lastSide);

    if (layout.Active[i]) {
      boxes[i]->Visible = true;
      lastSide = pos;
    } else {
      // skip invisible boxes
      boxes[i]->Visible = false;
      continue;
    }

    if (pos == left || (first && (pos == top || pos == bottom))) {
      boxSize.X = Width - layoutW;
    } else if (!first && (pos == right)) {
      // retain the last X position
    } else {
      boxSize.X = splitH;
    }

    if (pos == top || (first && (pos == left || pos == right))) {
      boxSize.Y = Height - layoutH;
    } else if (!first && (pos == bottom)) {
      // retain the last Y position
    } else {
      boxSize.Y = splitV;
    }

    if (pos == top || pos == bottom) {
      // horizontal fill
      if (Width >= boxSize.X + GRID * 2) {
        boxSize.W = Width - boxSize.X;
      } else {
        return FixLayout();
      }

      if (layout.SizeSpan == i && pos == top && first) {
        // this element determines the size of the split
        lint splitSize = Height * layout.Split;
        splitSize = splitSize - (splitSize % GRID);
        // make sure the requested split is viable
        if (splitSize < GRID * 2 || Height - splitSize < GRID * 2) {
          return FixLayout();
        }
        boxSize.H = splitSize;
      } else {
        // find how much vertical space is left
        if (first) {
          if (Height > splitV + GRID * 2) {
            boxSize.H = Height - splitV;
          } else {
            return FixLayout();
          }
        } else {
          if (splitV > boxSize.Y + GRID * 2) {
            boxSize.H = splitV - boxSize.Y;
          } else {
            return FixLayout();
          }
        }
      }
    } else {
      // vertical fill
      if (Height >= boxSize.Y + GRID * 2) {
        boxSize.H = Height - boxSize.Y;
      } else {
        return FixLayout();
      }

      if (layout.SizeSpan == i && pos == left && first) {
        // this element determines the size of the split
        lint splitSize = Width * layout.Split;
        splitSize = splitSize - (splitSize % GRID);
        // make sure the requested split is viable
        if (splitSize < GRID * 2 || Width - splitSize < GRID * 2) {
          return FixLayout();
        }
        boxSize.W = splitSize;
      } else {
        // find how much horizontal space is left
        if (first) {
          if (Width > splitH + GRID * 2) {
            boxSize.W = Width - splitH;
          } else {
            return FixLayout();
          }
        } else {
          if (splitH > boxSize.X + GRID * 2) {
            boxSize.W = splitH - boxSize.X;
          } else {
            return FixLayout();
          }
        }
      }
    }

    boxes[i]->SetSize(boxSize);
    // resizing is allowed only vertically
    if (boxSize.W < boxes[i]->Size.W) {
      return FixLayout();
    }

    boxSize = boxes[i]->Size;
    splitH = boxSize.X + boxSize.W;
    splitV = boxSize.Y + boxSize.H;

    if (pos == top) {
      layoutH -= boxes[i]->Size.H + GRID / 2;
    }
  }

#ifdef DEVBUILD
  Logger.SetSize(MainImage.Size);
#endif

  return CurrentLayout;
}

real Reader::GetGridPercentage()
{
  return (real)GRID / (real)Width;
}

Layout& Reader::GetCurrentLayout()
{
  return Layouts[CurrentOrientation][CurrentLayout];
}
