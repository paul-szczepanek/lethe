#ifndef FONT_H
#define FONT_H

#include "main.h"
#include <SDL.h>

typedef struct _TTF_Font TTF_Font;

class Font
{
public:
  Font() { };
  ~Font();

  static bool SystemInit();

  bool Init(const string& Filename, const size_t Size);
  lint GetHeight() const;
  lint GetLineSkip() const;
  lint GetWidth(const string& Text) const;
  size_t_pair GetSize(const string& Text) const;


public:
  TTF_Font* SDLFont = NULL;
};

#endif // FONT_H
