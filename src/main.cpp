#include "main.h"
#include "reader.h"
#include "input.h"

#ifdef LOGGER
string GLog = "";
#endif

int main (int Count, char* Switches[])
{
  bool sound = true;

  for (int i = 0; i < Count; ++i) {
    const string swtich(Switches[i]);
    if (swtich == "-s" || swtich == "-silent" || swtich == "-no-sound") {
      sound = false;
    }
  }

  Reader reader(1000, 800, 32, sound);

  if (reader.Init()) {
    ulint lastTime = 0;
    real deltaTime = 0.1;
    while (reader.Tick(deltaTime)) {
      deltaTime = Input::limitFPS(lastTime);
    }
  } else {
    return 1;
  }

  return 0;
}

/* helper functions for debugging
 */
const char* string_pair::c_str()
{
  return (X + Y).c_str();
}

size_t string_pair::size()
{
  return X.size() + Y.size();
}
