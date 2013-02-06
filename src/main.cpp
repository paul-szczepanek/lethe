#include "main.h"
#include "reader.h"
#include "input.h"

#ifdef DEVBUILD
string GLog = "";
string GTrace = "";
sz GTraceIndent = 0;
#endif

int main (int Count, char* Switches[])
{
  bool sound = true;
  int width = 800;
  int height = 600;
  for (int i = 1; i < Count; ++i) {
    const string argument(Switches[i]);
    //screen size
    if (i == 1 && !argument.empty() && isdigit(argument[0])) {
      sz xPos = FindCharacter(argument, 'x');
      if (xPos != string::npos) {
        width = IntoInt(CutString(argument, 0, xPos));
        height = IntoInt(CutString(argument, xPos + 1));
      }
    }
    if (argument == "-s" || argument == "-silent" || argument == "-no-sound") {
      sound = false;
    }
  }

  Reader reader(width, height, 32, sound);

  if (reader.Init()) {
    ulint lastTime = 0;
    real deltaTime = 0.1;
    while (reader.Tick(deltaTime)) {
      deltaTime = Input::LimitFPS(lastTime);
    }
    return 0;
  } else {
    LOG("Reader init failed.");
    return 1;
  }
}

/* helper functions for debugging
 */
const char* string_pair::c_str()
{
  return (X + Y).c_str();
}

sz string_pair::size()
{
  return X.size() + Y.size();
}
