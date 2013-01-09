#ifndef AUDIO_H
#define AUDIO_H

#include "main.h"

class Mix_Chunk;

class Audio
{
public:
  static bool SystemInit();

  Audio();
  Audio(const string& Filename);
  virtual ~Audio();

  bool Load(const string& Filename);
  bool Unload();

  bool Play(const real Volume = 1.0);
  bool IsPlaying();
  static void SoundVolume(const real Volume = 1.0);

private:
  Mix_Chunk* SDLAudio = NULL;
  int ChannelUsed = -1;
  static int PlayingChannels;
};

#endif // AUDIO_H
