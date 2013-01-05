#ifndef ASSET_H
#define ASSET_H

#include "main.h"

class MediaManager;

class Asset
{
public:
  Asset(MediaManager& Manager) : Media(Manager) { };
  virtual ~Asset() { };

  virtual void Play() = 0;
  virtual void Stop() = 0;

protected:
  bool Playing = false;
  MediaManager& Media;
};

#endif // ASSET_H
