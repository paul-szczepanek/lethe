#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include "reader.h"
#include "book.h"
#include "keywordsmap.h"

const real REDRAW_TIMEOUT = 1.0;

Reader::Reader() : Quit(false), RedrawPending(true)
{

}

Reader::~Reader()
{
  if (FontMain) {
    TTF_CloseFont(FontMain);
  }
  if (FontSys) {
    TTF_CloseFont(FontSys);
  }
  if (Backdrop) {
    SDL_FreeSurface(Backdrop);
  }
  if (MyBook) {
    delete MyBook;
  }
#ifdef LOGGER
  if (Logger) {
    delete Logger;
  }
#endif
}

bool Reader::Init(int Width,
                  int Height,
                  int Bpp)
{
  BPP = Bpp;
  // create a new window
  Screen = SDL_SetVideoMode(Width, Height, BPP, SDL_HWSURFACE|SDL_DOUBLEBUF);

  if (!Screen) {
    std::cout << "Unable to set video: " << SDL_GetError() << std::endl;
    return false;
  }

  FontSys = TTF_OpenFont("data/font/mono.ttf", 16);
  FontMain = TTF_OpenFont("data/font/mono.ttf", 20);

  if (!FontSys || !FontMain) {
    std::cout << "Fonts failed to load" << std::endl;
    return false;
  }

  // load an image
  Backdrop = IMG_Load("data/default_bg.png");

  if (!Backdrop) {
    Backdrop = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height, BPP, MASK_R,
                                    MASK_G, MASK_B, MASK_A);
    LOG("default - bg image missing");
  }

  MyBook = new Book();
  MyBook->Open("test");

  ChoiceMenu.Init(FontMain, "default", BPP);
  VerbMenu.Init(FontMain, "default", BPP);
  SideMenu.Init(FontMain, "default", BPP);
  MainText.Init(FontMain, "default", BPP);

  // read layout
  // TODO: read defaults from file
  SplitH = Height;
  SplitV = Height - (Width - Height);

  // calculate positions of text boxes
  Rect boxSize;
  boxSize.W = Width - SplitH;
  boxSize.H = Height - SplitV;
  boxSize.X = SplitH;
  boxSize.Y = SplitV;
  ChoiceMenu.SetSize(boxSize);
  boxSize.W = Width - SplitH;
  boxSize.H = SplitV;
  boxSize.X = SplitH;
  boxSize.Y = 0;
  SideMenu.SetSize(boxSize);
  boxSize.W = SplitH;
  boxSize.H = Height/2;
  boxSize.X = 0;
  boxSize.Y = 0;
  MainText.SetSize(boxSize);

  PageSource = MyBook->Start();
  QuickMenuSource = MyBook->QuickMenu();

  MainText.SetText(PageSource);
  MainText.Visible = true;
  SideMenu.SetText(QuickMenuSource);
  SideMenu.Visible = true;

#ifdef LOGGER
  GLog = "Log";
  Logger = new TextBox();
  Logger->Visible = true;
  Logger->Init(FontSys, "default", BPP);
  boxSize.Y += Height / 2;
  boxSize.H = Height / 2;
  Logger->SetSize(boxSize);
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
    } else if (!SideMenu.Select(Mouse, DeltaTime)) {
      SideMenu.Deselect();
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
      } else if (SideMenu.GetSelectedKeyword(KeywordAction.X)) {

      } else if (MainText.GetSelectedKeyword(KeywordAction.X)) {

      }

      SideMenu.Deselect();
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
            const uint_pair& maxSize = VerbMenu.GetMaxSize();
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
    SideMenu.SetText(QuickMenuSource);

    return true;
  }

  return false;
}

void Reader::RedrawScreen(real DeltaTime)
{
  DrawBackdrop();

  DrawPage();

  PrintFPS(DeltaTime);

#ifdef LOGGER
  const size_t maxlog = 1024;
  if (GLog.size() > maxlog) {
    GLog = GLog.substr(GLog.size() - maxlog, maxlog);
  }
  Logger->SetText(GLog);
  Logger->Draw(NULL); // so we can scroll it first
  Logger->Pane.PaneScroll = 10000;
  Logger->Draw(Screen);
#endif

  SDL_Flip(Screen);
}

/** @brief DrawBackdrop
  *
  * @todo: document this function
  */
void Reader::DrawBackdrop()
{
  double zoom;

  if (Screen->w > Screen->h) {
    zoom = (Screen->w) / double(Backdrop->w);
  } else {
    zoom = (Screen->h) / double(Backdrop->w);
  }

  if (zoom > 1.01d || zoom < 0.99d) {
    SDL_Surface* temp = zoomSurface(Backdrop, zoom, zoom, 1);
    SDL_FreeSurface(Backdrop);
    Backdrop = temp;
    SDL_SetAlpha(Backdrop, SDL_SRCALPHA, 32);
  }

  SDL_Rect dstRect;
  dstRect.x = (Screen->w - Backdrop->w) / 2;
  dstRect.y = (Screen->h - Backdrop->h) / 2;

  SDL_FillRect(Screen, 0, SDL_MapRGB(Screen->format, 0, 0, 0));
  SDL_BlitSurface(Backdrop, 0, Screen, &dstRect);
}

/** @brief DrawPage
  *
  * @todo: document this function
  */
void Reader::DrawPage()
{
  if (MainText.Visible) {
    MainText.Draw(Screen);
  }

  if (ChoiceMenu.Visible) {
    ChoiceMenu.Draw(Screen);
  }

  if (SideMenu.Visible) {
    SideMenu.Draw(Screen);
  }

  if (VerbMenu.Visible) {
    VerbMenu.Draw(Screen);
  }
}

void Reader::PrintFPS(real DeltaTime)
{
  const string& text = realIntoString(1.f / DeltaTime);
  SDL_Color white;
  white.r = white.g = white.b = 255;

  SDL_Surface* textSurface = TTF_RenderText_Solid(FontSys, text.c_str(), white);
  SDL_SetAlpha(textSurface, SDL_SRCALPHA, 128);

  SDL_Rect dstRect;
  dstRect.x = 10;
  dstRect.y = 10;

  SDL_BlitSurface(textSurface, 0, Screen, &dstRect);
  SDL_FreeSurface(textSurface);
}

