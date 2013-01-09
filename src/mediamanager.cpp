#include "mediamanager.h"
#include "tokens.h"
#include "image.h"
#include "sound.h"
#include "book.h"

MediaManager::~MediaManager()
{
  for (auto asset : Assets) {
    delete asset;
  }
}

bool MediaManager::CreateAssets(const vector<string_pair>& AssetDefs,
                                const string& BookTitle)
{
  AssetDir = STORY_DIR + BookTitle + "/";
  for (string_pair assetDef : AssetDefs) {
    const size_t_pair& funcPos = FindToken(assetDef.Y, token::function); // ()
    const string& type = cutString(assetDef.Y, 0, funcPos.X);
    const string& arguments = cutString(assetDef.Y, funcPos.X + 1, funcPos.Y);

    Asset* asset = NULL;

    if (type == "BG" || type == "Image") {
      asset = new Image(*this, assetDef.X, arguments);
    }
    if (type == "Sound") {
      asset = new Sound(*this, assetDef.X, arguments);
    }
    if (type == "Voice") {
      asset = new Sound(*this, assetDef.X, arguments);

    }

    if (asset) {
      auto it = Assets.begin();
      for (; it < Assets.end(); ++it) {
        if ((*it)->Priority > asset->Priority) {
          break;
        }
      }
      Assets.insert(it, asset);
    }
  }

  return true;
}

/** @brief Draw
  *
  * @todo: document this function
  */
bool MediaManager::Tick(real DeltaTime,
                        Book& MyBook)
{
  bool dirty = false;
  ImageWindow.Blank(); //todo only draw if needed, separate sounds and images

  for (Asset* asset : Assets) {
    if (MyBook.GetAssetState(asset->Name)) {
      if (asset->Playing) {
        asset->Tick(DeltaTime);
      } else {
        dirty = asset->Play();
      }
    } else if (asset->Playing) {
      dirty = asset->Stop();
    }
  }

  return dirty;
}

void MediaManager::Draw()
{
  if (Visible) {
    ImageWindow.Draw();
  }
}

/** @brief SetImageWindowSize
  *
  * @todo: document this function
  */
void MediaManager::SetImageWindowSize(Rect Size)
{
  Size.W -= BLOCK_SIZE;
  Size.H -= BLOCK_SIZE;
  Size.X += BLOCK_SIZE / 2;
  Size.Y += BLOCK_SIZE / 2;

  if (ImageWindowSize != Size) {
    ImageWindowSize = Size;
    ImageWindow.Init();
  }
}
