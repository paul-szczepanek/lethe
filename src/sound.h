#ifndef SOUND_H
#define SOUND_H

#include "asset.h"

class Sound : public Asset
{
public:
  Sound(MediaManager& Manager, const string& AssetName, const string& Params);
  virtual ~Sound() { };

  virtual bool Play();
  virtual bool Stop();
  virtual bool Tick(real DeltaTime);

private:
};

#endif // SOUND_H
