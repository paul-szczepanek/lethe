#ifndef SOUND_H
#define SOUND_H

#include "asset.h"

class Sound : public Asset
{
public:
  Sound(MediaManager& Manager, const string& Params);
  virtual ~Sound() { };

  virtual void Play();
  virtual void Stop();

private:
};

#endif // SOUND_H
