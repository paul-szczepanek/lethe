#include "audio.h"
#include "SDL.h"
#include "SDL_mixer.h"

const int NUM_CHANNELS = 16;
bool Audio::Active = true;

Audio::Audio(const string& Filename)
{
  Load(Filename);
}

bool Audio::Load(const string& Filename)
{
  if (Active) {
    Unload();
    SDLAudio = Mix_LoadWAV(Filename.c_str());
    if (!SDLAudio) {
      LOG(Filename + " - sound missing");
      return false;
    }
  }
  return true;
}

bool Audio::Play(const real Volume)
{
  if (SDLAudio) {
    Mix_VolumeChunk(SDLAudio, Volume * MIX_MAX_VOLUME);
    Mix_PlayChannel(-1, SDLAudio, 0);

    return true;
  }
  return false;
}

bool Audio::IsPlaying()
{
  return Active && Mix_Playing(ChannelUsed);
}

void Audio::SoundVolume(const real Volume)
{
  if (Active) {
    Mix_Volume(-1, Volume * MIX_MAX_VOLUME);
  }
}

/** @brief Releases the memory
  */
bool Audio::Unload()
{
  if (SDLAudio) {
    Mix_FreeChunk(SDLAudio);
    SDLAudio = NULL;
    return true;
  }
  return false;
}

bool Audio::SystemInit(bool Silent)
{
  if (Silent) {
    // don't initialise the sound and ignore all sound calls
    Active = false;
    return true;
  }
  if(SDL_Init(SDL_INIT_AUDIO) < 0) {
    LOG("Unable to init SDL: " + IntoString(SDL_GetError()));
    return false;
  }
  atexit(SDL_Quit);
  if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
    LOG("Unable to init Mixer: " + IntoString(Mix_GetError()));
    return false;
  }
  atexit(Mix_Quit);
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
    LOG("Unable to Open Audio: " + IntoString(Mix_GetError()));
    return false;
  }
  atexit(Mix_CloseAudio);
  Mix_AllocateChannels(NUM_CHANNELS);
  return true;
}
