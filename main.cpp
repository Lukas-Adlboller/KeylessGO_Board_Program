#include "mbed.h"
#include "BoardProgram.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

void PrintArray(uint8_t* data)
{
  printf("Array: \n");
  for(int i = 0; i < 128; i++)
  {
    printf("%X ", data[i]);
  }
  printf("\n");
}

int main()
{
  printf("\n\n----------------------------------\n");
  MT25Q flashMemory(PA_7, PA_6, PA_5, D8);
  CryptoEngine cryptoEngine;

  flashMemory.eraseChip();

  uint8_t masterPwd[MASTER_PASSWORD_LENGTH] = {"UltiPwd1234!"};
  uint8_t genSalt[MAX_SALT_LENGTH];

  // Initialize Crypto Engine
  cryptoEngine.setMasterPassword(masterPwd);
  cryptoEngine.generateRandomSalt(genSalt);
  cryptoEngine.setSalt(genSalt);
  cryptoEngine.generateAesKeyAndIV();

  EntryManager entryManager(&flashMemory, &cryptoEngine);
  entryManager.addEntry("Twitter", "Modex", "email@gmail.com", "1234", "www.twitter.com");
  entryManager.addEntry("Twitch", "SimplyLukas", "email@gmail.com", "1234", "www.twitch.com");
  entryManager.addEntry("Steam", "Werti", "email@gmail.com", "1304", "www.steam.com");

  entryManager.removeEntry(0x00);
  entryManager.removeEntry(0x02);

  entryManager.addEntry("YouTube", "XImSyntax", "email@gmail.com", "0000", "www.youtube.com");

  vector<string> titles = entryManager.getEntriesTitleInfo();

  for(int i = 0; i < titles.size(); i++)
  {
    printf("Entry: %s\n", titles.at(i).c_str());
  }

  uint8_t title[ENTRY_TITLE_SIZE];
  uint8_t usr[ENTRY_USERNAME_SIZE];
  uint8_t url[ENTRY_URL_SIZE];
  uint8_t email[ENTRY_EMAIL_SIZE];
  uint8_t pwd[ENTRY_PASSWORD_SIZE];

  entryManager.getEntry(0x01, title, usr, email, pwd, url);

  printf("TITLE: %s\nUSR: %s\nURL: %s\nEMAIL: %s\nPWD: %s\n", title, usr, url, email, pwd);

  entryManager.saveSettings();

  uint8_t data[256];
  flashMemory.readBytes(DEVICE_SETTINGS_START_ADDRESS, data, MT25Q_PAGE_SIZE);
  PrintArray(data);
}