#include "sound.h"

Sound::Sound(MediaManager& Manager,
             const string& AssetName,
             const string& Params)
  : Asset(Manager, AssetName)
{

}

/** @brief Play
  *
  * @todo: document this function
  */
bool Sound::Play()
{
  return false;
}

/** @brief Stop
  *
  * @todo: document this function
  */
bool Sound::Stop()
{
  return false;
}

/** @brief Stop
  *
  * @todo: document this function
  */
bool Sound::Tick(real DeltaTime)
{
  return true;
}
