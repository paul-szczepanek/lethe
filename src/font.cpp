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
                csz Size)
{
  const string& path = FONTS_DIR + SLASH + Filename;
  TTF_Font* newFont = TTF_OpenFont(path.c_str(), Size);
  if (newFont) {
    if (SDLFont) {
      TTF_CloseFont(SDLFont);
    }
    SDLFont = newFont;
    return true;
  }
  LOG("Font " + path + " failed to load.");
  return false;
}

/** @brief Get font height
  */
lint Font::GetHeight() const
{
  return TTF_FontHeight(SDLFont);
}

/** @brief Get font line skip
  */
lint Font::GetLineSkip() const
{
  return 0.2 * TTF_FontLineSkip(SDLFont);
}

/** @brief Return the size of the surface the text would need
  */
lint Font::GetWidth(const string& Text) const
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
sz_pair Font::GetSize(const string& Text) const
{
  int width = 0;
  int height = 0;
  if (SDLFont) {
    TTF_SizeText(SDLFont, Text.c_str(), &width ,&height);
  }
  return sz_pair(width, height);
}
