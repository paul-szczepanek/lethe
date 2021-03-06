#ifndef SOUND_H
#define SOUND_H

#include "asset.h"
#include "audio.h"

class Sound : public Asset
{
public:
  Sound(MediaManager& Manager, const string& AssetName, const string& Params);
  ~Sound() { };

  bool Play();
  bool Stop();
  bool Tick(real DeltaTime);


private:
  real Volume;
  Audio SoundAudio;
  real Time;
  bool Loop = false;
};

#endif // SOUND_H
