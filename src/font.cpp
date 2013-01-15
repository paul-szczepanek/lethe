#include "font.h"

#include <SDL.h>
#include <SDL_ttf.h>

/** @brief Init the SDL
  */
bool Font::SystemInit()
{
  if (TTF_Init() < 0) {
    cout << "Unable to init TTF: " << SDL_GetError() << endl;
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

/** @brief Build the font
  */
bool Font::Init(const string& Filename,
                const size_t Size)
{
  if (SDLFont) {
    TTF_CloseFont(SDLFont);
  }

  const string path = FONTS_DIR + SLASH + Filename;

  SDLFont = TTF_OpenFont(path.c_str(), Size);
  if (SDLFont) {
    return true;
  }

  cout << "Font: " << path << " failed to load." << endl;

  return false;
}

/** @brief Get font height
  */
size_t Font::GetHeight()
{
  return TTF_FontHeight(SDLFont);
}

/** @brief Get font line skip
  */
size_t Font::GetLineSkip()
{
  return 0.5 * TTF_FontLineSkip(SDLFont);
}

/** @brief Return the size of the surface the text would need
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

/** @brief Return the size of the surface the text would need
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
