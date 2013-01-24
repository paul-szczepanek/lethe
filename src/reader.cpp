#include "reader.h"
#include "audio.h"
#include "input.h"

#ifdef LOGGER
#include "disk.h"
#endif

const string FRAME_SOLID = "solid";
const string FRAME_TEXT = "text";
const string FRAME_MENU = "menu";
const string FRAME_IMAGE = "image";

const string SKEY_CURRENT_LAYOUT = "current layout";
const string SKEY_FONT_NAMES = "fonts";
const string SKEY_FONT_SCALE = "font scale";
const string SKEY_LAYOUTS = "layouts";

const real REDRAW_TIMEOUT = 5.0;
const real MIN_TIMEOUT = 0.1;

Reader::Reader(int ReaderWidth, int ReaderHeight, int ReaderBPP, bool Sound)
  : Width(ReaderWidth), Height(ReaderHeight), BPP(ReaderBPP), Silent(!Sound),
    Settings(SETTINGS_FILE)
{
  LoadSettings();
}

Reader::~Reader()
{
  SaveSettings();
#ifdef LOGGER
  if (Logger) {
    delete Logger;
    Disk::Write("log", GLog);
  }
#endif
}

void Reader::LoadSettings()
{
  // load reader layouts
  for (size_t i = 0; i < ORIENTATION_MAX; ++i) {
    // default layouts
    vector<string> layoutStrings;
    layoutStrings.resize(NUM_LAYOUTS, "0132 3111 1111 0 +000");
    // overwrite with stored ones
    const string& key = SKEY_LAYOUTS + IntoString(i);
    Settings.GetValue(key, layoutStrings);
    // initialise the layouts with default or stored layouts strings
    for (size_t j = 0; j < NUM_LAYOUTS; ++j) {
      Layouts[i][j].Init(layoutStrings[j]);
    }
  }
  Settings.GetValue(SKEY_CURRENT_LAYOUT, CurrentLayout);

  // font defaults
  FontNames = { "sans.ttf", "king.ttf", "serif.ttf", "mono.ttf" };
  FontNames.resize(TEXT_STYLE_MAX);
  // replaced defaults with saved ones if present
  Settings.GetValue(SKEY_FONT_NAMES, FontNames);

  Settings.GetValue(SKEY_FONT_SCALE, FontScale);

  Timeout = MIN_TIMEOUT;
}

void Reader::SaveSettings()
{
  // save reader layouts
  for (size_t i = 0; i < ORIENTATION_MAX; ++i) {
    vector<string> layoutStrings;
    for (size_t j = 0; j < NUM_LAYOUTS; ++j) {
      layoutStrings.push_back(Layouts[i][j].GetDefinition());
    }
    const string& key = SKEY_LAYOUTS + IntoString(i);
    Settings.SetValue(key, layoutStrings);
  }
  Settings.SetValue(SKEY_CURRENT_LAYOUT, IntoString(CurrentLayout));

  // fonts
  Settings.SetValue(SKEY_FONT_NAMES, FontNames);
  Settings.SetValue(SKEY_FONT_SCALE, FontScale);
}

bool Reader::InitFonts()
{
  size_t fonstSizes[TEXT_STYLE_MAX] = { 20, 46, 16, 16 };
  Fonts.resize(TEXT_STYLE_MAX);
  const real scale = (real)FontScale / 100.f;
  for (size_t i = 0; i < TEXT_STYLE_MAX; ++i) {
    if (!Fonts[i].Init(FontNames[i], fonstSizes[i] * scale)) {
      return false;
    }
  }
  if (!FontSys.Init("mono.ttf", 12 * scale)) {
    return false;
  }
  return true;
}

bool Reader::Init()
{
  // initialise all the critical systems and assets
  if (!Surface::SystemInit()
      || !Font::SystemInit()
      || !Screen.InitScreen(Width, Height, BPP)
      || (!Silent && !Audio::SystemInit())
      || !InitFonts()) {
    return false;
  }

  if (!Backdrop.LoadImage("data/bg.png")) {
    Backdrop.Init(Width, Height);
  }

  QuickMenu.Init(Fonts, FRAME_MENU, BPP);
  MainText.Init(Fonts, FRAME_TEXT, BPP);
  MainImage.Init(FRAME_IMAGE, BPP);
  ReaderButtons.Init(FRAME_MENU, BPP);
  MainMenu.Init(Fonts, FRAME_SOLID, BPP);
  VerbMenu.Init(Fonts, FRAME_SOLID, BPP);
  GameDialog.Init(Fonts, FRAME_SOLID, BPP);
  MainMenu.Centered = GameDialog.Centered = true;
  MainText.AspectW = 4;
  QuickMenu.AspectW = 4;
  MainImage.AspectW = 4;
  MainImage.AspectH = 2;
  ReaderButtons.AspectW = 100;
  ReaderButtons.AspectH = 2;

  SetLayout();

  // Windows not part of the layout
  Rect mainWindow = { Width - 2 * BLOCK_SIZE, Height - 2 * BLOCK_SIZE,
                      BLOCK_SIZE, BLOCK_SIZE
                    };
  MainMenu.SetSize(mainWindow);
  GameDialog.SetSize(mainWindow);
  MainMenu.Visible = true;

#ifdef LOGGER
  GLog = "Log";
  Logger = new TextBox();
  Logger->Visible = true;
  Logger->Init(FontSys, "", BPP);
  Logger->SetSize(MainImage.Size);
  Logger->SetText("LOG");
#endif

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
      // we've dealt with all the dialogs
      MyBook.DialogOpen = false;
      MyBook.Dialogs.clear();
      DialogIndex = 0;
    } else {
      // show the dialog
      string text;
      GameDialog.Visible = true;
      const Dialog& dialog = MyBook.Dialogs[DialogIndex];
      // prepare the dialog text
      if (dialog.Input) {
        GameDialog.EnableInput(dialog.Noun, dialog.Message);
        text += "\n\n";
      } else {
        text += dialog.Message + '\n';
      }
      for (const string& button : dialog.Buttons) {
        text += "\n<" + button + '[' + dialog.Noun + ':' + button + "]>\n";
      }
      GameDialog.SetText(text);
      // center it on the screen
      GameDialog.Size.H = GameDialog.PageHeight + 2 * BLOCK_SIZE;
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

  // this is how far into the text the valid keywords get checked
  MainText.ValidateKeywords = PageSource.size();
  // fill the text from the book
  PageSource = MyBook.ProcessStoryQueue();
  //PageSource += '\n';
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

  MenuSource = MyBook.ProcessMenuQueue();
  MainMenu.SetText(MenuSource);
  MainMenu.Size.H = MainMenu.PageHeight + 2 * BLOCK_SIZE;
  MainMenu.Size.Y = (Height - MainMenu.Size.H) / 2;
  MainMenu.SetSize(MainMenu.Size);
  return true;
}

bool Reader::ProcessInput(real DeltaTime)
{
  Keys.InputMode = MyBook.DialogOpen;
  RedrawPending = Input::Tick(Mouse, Keys);

  if (Keys.KeyPressed) {
    if (Keys.InputMode) {
      GameDialog.AddCharacter(Keys.Letter);
    } else if (Keys.SplitShrink) {
      --(GetCurrentLayout().Split);
      SetLayout();
    } else if (Keys.SplitGrow) {
      ++(GetCurrentLayout().Split);
      SetLayout();
    } else if (Keys.LayoutToggle) {
      SetLayout((CurrentLayout + 1) % NUM_LAYOUTS);
    } else if (Keys.PgDown) {
      MainText.Scroll(100);
    } else if (Keys.PgUp) {
      MainText.Scroll(-100);
    } else if (Keys.Menu || Keys.Escape) {
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

  if (Mouse.Left) {
    if (VerbMenu.Visible && !VerbMenu.Select(Mouse, DeltaTime)) {
      VerbMenu.Visible = false;
      KeywordAction.clear();
    } else if (GameDialog.Visible) {
      GameDialog.Select(Mouse, DeltaTime);
    } else if (MainMenu.Visible) {
      MainMenu.Select(Mouse, DeltaTime);
    } else if (!QuickMenu.Select(Mouse, DeltaTime)) {
      QuickMenu.Deselect();
      MainText.Select(Mouse, DeltaTime);
    }
  } else if (Mouse.LeftUp) {
    Mouse.LeftUp = false;
    // touch released, see if we clicked on something interactive
    // verb menu -> main menu -> quick menu -> main text
    if (VerbMenu.Visible) {
      if (VerbMenu.GetSelectedKeyword(KeywordAction.Y)) {
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else {
          MyBook.SetStoryAction(KeywordAction);
        }
      }
      // clean up after action
      VerbMenu.Visible = false;
      KeywordAction.clear();
      VerbMenu.Deselect();
    } else if (GameDialog.Visible) {
      if (GameDialog.GetSelectedKeyword(KeywordAction.X)) {
        // close the dialog and get ready to process the next one
        ++DialogIndex;
        GameDialog.Visible = false;
        // if the dialog had input, send that input to the noun
        if (GameDialog.Input) {
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
    } else if (MyBook.ActiveBranch
               && QuickMenu.GetSelectedKeyword(KeywordAction.X)) {

    } else if (MyBook.ActiveBranch
               && MainText.GetSelectedKeyword(KeywordAction.X)) {

    }

    GameDialog.Deselect();
    MainMenu.Deselect();
    QuickMenu.Deselect();
    MainText.Deselect();

    if (!KeywordAction.X.empty()) {
      if (MyBook.GetChoice(KeywordAction)) {
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else if (MyBook.ActiveBranch) {
          MyBook.SetStoryAction(KeywordAction);
        } else {
          // show a pop-up to ask to branch the story
        }
        KeywordAction.clear();
      } else {
        // we clicked on a keyword, create a menu full of verbs
        const string& verbsText = MyBook.GetStoryVerbs(KeywordAction.X);
        ShowVerbMenu(verbsText);
      }
    }
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
  verbSize.H = VerbMenu.PageHeight + VerbMenu.Size.H - VerbMenu.PageSize.H;
  verbSize.W = VerbMenu.PageWidth + VerbMenu.Size.W - VerbMenu.PageSize.W;
  verbSize.H += BLOCK_SIZE;
  verbSize.W += BLOCK_SIZE;
  if (verbSize.X + verbSize.W > Width) {
    verbSize.X = Width - verbSize.W;
  }
  if (verbSize.Y + verbSize.H > Height) {
    verbSize.Y = Height - verbSize.H;
  }
  VerbMenu.SetSize(verbSize);
  VerbMenu.Visible = true;
  return true;
}

void Reader::RedrawScreen(real DeltaTime)
{
  DrawBackdrop();
  MyBook.DrawImage();
  DrawWindows();
  PrintFPS(DeltaTime);
#ifdef LOGGER
  const size_t maxlog = 10240;
  if (GLog.size() > maxlog) {
    GLog = CutString(GLog, GLog.size() - maxlog);
  }
  Logger->SetText(GLog);
  Logger->Scroll(10000);
  Logger->Draw();
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
size_t Reader::FixLayout()
{
  Layout& layout = Layouts[CurrentOrientation][CurrentLayout];
  // can't fit, abort and try a smaller split
  if (layout.Split < 0) {
    ++layout.Split;
    return SetLayout();
  } else if (layout.Split > 0) {
    --layout.Split;
    return SetLayout();
  } else {
    LOG("layout impossible on this screen");
    return CurrentLayout;
    // TODO: try a fall-back
  }
}

size_t Reader::SetLayout(size_t LayoutIndex)
{
  CurrentOrientation = Width > Height? landscape : portrait;
  if (LayoutIndex < NUM_LAYOUTS) {
    CurrentLayout = LayoutIndex;
  }
  Layout& layout = Layouts[CurrentOrientation][CurrentLayout];
  WindowBox* boxes[BOX_TYPE_MAX];

  for (size_t i = 0; i < BOX_TYPE_MAX; ++i) {
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

  size_t splitH = 0, splitV = 0;
  Rect boxSize; // we retain certain position data in the loop between boxes
  for (size_t i = 0; i < BOX_TYPE_MAX; ++i) {
    // check if neighbours are on the same side
    const side pos = layout.Side[i];
    bool first = (i == 0) || (pos != layout.Side[i-1]);

    boxes[i]->Visible = layout.Active[i];

    if (pos == left || (first && (pos == top || pos == bottom))) {
      boxSize.X = 0;
    } else if (!first && (pos == right)) {
      // retain the last X position
    } else {
      boxSize.X = splitH;
    }

    if (pos == top || (first && (pos == left || pos == right))) {
      boxSize.Y = 0;
    } else if (!first && (pos == bottom)) {
      // retain the last Y position
    } else {
      boxSize.Y = splitV;
    }

    if (pos == top || pos == bottom) {
      // horizontal fill
      if (Width >= boxSize.X + BLOCK_SIZE * 2) {
        boxSize.W = Width - boxSize.X;
      } else {
        return FixLayout();
      }

      if (layout.SizeSpan == i && pos == top && first) {
        // this element determines the size of the split
        size_t halfH = Height / 2;
        // make sure the requested split is viable
        if ((size_t)abs(layout.Split) * BLOCK_SIZE + BLOCK_SIZE * 2 >= halfH) {
          return FixLayout();
        }
        boxSize.H = halfH - (halfH % BLOCK_SIZE) + layout.Split * BLOCK_SIZE;
      } else {
        // find how much vertical space is left
        if (first) {
          if (Height > splitV + BLOCK_SIZE * 2) {
            boxSize.H = Height - splitV;
          } else {
            return FixLayout();
          }
        } else {
          if (splitV > boxSize.Y + BLOCK_SIZE * 2) {
            boxSize.H = splitV - boxSize.Y;
          } else {
            return FixLayout();
          }
        }
      }
    } else {
      // vertical fill
      if (Height >= boxSize.Y + BLOCK_SIZE * 2) {
        boxSize.H = Height - boxSize.Y;
      } else {
        return FixLayout();
      }

      if (layout.SizeSpan == i && pos == left && first) {
        // this element determines the size of the split
        size_t halfW = Width / 2;
        // make sure the requested split is viable
        if ((size_t)abs(layout.Split) * BLOCK_SIZE + BLOCK_SIZE * 2 >= halfW) {
          return FixLayout();
        }
        boxSize.W = halfW - (halfW % BLOCK_SIZE) + layout.Split * BLOCK_SIZE;
      } else {
        // find how much horizontal space is left
        if (first) {
          if (Width > splitH + BLOCK_SIZE * 2) {
            boxSize.W = Width - splitH;
          } else {
            return FixLayout();
          }
        } else {
          if (splitH > boxSize.X + BLOCK_SIZE * 2) {
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
  }

  if (MainImage.Visible) {
    MyBook.ShowImage(MainImage.Size);
  } else {
    MyBook.HideImage();
  }

#ifdef LOGGER
  if (Logger) {
    Logger->SetSize(MainImage.Size);
  }
#endif

  return CurrentLayout;
}

Layout& Reader::GetCurrentLayout()
{
  return Layouts[CurrentOrientation][CurrentLayout];
}
