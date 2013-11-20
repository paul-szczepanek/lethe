#include "surface.h"
#include "font.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "SDL_ttf.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
const uint32_t MASK_R = 0xFF000000;
const uint32_t MASK_G = 0x00FF0000;
const uint32_t MASK_B = 0x0000FF00;
const uint32_t MASK_A = 0x000000FF;
#else
const uint32_t MASK_R = 0x000000FF;
const uint32_t MASK_G = 0x0000FF00;
const uint32_t MASK_B = 0x00FF0000;
const uint32_t MASK_A = 0xFF000000;
#endif

SDL_Surface* Surface::Screen = NULL;
int Surface::BPP = 32;

/** @brief Initialise the SDL
  */
bool Surface::SystemInit()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    LOG("Unable to init SDL: " + IntoString(SDL_GetError()));
    return false;
  }
  if (IMG_Init(IMG_INIT_PNG) < 0) {
    LOG("Unable to init IMG: " + IntoString(SDL_GetError()));
    return false;
  }
  atexit(SDL_Quit);
  return true;
}

/** @brief Draw the screen surface
  */
bool Surface::SystemDraw()
{
  if (Screen) {
    SDL_Flip(Screen);
    return true;
  }
  return false;
}

Surface::Surface(const string& NewFilename)
{
  LoadImage(NewFilename);
}

Surface::Surface(const lint Width,
                 const lint Height)
{
  Init(Width, Height);
}

Surface::~Surface()
{
  Unload();
}

/** @brief Prepare the screen for drawing, needs to be called first
  */
bool Surface::InitScreen(lint& ScreenWidth,
                         lint& ScreenHeight,
                         const int ScreenBPP)
{
  Unload();

  Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE;
#if defined(__ANDROID__) && ! defined (FAKEANDROID)
  const SDL_VideoInfo* info = SDL_GetVideoInfo();
  ScreenHeight = info->current_h;
  ScreenWidth = info->current_w;
#endif

  SDLSurface = SDL_SetVideoMode(ScreenWidth, ScreenHeight, ScreenBPP, flags);
  if (SDLSurface) {
    W = SDLSurface->w;
    H = SDLSurface->h;
    if (ScreenWidth != W || ScreenHeight != H) {
      // retry if we can't get a screen as big as we wanted
      ScreenWidth = W;
      ScreenHeight = H;
      return InitScreen(ScreenWidth, ScreenHeight, ScreenBPP);
    } else {
      BPP = ScreenBPP;
      Screen = SDLSurface;
    }
  } else {
    LOG("Unable to set video: " + IntoString(SDL_GetError()));;
  }

  return OnInit();
}

/** @brief Load Image, unload old and store filename for future reloads
  */
bool Surface::LoadImage(const string& NewFilename)
{
  if (!NewFilename.empty() && NewFilename != Filename) {
    Filename = NewFilename;
  }
  Unload();
  if (!Filename.empty()) {
    SDLSurface = IMG_Load(Filename.c_str());
    if (!SDLSurface) {
      LOG(Filename + " - image missing");
    }
  }

  return OnInit();
}

/** @brief create an empty surface of the same size as the screen
  */
bool Surface::Init()
{
  return Init(Screen->w, Screen->h);
}

/** @brief create an empty surface of the same size as the screen
  */
bool Surface::Init(const Rect& InitSize)
{
  return Init(InitSize.W, InitSize.H);
}

/** @brief create an empty surface of given size
  */
bool Surface::Init(const lint Width,
                   const lint Height)
{
  Unload();
  SDLSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height,
                                    BPP, MASK_R, MASK_G, MASK_B, MASK_A);
  return OnInit();
}

/** @brief Set global surface alpha
  */
bool Surface::SetAlpha(const usint Alpha)
{
  if (SDLSurface) {
    SDL_SetAlpha(SDLSurface, SDL_SRCALPHA, Alpha);
    return true;
  }
  return false;
}

/** @brief Enlarge or shrink the image
  */
bool Surface::Resize(lint NewW,
                     lint NewH)
{
  if (NewH) {
    return Zoom((real)NewW / (real)W, (real)NewH / (real)H);
  } else {
    return Zoom((real)NewW / (real)W, (real)NewW / (real)W);
  }
}

/** @brief Enlarge or shrink the image
  */
bool Surface::Zoom(const real X,
                   const real Y)
{
  if (SDLSurface) {
    SDL_Surface* temp = zoomSurface(SDLSurface, X, Y, 1);
    SDL_FreeSurface(SDLSurface);
    SDLSurface = temp;
    OnInit();
    return true;
  }
  return false;
}

/** @brief Get values from sdl for external use
  */
bool Surface::OnInit()
{
  if (SDLSurface) {
    W = SDLSurface->w;
    H = SDLSurface->h;
    Clip = { 0, 0, 0, 0 };
    return true;
  }

  W = 0;
  H = 0;
  Clip = { 0, 0, 0, 0 };
  return false;
}

/** @brief Draw onto the passed in surface at given position
 */
bool Surface::Draw(Surface& Destination,
                   const Rect& Position)
{
  SDL_Rect dst = {
    (Sint16)Position.X,
    (Sint16)Position.Y,
    (Uint16)Position.W,
    (Uint16)Position.H
  };
  if (SDLSurface && Destination.SDLSurface) {
    Blit(SDLSurface, Clip, Destination.SDLSurface, &dst);
    return true;
  }
  return false;
}

/** @brief Draw onto the passed in surface
 */
bool Surface::Draw(Surface& Destination)
{
  if (SDLSurface && Destination.SDLSurface) {
    Blit(SDLSurface, Clip, Destination.SDLSurface, NULL);
    return true;
  }
  return false;
}

/** @brief Draw onto the screen at given position
  */
bool Surface::Draw(const Rect& Position)
{
  SDL_Rect dst = {
    (Sint16)Position.X,
    (Sint16)Position.Y,
    (Uint16)Position.W,
    (Uint16)Position.H
  };
  if (SDLSurface && Screen) {
    Blit(SDLSurface, Clip, Screen, &dst);
    // return true only if the image got blitted
    return (dst.w > 0 && dst.h > 0);
  }
  return false;
}

/** @brief Draw onto the screen at X, Y
  */
bool Surface::Draw(const lint X,
                   const lint Y)
{
  SDL_Rect dst = { (Sint16)X, (Sint16)Y, (Uint16)W, (Uint16)H };
  if (SDLSurface && Screen) {
    Blit(SDLSurface, Clip, Screen, &dst);
    // return true only if the image got blitted
    return (dst.w > 0 && dst.h > 0);
  }
  return false;
}

/** @brief Draw onto the screen
*/
bool Surface::Draw()
{
  if (SDLSurface && Screen) {
    Blit(SDLSurface, Clip, Screen, NULL);
    return true;
  }
  return false;
}

/** @brief paint the surface black and transparent but keep the surface
*/
bool Surface::Blank()
{
  if (SDLSurface) {
    SDL_FillRect(SDLSurface, 0, SDL_MapRGBA(SDLSurface->format, 0, 0, 0, 0));
    return true;
  }
  return false;
}

/** @brief Free up the memory
*/
bool Surface::Unload()
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
    SDLSurface = NULL;
    return true;
  }
  return false;
}

/** @brief draw rectangle on the surface
  */
bool Surface::DrawRectangle(const Rect& Rectangle,
                            const usint R,
                            const usint G,
                            const usint B)
{
  if (SDLSurface) {
    SDL_Rect dst = {
      (Sint16)Rectangle.X,
      (Sint16)Rectangle.Y,
      (Uint16)Rectangle.W,
      (Uint16)Rectangle.H
    };
    SDL_FillRect(SDLSurface, &dst, SDL_MapRGB(SDLSurface->format, R, B, G));
    return true;
  }
  return false;
}

/** @brief Print the text on the current surface without replacing it
  */
bool Surface::PrintText(const Rect& Position,
                        const Font& TextFont,
                        const string& Text,
                        const usint R,
                        const usint G,
                        const usint B)
{
  if (SDLSurface) {
    SDL_Color colour = { (Uint8)R, (Uint8)G, (Uint8)B, 0 };
    SDL_Surface* textSurface = TTF_RenderText_Blended(TextFont.SDLFont,
                               Text.c_str(), colour);
    if (textSurface) {
      SDL_Rect dst = {
        (Sint16)Position.X,
        (Sint16)Position.Y,
        (Uint16)0,
        (Uint16)0
      };
      SDL_SetAlpha(textSurface, 0,  SDL_ALPHA_OPAQUE);
      SDL_BlitSurface(textSurface, 0, SDLSurface, &dst);
      SDL_FreeSurface(textSurface);
      return true;
    }
  }
  return false;
}

/** @brief replaces the surface with the printed text
  */
bool Surface::CreateText(const Font& TextFont,
                         const string& Text,
                         const usint R,
                         const usint G,
                         const usint B)
{
  Unload();
  SDL_Color colour = { (Uint8)R, (Uint8)G, (Uint8)B, 0 };
  SDLSurface = TTF_RenderText_Blended(TextFont.SDLFont, Text.c_str(), colour);

  return OnInit();
}

/** @brief Set clip by width and height only, centering the clipped area
  */
void Surface::Trim(const lint ClipW,
                   const lint ClipH)
{
  Clip.W = min(ClipW, W);
  Clip.H = min(ClipH, H);
  Clip.X = (W - Clip.W) / 2;
  Clip.Y = (H - Clip.H) / 2;
}

/** @brief Set the Clipping rectangle
  */
void Surface::SetClip(const Rect& NewClip)
{
  Clip = NewClip;
}

/** @brief Helper function to check for Clip before blitting
  */
void Blit(SDL_Surface* Source,
          Rect& Clip,
          SDL_Surface* Destination,
          SDL_Rect* Dst)
{
  if (Clip.W) {
    SDL_Rect clip = {
      (Sint16)Clip.X,
      (Sint16)Clip.Y,
      (Uint16)Clip.W,
      (Uint16)Clip.H
    };
    SDL_BlitSurface(Source, &clip, Destination, Dst);
  } else {
    SDL_BlitSurface(Source, 0, Destination, Dst);
  }
}
