#include "main.h"
#include "reader.h"
#include "input.h"

#ifdef __ANDROID__
// android only has the SDL reader and we need SDL_main defined
#include "SDL.h"
#include "SDL_main.h"

// android Java bindings
#include <jni.h>

extern "C" void SDL_Android_Init(JNIEnv* env, jclass cls);

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
  return JNI_VERSION_1_4;
}

extern "C" void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env,
    jclass cls, jobject obj)
{
  SDL_Android_Init(env, cls);

  int status;
  char* argv[2];
  argv[0] = strdup("Lethe");
  argv[1] = NULL;
  status = SDL_main(1, argv);

  exit(status);
}
#endif

#ifdef DEVBUILD
string GLog = "";
string GTrace = "";
szt GTraceIndent = 0;
#endif

int main (int Count, char* Switches[])
{
  bool sound = true;
  int width = 0;
  int height = 0;
  for (int i = 1; i < Count; ++i) {
    const string argument(Switches[i]);
    //screen size
    if (i == 1 && !argument.empty() && isdigit(argument[0])) {
      szt xPos = FindCharacter(argument, 'x');
      if (xPos != string::npos) {
        width = Abs(IntoInt(CutString(argument, 0, xPos)));
        height = Abs(IntoInt(CutString(argument, xPos + 1)));
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
    LOG("Reader initialisation failed.");
    return 1;
  }
}

/* helper functions for debugging
 */
const char* string_pair::c_str()
{
  return (X + Y).c_str();
}

szt string_pair::size()
{
  return X.size() + Y.size();
}
