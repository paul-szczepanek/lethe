#ifndef AUDIO_H
#define AUDIO_H

#include "main.h"

class Mix_Chunk;

class Audio
{
public:
  Audio() { };
  Audio(const string& Filename);
  ~Audio() { };

  static bool SystemInit();

  bool Load(const string& Filename);
  bool Unload();

  bool Play(const real Volume = 1.0);
  bool IsPlaying();
  static void SoundVolume(const real Volume = 1.0);


private:
  Mix_Chunk* SDLAudio = NULL;
  int ChannelUsed = -1;
};

#endif // AUDIO_H
