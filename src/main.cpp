#include <SDL.h>
#include <SDL_ttf.h>

#include "main.h"
#include "reader.h"

typedef unsigned int uint;

#ifdef LOGGER
string GLog;
#endif

void limitFPS(Uint32 LastTime)
{
  // new time after game logic has finished
  Uint32 newTime = SDL_GetTicks();
  const Uint32 fpsInterval = 32; //pegs it at around 30fps

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
}

int main (int argc, char** argv)
{
#ifdef LOGGER
  GLog = "";
#endif

  // initialize SDL video
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf( "Unable to init SDL: %s\n", SDL_GetError() );
    return 1;
  }

  if (TTF_Init() < 0) {
    printf( "Unable to init TTF: %s\n", SDL_GetError() );
    return 1;
  }

  // make sure SDL cleans up before exit
  atexit(SDL_Quit);
  atexit(TTF_Quit);

  Reader* reader = new Reader();

  if (reader->Init(1000, 600, 32)) {
    Uint32 lastTime = 0.f;
    real deltaTime = 1.f;

    while (reader->Tick(deltaTime)) {
      Uint32 newTime = SDL_GetTicks();
      deltaTime = real(newTime - lastTime) * 0.001f;
      limitFPS(lastTime);
      lastTime = newTime;
    }
  }

  return 0;
}

