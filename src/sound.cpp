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

  Filename = Media.AssetDir + CutString(params, 0, volPos);

  if (volPos != string::npos) {
    const string& vol = CutString(params, volPos);
    Volume = IntoReal(vol);
  }
}

/** @brief Load and play if not playing already, otherwise ignore
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

/** @brief Stop sound if playing, otherwise ignore
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

/** @brief Tick
  */
bool Sound::Tick(real DeltaTime)
{
  Time += DeltaTime;
  if (Loop) {
    if (SoundAudio.IsPlaying()) {
      SoundAudio.Play();
    }
    return true;
  } else {
    return SoundAudio.IsPlaying();
  }
}
