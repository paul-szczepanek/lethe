#include "audio.h"
#include <SDL.h>
#include <SDL_mixer.h>

int Audio::PlayingChannels = 0;

const int NUM_CHANNELS = 16;

Audio::Audio()
{
  //ctor
}

Audio::Audio(const string& Filename)
{
  Load(Filename);
}

bool Audio::Load(const string& Filename)
{
  Unload();
  SDLAudio = Mix_LoadWAV(Filename.c_str());
  if (!SDLAudio) {
    LOG(Filename + " - sound missing");
    return false;
  }
  return true;
}

bool Audio::Play(const real Volume)
{
  if (SDLAudio) {
    ++PlayingChannels;

    Mix_VolumeChunk(SDLAudio, Volume * MIX_MAX_VOLUME);
    Mix_PlayChannel(-1, SDLAudio, 0);

    return true;
  }
  return false;
}

bool Audio::IsPlaying()
{
  return Mix_Playing(ChannelUsed);
}

void Audio::SoundVolume(const real Volume)
{
  Mix_Volume(-1, Volume * MIX_MAX_VOLUME);
}

bool Audio::Unload()
{
  if (SDLAudio) {
    Mix_FreeChunk(SDLAudio);
    SDLAudio = NULL;
    return true;
  }
  return false;
}

bool Audio::SystemInit()
{
  if(SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cout << "Unable to init SDL: " << SDL_GetError() << std::endl;
    return false;
  }

  atexit(SDL_Quit);

  if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
    std::cout << "Unable to init Mixer: " << Mix_GetError() << std::endl;
    return false;
  }

  atexit(Mix_Quit);

  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
    std::cout << "Unable to Open Audio: " << Mix_GetError() << std::endl;
    return false;
  }

  atexit(Mix_CloseAudio);

  Mix_AllocateChannels(NUM_CHANNELS);

  return true;
}

Audio::~Audio()
{
  //dtor
}
