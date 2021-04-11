#include "EntryManager.h"

uint8_t EntryManager::addressTable[];
vector<tuple<uint16_t, string>> EntryManager::credentialInfo;

/*
  EntryManager(MT25Q*, CryptoEngine*) initializes class.

  Important: MT25Q and CryptoEngine instance must be initialized before use of this class.
*/
EntryManager::EntryManager(MT25Q* flashMemory, CryptoEngine* cryptoEngine)
{
  this->flashMemory = flashMemory;
  this->cryptoEngine = cryptoEngine;

  reloadSettings();
}

/*
  void reloadSettings(void) reads settings from memory.
*/
void EntryManager::reloadSettings(void)
{
  flashMemory->readBytes(DEVICE_SETTINGS_START_ADDRESS, deviceSettings, MT25Q_PAGE_SIZE);
  flashMemory->readBytes(ADDRESS_TABLE_START_ADDRESS, addressTable, MT25Q_SUBSECTOR_SIZE);

  usedIds.clear();

  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId != 0xFFFF)
    {
      usedIds.push_back(foundId);
    }
  }
  
  if(getEntryCount() == 0xFFFF)
  {
    setEntryCount(0x00);
  }
}

/*
  uint16_t getEntryCount(void) returns current entry count from device settings.
*/
uint16_t EntryManager::getEntryCount(void)
{
  return (deviceSettings[1] << 8) | deviceSettings[2];
}

/*
  void setEntryCount(uint16_t) sets new entry count.
*/
void EntryManager::setEntryCount(uint16_t entryCount)
{
  deviceSettings[1] = (entryCount & 0xFF00) >> 8;
  deviceSettings[2] = entryCount & 0xFF;
}

/*
  uint8_t getStringLength(const char*, uint8_t) returns length of a string (ending with '\0').
  If string is longer than maxLength, value of maxLength will be returned.
*/
uint8_t EntryManager::getStringLength(const char* str, uint8_t maxLength)
{
  int length = 0;
  while((str[length] != '\0' || str[length] != 0xFF) && length < maxLength)
  {
    length++;
  }

  return length;
}

/*
  void saveSettings(void) writes Device Settings and Address Table to memory.
*/
void EntryManager::saveSettings(void)
{
  flashMemory->updateBytes(DEVICE_SETTINGS_START_ADDRESS, deviceSettings);

  for(auto i = 0; i < MT25Q_SUBSECTOR_SIZE / MT25Q_PAGE_SIZE; i++)
  {
    flashMemory->updateBytes(ADDRESS_TABLE_START_ADDRESS + (i << 8), &addressTable[i << 8]);
  }
}

/*
  bool addEntry(const char*, const char*, const char*, const char*, const char*) adds entry to next free slot in
  address table and stores it in the corresponding address on the flash.

  Returns true if entry has been added.
*/
bool EntryManager::addEntry(const char* title, const char* usr, const char* email, const char* pwd, const char* url)
{
  if(getEntryCount() == MAX_ENTRY_COUNT)
  {
    printf("[Error] Already reached limit of max entries!\n");
    return false;
  }

  uint8_t tmpPage[MT25Q_PAGE_SIZE];
  memset(tmpPage, 0xFF, MT25Q_PAGE_SIZE);

  uint32_t entryAddress = ENTRY_START_ADDRESS;
  uint32_t addrTablePage = 0;

  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId == 0xFFFF)
    {
      uint16_t entryId = getUniqueId();
      usedIds.push_back(entryId);
      addressTable[i * 2] = tmpPage[0] = (entryId & 0xFF00) >> 8;
      addressTable[(i * 2) + 1] = tmpPage[1] = entryId & 0xFF;
      entryAddress = ENTRY_START_ADDRESS + MT25Q_PAGE_SIZE * i;
      addrTablePage = i / (MT25Q_PAGE_SIZE / 2);
      setEntryCount(getEntryCount() + 1);
      break;
    }
  }

  /*
    Entries are saved in 256 byte pages in following format:

    [ID 2 bytes] [TITLE 16 bytes] [URL 24 bytes] [Not Defined 86 bytes] (First 128 bytes - unencrypted)
    [USERNAME 32 bytes] [EMAIL 64 bytes] [PASSWORD 32 bytes]            (Last 128 bytes - encrypted with AES)
  */

  // [TITLE 16 bytes]
  copy_n(title, getStringLength(title, ENTRY_TITLE_SIZE), &tmpPage[2]);

  // [URL 24 bytes]
  copy_n(url, getStringLength(url, ENTRY_URL_SIZE), &tmpPage[2 + ENTRY_TITLE_SIZE]);

  // [USERNAME 32 bytes]
  copy_n(usr, getStringLength(usr, ENTRY_USERNAME_SIZE), &tmpPage[128]);

  // [EMAIL 64 bytes]
  copy_n(email, getStringLength(email, ENTRY_EMAIL_SIZE), &tmpPage[128 + ENTRY_USERNAME_SIZE]);

  // [PASSWORD 32 bytes]
  copy_n(pwd, getStringLength(pwd, ENTRY_PASSWORD_SIZE), &tmpPage[128 + ENTRY_USERNAME_SIZE + ENTRY_EMAIL_SIZE]);

  uint8_t encData[128];
  cryptoEngine->cryptWithAesCBC(&tmpPage[128], encData, MBEDTLS_AES_ENCRYPT);

  // Write encrypted Data (128 bytes) back to tmpPage
  copy_n(encData, 128, &tmpPage[128]);

  flashMemory->updateBytes(entryAddress, tmpPage);
  flashMemory->updateBytes(ADDRESS_TABLE_START_ADDRESS + addrTablePage, &addressTable[addrTablePage]);

  return true;
}

/*
  bool getEntry(uint16_t, char*, char*, char*, char*, char*) gets information of entry via its id.

  Returns true if entry was found.
*/
bool EntryManager::getEntry(uint16_t id, uint8_t *title, uint8_t *usr, uint8_t *email, uint8_t *pwd, uint8_t *url)
{
  if(getEntryCount() == 0)
  {
    printf("[Error] No entry found!\n");
    return false;
  }

  bool entryFound = false;
  uint32_t entryAddr = ENTRY_START_ADDRESS;
  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId == id)
    {
      entryAddr = ENTRY_START_ADDRESS + (i << 8);
      entryFound = true;
      break;
    }
  }

  if(!entryFound)
  {
    printf("[Error] Entry with given id does not exist!\n");
    return false;
  }

  uint8_t tmpPage[256];
  flashMemory->readBytes(entryAddr, tmpPage, MT25Q_PAGE_SIZE);

  if(((tmpPage[0] << 8) | tmpPage[1]) != id)
  {
    printf("[Error] Found entry was saved with the wrong id! Id was: %d\n", (tmpPage[0] << 8) | tmpPage[1]);
    return false;
  }

  if(title != NULL)
  {
    // [TITLE 16 bytes]
    copy_n(&tmpPage[2], getStringLength((char*)&tmpPage[2], ENTRY_TITLE_SIZE), title);
  }

  if(url != NULL)
  {
    // [URL 24 bytes]
    copy_n(&tmpPage[2 + ENTRY_TITLE_SIZE], getStringLength((char*)&tmpPage[2 + ENTRY_TITLE_SIZE], ENTRY_URL_SIZE), url);
  }

  uint8_t encData[128];

  // Get encrypted data from entry page
  copy_n(&tmpPage[128], 128, encData);

  // Decrypt data and write it back to tmpPage
  cryptoEngine->cryptWithAesCBC(encData, &tmpPage[128], MBEDTLS_AES_DECRYPT);

  if(usr != NULL)
  {
    // [USERNAME 32 bytes]
    copy_n(&tmpPage[128], getStringLength((char*)&tmpPage[128], ENTRY_USERNAME_SIZE), usr);
  }

  if(email != NULL)
  {
    // [EMAIL 64 bytes]
    copy_n(
      &tmpPage[128 + ENTRY_USERNAME_SIZE], 
      getStringLength((const char*)&tmpPage[128 + ENTRY_USERNAME_SIZE], ENTRY_EMAIL_SIZE), 
      email);
  }

  if(pwd != NULL)
  {
    // [PASSWORD 32 bytes]
    copy_n(
      &tmpPage[128 + ENTRY_USERNAME_SIZE + ENTRY_EMAIL_SIZE], 
      getStringLength((char*)&tmpPage[128 + ENTRY_USERNAME_SIZE + ENTRY_EMAIL_SIZE], 
      ENTRY_PASSWORD_SIZE), pwd);
  }
  return true;
}

/*
  bool editEntry(uint16_t, const char*, const char*, const char*, const char*, const char*) edits entry at given id.
*/
bool EntryManager::editEntry(uint16_t id, const char* title, const char* usr, const char* email, const char* pwd, const char* url)
{
  if(getEntryCount() == 0)
  {
    printf("[Error] No entry found!\n");
    return false;
  }

  uint8_t tmpPage[MT25Q_PAGE_SIZE];
  memset(tmpPage, 0xFF, MT25Q_PAGE_SIZE);

  uint32_t entryAddress = ENTRY_START_ADDRESS;
  bool entryFound = false;
  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId == id)
    {
      entryAddress = ENTRY_START_ADDRESS + MT25Q_PAGE_SIZE * i;
      tmpPage[0] = (id & 0xFF00) >> 8;
      tmpPage[1] = id & 0xFF;
      entryFound = true;
      break;
    }
  }

  if(!entryFound)
  {
    printf("[Error] Entry with given id does not exist!\n");
    return false;
  }

  /*
    Entries are saved in 256 byte pages in following format:

    [ID 2 bytes] [TITLE 16 bytes] [URL 24 bytes] [Not Defined 86 bytes] (First 128 bytes - unencrypted)
    [USERNAME 32 bytes] [EMAIL 64 bytes] [PASSWORD 32 bytes]            (Last 128 bytes - encrypted with AES)
  */

  // [TITLE 16 bytes]
  copy_n(title, getStringLength(title, ENTRY_TITLE_SIZE), &tmpPage[2]);

  // [URL 24 bytes]
  copy_n(url, getStringLength(url, ENTRY_URL_SIZE), &tmpPage[2 + ENTRY_TITLE_SIZE]);

  // [USERNAME 32 bytes]
  copy_n(usr, getStringLength(usr, ENTRY_USERNAME_SIZE), &tmpPage[128]);

  // [EMAIL 64 bytes]
  copy_n(email, getStringLength(email, ENTRY_EMAIL_SIZE), &tmpPage[128 + ENTRY_USERNAME_SIZE]);

  // [PASSWORD 32 bytes]
  copy_n(pwd, getStringLength(pwd, ENTRY_PASSWORD_SIZE), &tmpPage[128 + ENTRY_USERNAME_SIZE + ENTRY_EMAIL_SIZE]);

  uint8_t encData[128];
  cryptoEngine->cryptWithAesCBC(&tmpPage[128], encData, MBEDTLS_AES_ENCRYPT);

  // Write encrypted Data (128 bytes) back to tmpPage
  copy_n(encData, 128, &tmpPage[128]);

  flashMemory->updateBytes(entryAddress, tmpPage);

  return true;
}

/*
  bool removeEntry(uint16_t) removes entry by deleting id in address table.
*/
bool EntryManager::removeEntry(uint16_t id)
{
  if(getEntryCount() == 0)
  {
    printf("[Error] No entry found!\n");
    return false;
  }

  bool idFound = false;
  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId == id)
    {
      addressTable[i * 2] = 0xFF;
      addressTable[(i * 2) + 1] = 0xFF;
      usedIds.erase(find(usedIds.begin(), usedIds.end(), foundId));
      idFound = true;
      break;
    }
  }

  if(!idFound)
  {
    printf("[Error] Entry with given id does not exist!\n");
    return false;
  }

  setEntryCount(getEntryCount() - 1);

  return true;
}

/*
  uint16_t getUniqueId(void) returns an unused id.
*/
uint16_t EntryManager::getUniqueId(void)
{
  uint16_t id = 0;
  while(find(usedIds.begin(), usedIds.end(), id) != usedIds.end())
  {
    id++;
  }

  return id;
}

/*
  vector<string> getEntriesTitleInfo(void) returns all saved entry titles.
*/
vector<tuple<uint16_t, string>> EntryManager::getEntriesTitleInfo(void)
{
  vector<tuple<uint16_t, string>> entriesTitleInfo;
  
  for(auto i = 0; i < MAX_ENTRY_COUNT; i++)
  {
    uint16_t foundId = (addressTable[i * 2] << 8) | addressTable[(i * 2) + 1];

    if(foundId != 0xFFFF)
    {
      uint8_t title[ENTRY_TITLE_SIZE];
      getEntry(foundId, title, NULL, NULL, NULL, NULL);

      entriesTitleInfo.push_back(tuple<uint16_t, string>(foundId, string((char*)title)));
    }
  }

  return entriesTitleInfo;
}

/*
  void saveSalt(uint8_t) writes salt to device settings.
*/
void EntryManager::saveSalt(uint8_t *salt)
{
  // Copy salt to device settings
  copy_n(salt, MAX_SALT_LENGTH, &deviceSettings[SALT_START_ADDRESS]);
}

/*
  void loadSalt(void) loads salt from device settings into crypto engine.
*/
void EntryManager::loadSalt(void)
{
  uint8_t salt[MAX_SALT_LENGTH];
  copy_n(&deviceSettings[SALT_START_ADDRESS], MAX_SALT_LENGTH, salt);
  cryptoEngine->setSalt(salt);
}

/*
  bool needsToBeInitialized(void) if first byte of memory is a specific value (0xFF) then device needs to be initialized first.
*/
bool EntryManager::needsToBeInitialized(void)
{
  return deviceSettings[0] != 0x01 ? true : false;
}

/*
  void savePassword(uint8_t*) hashes password (32 bytes long) and writes saves it on the memory.
*/
void EntryManager::savePassword(uint8_t *pwd)
{
  uint8_t hashedPwd[32];
  cryptoEngine->hashWithSha256(pwd, hashedPwd);

  copy_n(hashedPwd, 32, &deviceSettings[HASHED_PWD_START_ADDRESS]);
}

/*
  bool comparePassword(uint8_t*) compares password with hashed master password from memory.

  Returns:
    true:   When password is the same as the saved master password
    false:  When password is not the same as the saved master password
*/
bool EntryManager::comparePassword(uint8_t *pwd)
{
  uint8_t hashedPwd[32];
  cryptoEngine->hashWithSha256(pwd, hashedPwd);

  for(int i = 0; i < 32; i++)
  {
    if(hashedPwd[i] != deviceSettings[HASHED_PWD_START_ADDRESS + i])
    {
      return false;
    }
  }
  return true;
}

/*
  void setAsInitialized(void) sets first byte of device settings page to 0x01 which means that device
  was initialized.
*/
void EntryManager::setAsInitialized(void)
{
  deviceSettings[0] = 0x01;
}