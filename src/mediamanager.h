#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include "main.h"
#include "surface.h"

class Asset;
class Book;

class MediaManager
{
public:
  MediaManager() {};
  virtual ~MediaManager();

  bool Tick(real DeltaTime, Book& MyBook);
  void Draw();
  void SetImageWindowSize(Rect Size);

  bool CreateAssets(const vector<string_pair>& AssetDefs, const string& BookTitle);

public:
  Rect ImageWindowSize;
  Surface ImageWindow;
  bool Visible = false;
  real BGZoom = 1.0;

  string AssetDir;

private:
  vector<Asset*> Assets;

};

#endif // MEDIAMANAGER_H
