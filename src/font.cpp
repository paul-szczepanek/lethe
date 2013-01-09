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
    std::cout << "Unable to init TTF: " << SDL_GetError() << std::endl;
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

  const string path = FONTS_DIR + Filename;

  SDLFont = TTF_OpenFont(path.c_str(), Size);
  if (SDLFont) {
    return true;
  }

  std::cout << "Font: " << path << " failed to load." << std::endl;

  return false;
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
