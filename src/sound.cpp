#include "sound.h"
#include "tokens.h"
#include "mediamanager.h"

Sound::Sound(MediaManager& Manager,
             const string& AssetName,
             const string& Params)
  : Asset(Manager, AssetName)
{
  const string& params = CleanWhitespace(Params);
  size_t volPos = FindCharacter(params, ',');

  Filename = Media.AssetDir + cutString(params, 0, volPos);

  if (volPos != string::npos) {
    const string& vol = cutString(params, volPos);
    Volume = intoReal(vol);
  }
}

/** @brief Play
  *
  * @todo: document this function
  */
bool Sound::Play()
{
  if (!Playing) {
    Playing = true;
    Time = 0;
    SoundAudio.Load(Filename);
    SoundAudio.Play();
    return true;
  }
  return false;
}

/** @brief Stop
  *
  * @todo: document this function
  */
bool Sound::Stop()
{
  if (Playing) {
    Playing = false;
    SoundAudio.Unload();
    return true;
  }
  return false;
}

/** @brief Stop
  *
  * @todo: document this function
  */
bool Sound::Tick(real DeltaTime)
{
  Time += DeltaTime;
  if (Loop) {
    if (SoundAudio.IsPlaying()) {
      SoundAudio.Play();
    }
  } else {
    return SoundAudio.IsPlaying();
  }
}
