#include "mediamanager.h"
#include "tokens.h"
#include "image.h"
#include "sound.h"

MediaManager::~MediaManager()
{
  for (auto asset : Assets) {
    delete asset.second;
  }
}

bool MediaManager::CreateAssets(const vector<string_pair>& AssetDefs)
{
  for (string_pair assetDef : AssetDefs) {
    const size_t_pair& funcPos = FindToken(assetDef.Y, token::function); // ()
    const string& type = cutString(assetDef.Y, 0, funcPos.X);
    const string& arguments = cutString(assetDef.Y, funcPos.X + 1, funcPos.Y);

    if (type == "BG" || type == "Image") {
      Image* image = new Image(*this, arguments);
      Assets.insert(make_pair(assetDef.X, image));
    }
    if (type == "Sound") {
      Sound* sound = new Sound(*this, arguments);
      Assets.insert(make_pair(assetDef.X, sound));
    }
    if (type == "Voice") {
      Sound* sound = new Sound(*this, arguments);
      Assets.insert(make_pair(assetDef.X, sound));
    }
  }

  return true;
}

/** @brief Draw
  *
  * @todo: document this function
  */
void MediaManager::Draw()
{

}

/** @brief SetImageWindowSize
  *
  * @todo: document this function
  */
void MediaManager::SetImageWindowSize(Rect Size)
{
  if (ImageWindowSize != Size) {
    ImageWindowSize = Size;
  }
}
