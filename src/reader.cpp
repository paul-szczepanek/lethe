#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include "main.h"
#include "reader.h"
#include "book.h"
#include "textbox.h"
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
}

bool Reader::Init(int width,
                  int height,
                  int bpp)
{
  // create a new window
  Screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);

  SDL_SetAlpha(Screen, SDL_SRCALPHA, 255);

  // load an image
  Backdrop = SDL_LoadBMP("data/default_bg.bmp");

  if (!Screen) {
    printf("Unable to set video: %s\n", SDL_GetError());
    return false;
  }

  if (!Backdrop) {
    printf("Unable to load bitmap: %s\n", SDL_GetError());
    return false;
  }

  FontSys = TTF_OpenFont("data/font/mono.ttf", 20);
  FontMain = TTF_OpenFont("data/font/mono.ttf", 20);

  if (!(FontSys && FontMain)) {
    TTF_SetError("Loading failed :( (code: %d)", 142);
    std::cout << "Error: " << TTF_GetError() << std::endl;
  }

  // create or lead progress
  Progress.Name = "FirstRead";
  Progress.BookName = "test";

  MyBook = new Book();
  MyBook->Open(Progress.BookName);

  // read layout
  // TODO: read defaults from file
  SplitH = 0.6;
  SplitV = 0.8;

  // calculate positions of text boxes
  Rect boxSize;
  boxSize.W = (uint)(SplitH * (real)width);
  boxSize.H = (uint)(SplitV * (real)height);
  MainText.SetSize(boxSize);
  boxSize.W = (uint)(SplitH * (real)width);
  boxSize.H = (uint)((1.f - SplitV) * (real)height);
  boxSize.Y = (uint)(SplitV * (real)height);
  ChoiceMenu.SetSize(boxSize);
  boxSize.W = (uint)((1.f - SplitH) * (real)width);
  boxSize.H = (uint)height;
  boxSize.X = (uint)(SplitH * (real)width);
  boxSize.Y = 0;
  SideMenu.SetSize(boxSize);

  PageSource = MyBook->Read(Progress, "story", "begin");


  MainText.SetText(PageSource, FontMain);
  MainText.Visible = true;
  ChoiceMenu.SetText("choice", FontMain);
  ChoiceMenu.Visible = true;
  SideMenu.SetText("side", FontMain);
  SideMenu.Visible = true;

#ifdef LOGGER
  GLog = "Log";
  Logger = new TextBox();
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
        }
        if (event.key.keysym.sym == SDLK_PAGEDOWN) {
          MainText.Pane.PaneScroll = 100;
        }
        if (event.key.keysym.sym == SDLK_PAGEUP) {
          MainText.Pane.PaneScroll = -100;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        RedrawPending = true;
        if (event.button.button == SDL_BUTTON_LEFT) {
          Mouse.Left = true;
        }
        if (event.button.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = true;
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = true;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        RedrawPending = true;
        if (event.button.button == SDL_BUTTON_LEFT) {
          Mouse.Left = false;
          Mouse.LeftUp = true;
        }
        if (event.button.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = false;
          Mouse.RightUp = true;
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
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
  if (!Screen) {
    printf("Unable to set video: %s\n", SDL_GetError());
    return false;
  }

  // do a forced update
  RedrawCountdown -= DeltaTime;
  if (RedrawCountdown < 0.f) {
    RedrawPending = true;
  }

  ProcessInput();

  if (Mouse.Left) {
    if (VerbMenu.Visible && VerbMenu.Select(Mouse, DeltaTime)) {
    } else {
      VerbMenu.Visible = false;
      MainText.Select(Mouse, DeltaTime);
    }

    RedrawPending = true;
  } else if (Mouse.LeftUp) {
    // touch released, see if we cliked on something interative
    Mouse.LeftUp = false;

    string keyword;

    if (VerbMenu.Visible) {
      VerbMenu.Deselect();
      if (VerbMenu.GetSelectedKeyword(keyword)) {
        LOG("verb clicked: " + keyword);
      }
    }

    if (MainText.Visible) {
      if (MainText.GetSelectedKeyword(keyword)) {
        // we clicked on a keyword, create a menu full of verbs
        string VerbsText = MyBook->GetVerbList(Progress, keyword);

        Rect boxSize(400, 400, Mouse.X, Mouse.Y);
        VerbMenu.Visible = true;
        VerbMenu.SetSize(boxSize);
        VerbMenu.SetText(VerbsText, FontMain);
      }
      MainText.Deselect();
    }
  }

  if (RedrawPending) {
    RedrawScreen(DeltaTime);
    RedrawPending = false;
    RedrawCountdown = REDRAW_TIMEOUT;
  }

  return !Quit;
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
    zoom = Screen->w / double(Backdrop->w);
  } else {
    zoom = Screen->h / double(Backdrop->w);
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

