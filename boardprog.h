#include <cstdint>
#include "SPIFlash.h"
#include "cryptoEngine.h"

#define BOARD_SETTINGS_ADDRESS    0x00
#define PWD_HASH_START_ADDRESS    0x03
#define RANDOM_SALT_START_ADDRESS 0x23

class BoardProgram
{
  public:
    BoardProgram(void);
    uint8_t initialize(void);
    uint8_t start(void);
    void reset(void);

  private:
    uint8_t* tempDeviceSettings;
    CryptoEngine* cryptoEngine;
    SPIFlash spiFlash;

    uint8_t firstRunSetup(void);
    uint8_t encryptAndWrite(uint8_t* inputPage, uint32_t pageAddress);
    uint8_t readAndDecrypt(uint8_t* outputPage, uint32_t pageAddress);
    uint16_t getEntryCount(void);
    void updateEntryCount(uint16_t entryCount);
    void addUserCredential(uint8_t* title, uint8_t* userName, uint8_t* email, uint8_t* password, uint8_t* url);
};