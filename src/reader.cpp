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
  TTF_CloseFont(FontMain);
  TTF_CloseFont(FontSys);
  SDL_FreeSurface(Backdrop);

  delete MyBook;

#ifdef LOGGER
  if (Logger) {
    delete Logger;
  }
#endif
}

bool Reader::Init(int width,
                  int height,
                  int bpp)
{
  // create a new window
  Screen = SDL_SetVideoMode(width, height, bpp, SDL_HWSURFACE|SDL_DOUBLEBUF);

  if (!Screen) {
    printf("Unable to set video: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetAlpha(Screen, SDL_SRCALPHA, 255);

  // load an image
  Backdrop = IMG_Load("data/default_bg.png");

  if (!Backdrop) {
    printf("Unable to load bitmap: %s\n", SDL_GetError());
    Backdrop = SDL_SetVideoMode(width, height, 32, 0);
  }

  FontSys = TTF_OpenFont("data/font/mono.ttf", 20);
  FontMain = TTF_OpenFont("data/font/mono.ttf", 20);

  if (!(FontSys && FontMain)) {
    TTF_SetError("Loading failed :( (code: %d)", 142);
    std::cout << "Error: " << TTF_GetError() << std::endl;
  }

  MyBook = new Book();
  MyBook->Open("test");

  // read layout
  // TODO: read defaults from file
  SplitH = height;
  SplitV = height - 200;

  // calculate positions of text boxes
  Rect boxSize;
  boxSize.W = width - SplitH;
  boxSize.H = height - SplitV;
  boxSize.X = SplitH;
  boxSize.Y = SplitV;
  ChoiceMenu.SetSize(boxSize);
  boxSize.W = width - SplitH;
  boxSize.H = SplitV;
  boxSize.X = SplitH;
  boxSize.Y = 0;
  SideMenu.SetSize(boxSize);
  boxSize.W = SplitH;
  boxSize.H = height/2;
  boxSize.X = 0;
  boxSize.Y = 0;
  MainText.SetSize(boxSize);

  PageSource = MyBook->Start();
  QuickMenuSource = MyBook->QuickMenu();

  MainText.SetText(PageSource, FontMain);
  MainText.Visible = true;
  SideMenu.SetText(QuickMenuSource, FontMain);
  SideMenu.Visible = true;

#ifdef LOGGER
  GLog = "Log";
  Logger = new TextBox();
  Logger->Visible = true;
  boxSize.Y += height / 2;
  boxSize.H = height / 2;
  Logger->SetSize(boxSize);
  Logger->SetText("", FontMain);
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
          MainText.SetText(PageSource, FontMain);
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
      NounKeyword.clear();
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
        if (VerbMenu.GetSelectedKeyword(VerbKeyword)) {
          VerbMenu.Visible = false;
          ReadBook();
          QuickMenuSource = MyBook->QuickMenu();
          MainText.SetText(PageSource, FontMain);
          SideMenu.SetText(QuickMenuSource, FontMain);
        }
        VerbMenu.Deselect();
      } else if (SideMenu.GetSelectedKeyword(NounKeyword)) {
        SideMenu.Deselect();
      } else if (MainText.GetSelectedKeyword(NounKeyword)) {
        MainText.Deselect();
      }

      if (!NounKeyword.empty()) {
        // we clicked on a keyword, create a menu full of verbs
        string VerbsText = MyBook->GetVerbList(NounKeyword);

        Rect boxSize(400, 400, Mouse.X, Mouse.Y);
        VerbMenu.Visible = true;
        VerbMenu.SetSize(boxSize);
        VerbMenu.SetText(VerbsText, FontMain);
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
  if (NounKeyword.empty() || VerbKeyword.empty()) {
    return false;
  }

  PageSource += MyBook->Read(NounKeyword, VerbKeyword);
  NounKeyword.clear();
  VerbKeyword.clear();

  return true;
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

