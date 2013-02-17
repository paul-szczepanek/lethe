#include "input.h"
#include "SDL.h"

real Input::LimitFPS(ulint& LastTime)
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
                 KeysState& Keys,
                 SystemState& System)
{
  bool changed;
  SDL_Event event;
  const SDL_MouseButtonEvent& mouse = event.button;
  const SDLKey& key = event.key.keysym.sym;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        Keys.KeyPressed = changed = true;
        Keys.Quit = true;
        break;

      case SDL_KEYDOWN:
        Keys.KeyPressed = changed = true;
        if ((key == SDLK_q || key == SDLK_c)
            && (event.key.keysym.mod & (KMOD_LALT | KMOD_LCTRL))) {
          Keys.Quit = true;
        } else if (Keys.InputMode) {
          // input mode where we just grab the letter and leave
          if (key == SDLK_ESCAPE) {
            Keys.Letter = BACKSPACE_CHAR;
          } else if (key == SDLK_BACKSPACE || key == SDLK_DELETE) {
            Keys.Letter = BACKSPACE_CHAR;
          } else if ((SDLK_a <= key && key <= SDLK_z)
                     || (SDLK_0 <= key && key <= SDLK_9)
                     || SDLK_SPACE == key || SDLK_QUOTE == key
                     || SDLK_UNDERSCORE == key || SDLK_SEMICOLON == key) {
            Keys.Letter = (char)key; // ASCII mapped
          } else {
            // invalid input, cancel the press
            Keys.KeyPressed = changed = false;
          }
        } else {
          // normal, keys as shortcuts mode
          if (key == SDLK_ESCAPE) {
            Keys.Escape = true;
          } else if (key == SDLK_LEFTBRACKET) {
            Keys.SplitShrink = true;
          } else if (key == SDLK_RIGHTBRACKET) {
            Keys.SplitGrow = true;
          } else if (key == SDLK_l) {
            Keys.LayoutToggle = true;
          } else if (key == SDLK_b) {
            Keys.Bookmark = true;
          } else if (key == SDLK_m) {
            Keys.Menu = true;
          } else if (key == SDLK_SPACE) {
            Keys.ImageZoom = true;
          } else if (key == SDLK_PAGEDOWN) {
            Keys.PgDown = true;
          } else if (key == SDLK_PAGEUP) {
            Keys.PgUp = true;
          } else if (key == SDLK_u) {
            Keys.Undo = true;
#ifdef DEVBUILD
          } else if (key == SDLK_BACKQUOTE) {
            Keys.Console = true;
#endif
          } else if (key == SDLK_r) {
            Keys.Redo = true;
          }
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        changed = true;
        if (mouse.button == SDL_BUTTON_LEFT) {
          Mouse.Left = true;
        } else if (mouse.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = true;
        } else if (mouse.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = true;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        changed = true;
        if (mouse.button == SDL_BUTTON_LEFT) {
          Mouse.Left = false;
          Mouse.LeftUp = true;
        } else if (mouse.button == SDL_BUTTON_RIGHT) {
          Mouse.Right = false;
          Mouse.RightUp = true;
        } else if (mouse.button == SDL_BUTTON_MIDDLE) {
          Mouse.Middle = false;
          Mouse.MiddleUp = true;
        }
        break;

      case SDL_VIDEORESIZE:
        changed = true;
        System.W = event.resize.w;
        System.H = event.resize.h;
        System.Resized = true;

      default:
        changed = false;
        break;
    }

    Mouse.X = mouse.x;
    Mouse.Y = mouse.y;
  }

  changed |= Mouse.Left;
  return changed;
}
