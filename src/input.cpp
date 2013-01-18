#include "input.h"
#include <SDL.h>

real Input::limitFPS(ulint& LastTime)
{
  // new time after game logic has finished
  const Uint32 newTime = SDL_GetTicks();
  const real deltaTime = ((real)newTime - (real)LastTime) * 0.001f;
  const Uint32 fpsInterval = 30;
#ifdef PLATFORM_WIN32
  if (newTime - LastTime < fpsInterval) {
    DWORD sleep_time = (fpsInterval - (newTime - LastTime));
    Sleep(sleep_time);
  }
#else
  if (newTime - LastTime < fpsInterval) {
    timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = (fpsInterval - (newTime - LastTime)) * 1000000;
    nanosleep(&sleep_time, &sleep_time);
  }
#endif
  LastTime = newTime;
  return deltaTime;
}

/** @brief Translate SDL input into internal structs
  */
bool Input::Tick(MouseState& Mouse,
                 KeysState& Keys)
{
  bool changed = false;
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        Keys.Escape = true;
        break;

      case SDL_KEYDOWN:
        Keys.KeyPressed = changed = true;
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          Keys.Escape = true;
        } else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
          Keys.SplitShrink = true;
        } else if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
          Keys.SplitGrow = true;
        } else if (event.key.keysym.sym == SDLK_l) {
          Keys.LayoutToggle = true;
        } else if (event.key.keysym.sym == SDLK_b) {
          Keys.Bookmark = true;
        } else if (event.key.keysym.sym == SDLK_m) {
          Keys.Menu = true;
        } else if (event.key.keysym.sym == SDLK_SPACE) {
          Keys.ImageZoom = true;
        } else if (event.key.keysym.sym == SDLK_PAGEDOWN) {
          Keys.PgDown = true;
        } else if (event.key.keysym.sym == SDLK_PAGEUP) {
          Keys.PgUp = true;
        } else if (event.key.keysym.sym == SDLK_u) {
          Keys.Undo = true;
        } else if (event.key.keysym.sym == SDLK_r) {
          Keys.Redo = true;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        changed = true;
        if (event.button.button == SDL_BUTTON_LEFT) {
          Mouse.Left = true;
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = true;
        } else if (event.button.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = true;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        changed = true;
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

  changed |= Mouse.Left;
  return changed;
}
