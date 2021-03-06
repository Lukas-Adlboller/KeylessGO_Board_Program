#include "MT25Q.h"
#include "CryptoEngine.h"
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>
#include <functional>

#ifndef ENTRY_MANAGER_H
#define ENTRY_MANAGER_H

#define DEVICE_SETTINGS_START_ADDRESS 0x00    // Start address where device settings are stored
#define ADDRESS_TABLE_START_ADDRESS   0x1000  // Second subsector stores entry address table
#define ENTRY_START_ADDRESS           0x2000  // Entries are stored starting at address of third subsector
#define SALT_START_ADDRESS            0x04    // Salt is stored in device settings page
#define HASHED_PWD_START_ADDRESS      0x14    // Hashed master password (32 bytes) is stored in device settings page
#define MAX_ENTRY_COUNT               0x07FF  // Max count of stored entries is 2047

#define ENTRY_TITLE_SIZE              16      // Entry title size in bytes
#define ENTRY_USERNAME_SIZE           32      // Entry username size in bytes
#define ENTRY_EMAIL_SIZE              64      // Entry email size in bytes
#define ENTRY_PASSWORD_SIZE           32      // Entry password size in bytes
#define ENTRY_URL_SIZE                24      // Entry url size in bytes

class EntryManager
{
  public:
    EntryManager(MT25Q* flashMemory, CryptoEngine* cryptoEngine);
    void saveSettings(void);
    void reloadSettings(void);
    void saveSalt(uint8_t* salt);
    void savePassword(uint8_t* pwd);
    void loadSalt(void);
    void setAsInitialized(void);
    bool addEntry(const char* title, const char* usr, const char* email, const char* pwd, const char* url);
    bool editEntry(uint16_t id, const char* title, const char* usr, const char* email, const char* pwd, const char* url);
    bool getEntry(uint16_t id, uint8_t* title, uint8_t* usr, uint8_t* email, uint8_t* pwd, uint8_t* url);
    bool removeEntry(uint16_t id);
    bool needsToBeInitialized(void);
    bool comparePassword(uint8_t* pwd);
    uint16_t getEntryCount(void);
    uint16_t getUniqueId(void);
    vector<tuple<uint16_t, string>> getEntriesTitleInfo(void);

    static vector<tuple<uint16_t, string>> credentialInfo;
    
  private:
    MT25Q* flashMemory;
    CryptoEngine* cryptoEngine;
    vector<uint16_t> usedIds;
    static uint8_t addressTable[MT25Q_SUBSECTOR_SIZE];
    uint8_t deviceSettings[MT25Q_PAGE_SIZE];
    
    uint8_t getStringLength(const char* str, uint8_t maxLength);
    void setEntryCount(uint16_t entryCount);
};

#endif