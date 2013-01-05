#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include "main.h"
#include "surface.h"

class Asset;

class MediaManager
{
public:
  MediaManager() {};
  virtual ~MediaManager();

  void Draw();
  void SetImageWindowSize(Rect Size);

  bool CreateAssets(const vector<string_pair>& AssetDefs);

private:
  Rect ImageWindowSize;
  map<string, Asset*> Assets;
  Surface ImageWindow;
};

#endif // MEDIAMANAGER_H
