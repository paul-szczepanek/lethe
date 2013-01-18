#include "reader.h"
#include "audio.h"
#include "input.h"

const string FRAME_SOLID = "solid";
const string FRAME_TEXT = "text";
const string FRAME_MENU = "menu";
const string FRAME_IMAGE = "image";

Reader::~Reader()
{
#ifdef LOGGER
  if (Logger) {
    delete Logger;
  }
#endif
}

bool Reader::Init()
{
  // init all the critical systems and assets
  if (!Surface::SystemInit()) {
    return false;
  }
  if (!Font::SystemInit()) {
    return false;
  }
  //*
  if (!Audio::SystemInit()) {
    return false;
  } //*/
  if (!Screen.InitScreen(Width, Height, BPP)) {
    return false;
  }
  if (!FontSys.Init("mono.ttf", 12)
      || !FontMain.Init("sans.ttf", 20)
      || !FontSmall.Init("serif.ttf", 12)
      || !FontTitle.Init("king.ttf", 24)) {
    return false;
  }

  Screen.SetAlpha(255);

  if (!Backdrop.LoadImage("data/bg.png")) {
    Backdrop.Init(Width, Height);
  }

  // load layouts
  // TODO: read from file
  string layoutStrings[ORIENTATION_MAX][NUM_LAYOUTS] = {
    {
      "0132 3111 1111 0 +000",
      "1032 3311 1111 0 +001"
    },
    {
      "0132 3111 1111 0 -001",
      "0132 3111 1111 0 -002"
    }
  };
  for (size_t i = 0, for_size = ORIENTATION_MAX; i < for_size; ++i) {
    for (size_t j = 0, for_size = NUM_LAYOUTS; j < for_size; ++j) {
      Layouts[i][j].Init(layoutStrings[i][j]);
    }
  }

  QuickMenu.Init(FontMain, FRAME_MENU, BPP);
  MainText.Init(FontMain, FRAME_TEXT, BPP);
  MainImage.Init(FontMain, FRAME_IMAGE, BPP);
  ReaderButtons.Init(FontMain, FRAME_MENU, BPP);
  MainMenu.Init(FontMain, FRAME_SOLID, BPP);
  VerbMenu.Init(FontMain, FRAME_SOLID, BPP);
  MainText.AspectW = 4;
  QuickMenu.AspectW = 4;
  MainImage.AspectW = 4;
  MainImage.AspectH = 2;
  ReaderButtons.AspectW = 100;
  ReaderButtons.AspectH = 2;

  SetLayout();

  // Main Menu window
  Rect mainWindow = { Width - 2 * BLOCK_SIZE, Height - 2 * BLOCK_SIZE,
                      BLOCK_SIZE, BLOCK_SIZE
                    };
  MainMenu.SetSize(mainWindow);
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
    if (MyBook.MenuOpen) {
      RedrawPending |= ReadMenu();
    } else if (MyBook.BookOpen) {
      RedrawPending |= ReadBook();
    } else {
      // both menu and book are closed, quit
      return false;
    }
    // game logic migh have hidden the menu
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
  PageSource += MyBook.ProcessStoryQueue();
  QuickMenuSource = MyBook.GetQuickMenu();
  // clear out old keywords
  MainText.ValidKeywords.Reset();
  MyBook.GetStoryNouns(MainText.ValidKeywords);
  // actually set the received text to be shown
  MainText.SetText(PageSource);
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
  return true;
}

bool Reader::ProcessInput(real DeltaTime)
{
  KeysState keys;
  RedrawPending = Input::Tick(Mouse, keys);

  if (keys.KeyPressed) {
    if (keys.Escape) {
      if (MyBook.MenuOpen) {
        MyBook.MenuOpen  = false;
      } else {
        // this kills the game without closing the book and saving
        MyBook.BookOpen = false;
      }
    } else if (keys.SplitShrink) {
      --(GetCurrentLayout().Split);
      SetLayout();
    } else if (keys.SplitGrow) {
      ++(GetCurrentLayout().Split);
      SetLayout();
    } else if (keys.LayoutToggle) {
      SetLayout((CurrentLayout + 1) % NUM_LAYOUTS);
    } else if (keys.PgDown) {
      MainText.Pane.PaneScroll = 100;
    } else if (keys.PgUp) {
      MainText.Pane.PaneScroll = -100;
    } else if (keys.Menu) {
      if (MyBook.MenuOpen) {
        MyBook.HideMenu();
      } else {
        MyBook.ShowMenu();
      }
    } else if (keys.ImageZoom) {
      //
    } else if (keys.Undo) {
      MyBook.UndoSnapshot();
    } else if (keys.Redo) {
      MyBook.RedoSnapshot();
    } else if (keys.Bookmark) {
      //
    }
  }

  if (Mouse.Left) {
    if (VerbMenu.Visible && !VerbMenu.Select(Mouse, DeltaTime)) {
      VerbMenu.Visible = false;
      KeywordAction.clear();
    } else if (MainMenu.Visible) {
      MainMenu.Select(Mouse, DeltaTime);
    } else if (!QuickMenu.Select(Mouse, DeltaTime)) {
      QuickMenu.Deselect();
      MainText.Select(Mouse, DeltaTime);
    }
  } else if (Mouse.LeftUp) {
    Mouse.LeftUp = false;
    // touch released, see if we cliked on something interactive
    // verb menu -> main menu -> quick menu -> main text
    if (VerbMenu.Visible) {
      if (VerbMenu.GetSelectedKeyword(KeywordAction.Y)) {
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else {
          MyBook.SetStoryAction(KeywordAction);
        }
      }
      // cleanup after action
      VerbMenu.Visible = false;
      KeywordAction.clear();
      VerbMenu.Deselect();
    } else if (MainMenu.Visible) {
      MainMenu.GetSelectedKeyword(KeywordAction.X);
    } else if (QuickMenu.GetSelectedKeyword(KeywordAction.X)) {

    } else if (MainText.GetSelectedKeyword(KeywordAction.X)) {

    }

    MainMenu.Deselect();
    QuickMenu.Deselect();
    MainText.Deselect();

    if (!KeywordAction.X.empty()) {
      if (MyBook.GetChoice(KeywordAction)) {
        if (MainMenu.Visible) {
          MyBook.SetMenuAction(KeywordAction);
        } else {
          MyBook.SetStoryAction(KeywordAction);
        }
        KeywordAction.clear();
      } else {
        // we clicked on a keyword, create a menu full of verbs
        const string& VerbsText = MyBook.GetStoryVerbs(KeywordAction.X);
        if (!VerbsText.empty()) {
          VerbMenu.Visible = true;
          VerbMenu.SetText(VerbsText);
          const size_t_pair& maxSize = VerbMenu.GetMaxSize();
          Rect boxSize(maxSize.X, maxSize.Y, Mouse.X, Mouse.Y);
          VerbMenu.SetSize(boxSize);
        }
      }
    }
  }
  return true;
}

void Reader::RedrawScreen(real DeltaTime)
{
  DrawBackdrop();
  MyBook.DrawImage();
  DrawWindows();
  PrintFPS(DeltaTime);
#ifdef LOGGER
  const size_t maxlog = 1024;
  if (GLog.size() > maxlog) {
    GLog = GLog.substr(GLog.size() - maxlog, maxlog);
  }
  Logger->SetText(GLog);
  Logger->Pane.PaneScroll = 10000;
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
    Backdrop.SetAlpha(32);
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
  // popups
  MainMenu.Draw();
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
    // TODO: try a fallback
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

  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
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
  for (size_t i = 0, for_size = BOX_TYPE_MAX; i < for_size; ++i) {
    // check if neighbours are on the same side
    const side pos = layout.Side[i];
    bool first = (i == 0) || (pos != layout.Side[i-1]);
    //bool last = (i == for_size-1) || (where != layout.Side[i+1]);

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
