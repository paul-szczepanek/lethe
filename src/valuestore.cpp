#include "valuestore.h"
#include "disk.h"

const string MSG = "Don't hand edit (or if you do, keep the spacing)\n\n";

ValueStore::ValueStore(const string& StoreFilename)
{
  Filename = StoreFilename;
  File store;
  store.Read(Filename);
  string buffer;
  store.GetLine(buffer); // MSG
  store.GetLine(buffer);
  // read the values
  // [key]
  // value
  while (store.GetLine(buffer)) {
    if (IsKey(buffer)) {
      const string& key = CutString(buffer, 1, buffer.size() - 1);
      if (store.GetLine(buffer)) {
        string setting = buffer;
        while (store.GetLine(buffer)) {
          if (IsKey(buffer)) {
            LOG("Spacing in " + Filename + " broken");
          }
          setting += "\n";
          setting += buffer;
        }
        StoredValues[key] = setting;
      }
    } else {
      LOG(buffer + " - value store key malformed in " + Filename);
    }
  }
}

ValueStore::~ValueStore()
{
  string text = MSG;
  for (const auto& value : StoredValues) {
    if (!value.second.empty()) {
      text += '[';
      text += value.first;
      text += "]\n";
      text += value.second;
      text += "\n\n";
    }
  }
  Disk::Write(Filename, text);
}
