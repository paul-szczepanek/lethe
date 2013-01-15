#include "main.h"
#include "reader.h"
#include "input.h"

#ifdef LOGGER
string GLog = "";
#endif

int main (int argc,
          char** argv)
{
  Reader reader(1000, 800, 32);

  if (reader.Init()) {
    ulint lastTime = 0;
    real deltaTime = 0.1;

    while (reader.Tick(deltaTime)) {
      deltaTime = Input::limitFPS(lastTime);
    }
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
