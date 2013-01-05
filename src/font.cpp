#include "font.h"

#include <SDL.h>
#include <SDL_ttf.h>

/** @brief SystemInit
  *
  * @todo: document this function
  */
bool Font::SystemInit()
{
  if (TTF_Init() < 0) {
    printf( "Unable to init TTF: %s\n", SDL_GetError() );
    return false;
  }

  atexit(TTF_Quit);

  return true;
}


Font::~Font()
{
  if (SDLFont) {
    TTF_CloseFont(SDLFont);
  }
}

/** @brief Init
  *
  * @todo: document this function
  */
bool Font::Init(const string& Filename, const size_t Size)
{
  if (SDLFont) {
    TTF_CloseFont(SDLFont);
  }

  SDLFont = TTF_OpenFont(Filename.c_str(), Size);

  return SDLFont;
}

/** @brief GetHeight
  *
  * @todo: document this function
  */
size_t Font::GetHeight()
{
  return TTF_FontHeight(SDLFont);
}

/** @brief GetLineSkip
  *
  * @todo: document this function
  */
size_t Font::GetLineSkip()
{
  return TTF_FontLineSkip(SDLFont);
}

/** @brief GetWidth
  *
  * @todo: document this function
  */
size_t Font::GetWidth(const string& Text)
{
  int width = 0;
  int height = 0;
  if (SDLFont) {
    TTF_SizeText(SDLFont, Text.c_str(), &width ,&height);
  }
  return width;
}

/** @brief GetSize
  *
  * @todo: document this function
  */
size_t_pair Font::GetSize(const string& Text)
{
  int width = 0;
  int height = 0;
  if (SDLFont) {
    TTF_SizeText(SDLFont, Text.c_str(), &width ,&height);
  }
  return size_t_pair(width, height);
}
