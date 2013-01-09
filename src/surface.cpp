#include "surface.h"
#include "font.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <SDL_ttf.h>

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
size_t Surface::ScreenW = 0;
size_t Surface::ScreenH = 0;

/** @brief SystemInit
  *
  * @todo: document this function
  */
bool Surface::SystemInit()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "Unable to init SDL: " << SDL_GetError() << std::endl;
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) < 0) {
    std::cout << "Unable to init IMG: " << SDL_GetError() << std::endl;
    return false;
  }

  atexit(SDL_Quit);

  return true;
}

/** @brief SystemDraw
  *
  * @todo: document this function
  */
bool Surface::SystemDraw()
{
  if (Screen) {
    SDL_Flip(Screen);
    return true;
  }
  return false;
}

/** @brief Surface
  *
  * @todo: document this function
  */
Surface::Surface(const string& Filename)
{
  LoadImage(Filename);
}

/** @brief Surface
  *
  * @todo: document this function
  */
Surface::Surface(size_t Width, size_t Height)
{
  Init(Width, Height);
}

Surface::~Surface()
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
  }
}

/** @brief Screen
  *
  * @todo: document this function
  */
bool Surface::InitScreen(size_t ScreenWidth,
                         size_t ScreenHeight,
                         int ScreenBPP)
{
  ScreenW = W = ScreenWidth;
  ScreenH = H = ScreenHeight;

  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
  }

  BPP = ScreenBPP;
  SDLSurface = SDL_SetVideoMode(ScreenWidth, ScreenHeight, BPP,
                                SDL_HWSURFACE|SDL_DOUBLEBUF);

  Screen = SDLSurface;

  if (!SDLSurface) {
    std::cout << "Unable to set video: " << SDL_GetError() << std::endl;
  }

  return OnInit();
}

/** @brief LoadImage
  *
  * @todo: document this function
  */
bool Surface::LoadImage(const string& Filename)
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
  }

  SDLSurface = IMG_Load(Filename.c_str());
  if (!SDLSurface) {
    LOG(Filename + " - image missing");
  }

  return OnInit();
}

/** @brief Init
  *
  * @todo: document this function
  */
bool Surface::Init()
{
  return Init(ScreenW, ScreenH);
}

/** @brief Init
  *
  * @todo: document this function
  */
bool Surface::Init(size_t Width,
                   size_t Height)
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
  }

  SDLSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, Width, Height,
                                    BPP, MASK_R, MASK_G, MASK_B, MASK_A);

  return OnInit();
}

/** @brief SetAlpha
  *
  * @todo: document this function
  */
bool Surface::SetAlpha(size_t Alpha)
{
  if (SDLSurface) {
    SDL_SetAlpha(SDLSurface, SDL_SRCALPHA, Alpha);
    return true;
  }
  return false;
}

/** @brief Zoom
  *
  * @todo: document this function
  */
bool Surface::Zoom(real X,
                   real Y)
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


/** @brief DrawOnto
 *
 * @todo: document this function
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

/** @brief DrawOnto
 *
 * @todo: document this function
 */
bool Surface::Draw(Surface& Destination)
{
  if (SDLSurface && Destination.SDLSurface) {
    Blit(SDLSurface, Clip, Destination.SDLSurface, NULL);
    return true;
  }
  return false;
}

/** @brief Draw
  *
  * @todo: document this function
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

/** @brief Draw
  *
  * @todo: document this function
  */
bool Surface::Draw(const size_t X, const size_t Y)
{
  SDL_Rect dst = { (Sint16)X, (Sint16)Y, (Uint16)W, (Uint16)H };
  if (SDLSurface && Screen) {
    Blit(SDLSurface, Clip, Screen, &dst);
    // return true only if the image got blitted
    return (dst.w > 0 && dst.h > 0);
  }
  return false;
}


/** @brief Draw
*
* @todo: document this function
*/
bool Surface::Draw()
{
  if (SDLSurface && Screen) {
    Blit(SDLSurface, Clip, Screen, NULL);
    return true;
  }
  return false;
}

/** @brief Draw
*
* @todo: document this function
*/
bool Surface::Blank()
{
  if (SDLSurface) {
    SDL_FillRect(SDLSurface, 0, SDL_MapRGBA(SDLSurface->format, 0, 0, 0, 0));
    return true;
  }
  return false;
}
/** @brief Draw
*
* @todo: document this function
*/
bool Surface::Unload()
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
    return true;
  }
  return false;
}

/** @brief DrawRectangle
  *
  * @todo: document this function
  */
bool Surface::DrawRectangle(const Rect& Rectangle,
                            usint R,
                            usint G,
                            usint B)
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

/** @brief PrintText
  *
  * @todo: document this function
  */
bool Surface::PrintText(const Rect& Position,
                        const Font& TextFont,
                        const string& Text,
                        usint R,
                        usint G,
                        usint B)
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

/** @brief PrintText
  *
  * @todo: document this function
  */
bool Surface::CreateText(const Font& TextFont,
                         const string& Text,
                         usint R,
                         usint G,
                         usint B)
{
  if (SDLSurface) {
    SDL_FreeSurface(SDLSurface);
  }

  SDL_Color colour = { (Uint8)R, (Uint8)G, (Uint8)B, 0 };

  SDLSurface = TTF_RenderText_Blended(TextFont.SDLFont, Text.c_str(), colour);

  return OnInit();
}

/** @brief Set clip by width and height only, centering the clipped area
  */
void Surface::Trim(size_t ClipW,
                   size_t ClipH)
{
  ClipW = min(ClipW, W);
  ClipH = min(ClipH, H);
  Clip.W = ClipW;
  Clip.H = ClipH;
  Clip.X = (W - ClipW) / 2;
  Clip.Y = (H - ClipH) / 2;
}

/** @brief SetClip
  *
  * @todo: document this function
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
