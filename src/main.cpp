#include <SDL.h>

#include "main.h"
#include "reader.h"
#include "surface.h"
#include "font.h"

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

  Reader* reader = new Reader(1000, 800, 32);

  if (reader->Init()) {
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

const char* string_pair::c_str()
{
  return (X + ", " + Y).c_str();
}

size_t string_pair::size()
{
  return X.size() + Y.size();
};
