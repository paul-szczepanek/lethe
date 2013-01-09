#ifndef ASSET_H
#define ASSET_H

#include "main.h"

class MediaManager;

class Asset
{
public:
  Asset(MediaManager& Manager, const string& AssetName)
  : Name(AssetName), Media(Manager) { };
  virtual ~Asset() { };

  virtual bool Play() = 0;
  virtual bool Stop() = 0;

  virtual bool Tick(real DeltaTime) = 0;

public:
  const string Name;
  size_t Priority = 0;
  bool Playing = false;

protected:
  MediaManager& Media;
};

#endif // ASSET_H
