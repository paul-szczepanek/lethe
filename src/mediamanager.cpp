#include "mediamanager.h"
#include "tokens.h"
#include "image.h"
#include "sound.h"
#include "book.h"

MediaManager::~MediaManager()
{
  Reset();
}

void MediaManager::Reset()
{
  for (auto asset : Images) {
    delete asset;
  }
  for (auto asset : Sounds) {
    delete asset;
  }
  Images.clear();
  Sounds.clear();
}

bool MediaManager::CreateAssets(const vector<string_pair>& AssetDefs,
                                const string& BookTitle)
{
  Reset();
  AssetDir = STORY_DIR + SLASH + BookTitle + SLASH;
  for (string_pair assetDef : AssetDefs) {
    const size_t_pair& funcPos = FindToken(assetDef.Y, token::function); // ()
    const string& type = CutString(assetDef.Y, 0, funcPos.X);
    const string& arguments = CutString(assetDef.Y, funcPos.X + 1, funcPos.Y);
    if (type == "BG" || type == "Image") {
      Image* asset = new Image(*this, assetDef.X, arguments);
      if (asset) {
        auto it = Images.begin();
        for (; it < Images.end(); ++it) {
          if ((*it)->Priority > asset->Priority) {
            break;
          }
        }
        Images.insert(it, asset);
      }
    }
    if (type == "Sound" || type == "Voice") {
      Sound* asset = new Sound(*this, assetDef.X, arguments);
      Sounds.push_back(asset);
    }
  }
  return true;
}

/** @brief poll the session and start and stop assets as required
  */
bool MediaManager::Tick(real DeltaTime,
                        Book& MyBook)
{
  bool dirty = false;

  // first check if any of the images changed
  for (Image* asset : Images) {
    if (MyBook.GetAssetState(asset->Name)) {
      if (asset->Playing) {
        dirty |= asset->Tick(DeltaTime);
      } else {
        dirty |= asset->Play();
      }
    } else if (asset->Playing) {
      dirty |= asset->Stop();
    }
  }

  if (dirty) {
    // images changed so rebuild the image surface
    ImageWindow.Blank();
    for (Image* asset : Images) {
      if (asset->Playing) {
        asset->Draw();
      }
    }
  }

  for (Sound* asset : Sounds) {
    if (MyBook.GetAssetState(asset->Name)) {
      if (asset->Playing) {
        if (!asset->Tick(DeltaTime)) {
          MyBook.SetAssetState(asset->Name, false);
        }
      } else {
        asset->Play();
      }
    } else if (asset->Playing) {
      asset->Stop();
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

/** @brief Set the size of the big image
  * assets will use this size to determine their own relative sizes
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
