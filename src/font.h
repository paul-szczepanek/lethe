#ifndef FONT_H
#define FONT_H

#include "main.h"
#include <SDL.h>

typedef struct _TTF_Font TTF_Font;

class Font
{
public:
  static bool SystemInit();

  Font() { };
  virtual ~Font();

  bool Init(const string& Filename, const size_t Size);
  size_t GetHeight();
  size_t GetLineSkip();
  size_t GetWidth(const string& Text);
  size_t_pair GetSize(const string& Text);


public:
  TTF_Font* SDLFont = NULL;
};

#endif // FONT_H
