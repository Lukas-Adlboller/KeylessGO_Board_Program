#include "mbed.h"
#include "Window.h"
#include "EntryManager.h"
#include "BoardProgram.h"
#include "KeylessComm_STM32F746.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

int main()
{
  printf("\n\n----------------------------------\n");
  
  while(true)
  {
    BoardProgram boardProg;
    if(boardProg.initialize() == 0)
    {
      boardProg.run();
    }
  }
  
  /*MT25Q flashMemory(PE_6, PE_5, PE_12, D3);
  CryptoEngine cryptoEngine;

  flashMemory.eraseChip();

  uint8_t masterPwd[MASTER_PASSWORD_LENGTH] = {"12345"};
  uint8_t genSalt[MAX_SALT_LENGTH];

  // Initialize Crypto Engine
  cryptoEngine.setMasterPassword(masterPwd);
  cryptoEngine.generateRandomSalt(genSalt);
  cryptoEngine.setSalt(genSalt);
  cryptoEngine.generateAesKeyAndIV();

  EntryManager entryManager(&flashMemory, &cryptoEngine);
  KeylessCom boardCommunication(9600, &entryManager);

  entryManager.addEntry("Twitter", "Modex", "email@gmail.com", "1234", "www.twitter.com");
  entryManager.addEntry("Twitch", "SimplyLukas", "email@gmail.com", "1234", "www.twitch.com");
  entryManager.addEntry("Steam", "Werti", "email@gmail.com", "1304", "www.steam.com");
  entryManager.saveSettings();

  while(true)
  {
    boardCommunication.process();
  }

  entryManager.removeEntry(0x00);
  entryManager.removeEntry(0x02);

  entryManager.addEntry("YouTube", "XImSyntax", "email@gmail.com", "0000", "www.youtube.com");

  
  uint8_t title[ENTRY_TITLE_SIZE];
  uint8_t usr[ENTRY_USERNAME_SIZE];
  uint8_t url[ENTRY_URL_SIZE];
  uint8_t email[ENTRY_EMAIL_SIZE];
  uint8_t pwd[ENTRY_PASSWORD_SIZE];

  entryManager.getEntry(0x01, title, usr, email, pwd, url);
  

  printf("TITLE: %s\nUSR: %s\nURL: %s\nEMAIL: %s\nPWD: %s\n", title, usr, url, email, pwd);
  */
  //uint8_t data[256];
  //flashMemory.readBytes(DEVICE_SETTINGS_START_ADDRESS, data, MT25Q_PAGE_SIZE);
  //PrintArray(data);
}