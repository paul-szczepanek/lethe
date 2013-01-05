#include "reader.h"
#include "book.h"
#include <SDL.h>

Reader::Reader(int ReaderWidth,
               int ReaderHeight,
               int ReaderBPP)
{
  BPP = ReaderBPP;
  Width = ReaderWidth;
  Height = ReaderHeight;
}

Reader::~Reader()
{
  if (MyBook) {
    delete MyBook;
  }
#ifdef LOGGER
  if (Logger) {
    delete Logger;
  }
#endif
}

bool Reader::Init()
{
  if (!Screen.InitScreen(Width, Height, BPP)) {
    return false;
  }

  if (!FontSys.Init("data/font/mono.ttf", 16)
      || !FontMain.Init("data/font/mono.ttf", 20)) {
    std::cout << "Fonts failed to load" << std::endl;
    return false;
  }

  if (!Backdrop.LoadImage("data/default_bg.png")) {
    Backdrop.Init(Width, Height);
    LOG("default - bg image missing");
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

  QuickMenu.Init(FontMain, "default", BPP);
  MainText.Init(FontMain, "default", BPP);
  MainImage.Init(FontMain, "default", BPP);
  MainMenu.Init(FontMain, "default", BPP);

  MainMenu.AspectW = 100;
  MainMenu.AspectH = 2;
  MainText.AspectW = 4;
  QuickMenu.AspectW = 4;
  MainImage.AspectW = 4;
  MainImage.AspectH = 2;

  SetLayout();

  MainText.Visible = true;
  QuickMenu.Visible = true;
  MainMenu.Visible = true;
  MainImage.Visible = true;

  ChoiceMenu.Init(FontMain, "default", BPP);
  VerbMenu.Init(FontMain, "default", BPP);

  // start reading the book TODO: move this to a menu

  MyBook = new Book();
  MyBook->Open("test");

  Media.CreateAssets(MyBook->GetAssetDefinitions());

  PageSource = MyBook->Start();
  QuickMenuSource = MyBook->QuickMenu();

  MainText.SetText(PageSource);
  QuickMenu.SetText(QuickMenuSource);

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

void Reader::ProcessInput()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        Quit = true;
        break;

      case SDL_KEYDOWN:
        RedrawPending = true;
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          Quit = true;
        } else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
          --(GetCurrentLayout().Split);
          SetLayout();
        } else if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
          ++(GetCurrentLayout().Split);
          SetLayout();
        } else if (event.key.keysym.sym == SDLK_l) {
          SetLayout((CurrentLayout + 1) % NUM_LAYOUTS);
        } else if (event.key.keysym.sym == SDLK_PAGEDOWN) {
          MainText.Pane.PaneScroll = 100;
        } else if (event.key.keysym.sym == SDLK_PAGEUP) {
          MainText.Pane.PaneScroll = -100;
        } else if (event.key.keysym.sym == SDLK_r) {
          // temp for debug help
          delete MyBook;
          MyBook = new Book();
          MyBook->Open("test");
          PageSource = MyBook->Start();
          MainText.SetText(PageSource);
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        RedrawPending = true;
        if (event.button.button == SDL_BUTTON_LEFT) {
          Mouse.Left = true;
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = true;
        } else if (event.button.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = true;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        RedrawPending = true;
        if (event.button.button == SDL_BUTTON_LEFT) {
          Mouse.Left = false;
          Mouse.LeftUp = true;
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = false;
          Mouse.RightUp = true;
        } else if (event.button.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = false;
          Mouse.MiddleUp = true;
        }
        break;
    }

    Mouse.X = event.button.x;
    Mouse.Y = event.button.y;
  }
}

bool Reader::Tick(real DeltaTime)
{
  // do a forced update
  RedrawCountdown -= DeltaTime;
  if (RedrawCountdown < 0.f) {
    RedrawPending = true;
  }

  ProcessInput();

  if (Mouse.Left) {
    if (ChoiceMenu.Visible) {
      ChoiceMenu.Select(Mouse, DeltaTime);
    } else if (VerbMenu.Visible && !VerbMenu.Select(Mouse, DeltaTime)) {
      VerbMenu.Visible = false;
      KeywordAction.clear();
    } else if (!QuickMenu.Select(Mouse, DeltaTime)) {
      QuickMenu.Deselect();
      MainText.Select(Mouse, DeltaTime);
    }
    RedrawPending = true;
  } else if (Mouse.LeftUp) {
    Mouse.LeftUp = false;
    // touch released, see if we cliked on something interactive
    if (ChoiceMenu.Visible) {
      // todo: make choice
    } else {
      if (VerbMenu.Visible) {
        if (VerbMenu.GetSelectedKeyword(KeywordAction.Y)) {
          ReadBook();
        }
        VerbMenu.Visible = false;
        KeywordAction.clear();
        VerbMenu.Deselect();
      } else if (QuickMenu.GetSelectedKeyword(KeywordAction.X)) {

      } else if (MainText.GetSelectedKeyword(KeywordAction.X)) {

      }

      QuickMenu.Deselect();
      MainText.Deselect();

      if (!KeywordAction.X.empty()) {
        if (MyBook->GetChoice(KeywordAction)) {
          ReadBook();
        } else {
          // we clicked on a keyword, create a menu full of verbs
          string VerbsText = MyBook->GetVerbList(KeywordAction.X);
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
  }

  if (RedrawPending) {
    RedrawScreen(DeltaTime);
    RedrawPending = false;
    RedrawCountdown = REDRAW_TIMEOUT;
  }

  return !Quit;
}
/** @brief ReadBook
  *
  * @todo: document this function
  */
bool Reader::ReadBook()
{
  // clear out old keywords
  // PageSource;
  if (KeywordAction.full()) {
    PageSource += MyBook->Read(KeywordAction);
    KeywordAction.clear();

    QuickMenuSource = MyBook->QuickMenu();
    MainText.SetText(PageSource);
    QuickMenu.SetText(QuickMenuSource);

    return true;
  }

  return false;
}

void Reader::RedrawScreen(real DeltaTime)
{
  DrawBackdrop();

  DrawWindows();

  PrintFPS(DeltaTime);

#ifdef LOGGER
  const size_t maxlog = 1024;
  if (GLog.size() > maxlog) {
    GLog = GLog.substr(GLog.size() - maxlog, maxlog);
  }
  Logger->SetText(GLog);
  Logger->Draw(); // so we can scroll it first
  Logger->Pane.PaneScroll = 10000;
  Logger->Draw();
#endif

  Surface::SystemDraw();
}

/** @brief DrawBackdrop
  *
  * @todo: document this function
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
  MainText.Draw();

  ChoiceMenu.Draw();

  QuickMenu.Draw();

  VerbMenu.Draw();

  MainMenu.Draw();

  MainImage.Draw();
}

void Reader::PrintFPS(real DeltaTime)
{
  const string& text = realIntoString(1.f / DeltaTime);
  Surface textSurface;
  textSurface.CreateText(FontSys, text, 255, 255, 255);
  textSurface.SetAlpha(128);
  textSurface.Draw(10, 10);
}

// Layout

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
        boxes[i] = &MainMenu;
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
