#include "mbed.h"
#include "CryptoEngine.h"
#include "MT25Q.h"
#include "ILI9341.h"
#include "HR2046.h"
#include "EntryManager.h"
#include <cstdint>

class BoardProgram
{
  public:
    BoardProgram(void);
    uint8_t initialize(void);
    uint8_t run(void);
    void reset(void);

  private:
    CryptoEngine* cryptoEngine;
    MT25Q flashMemory;
    EntryManager* entryManager;

    uint8_t runFirstStartupRoutine(void);
    void updateEntryCount(uint16_t entryCount);
};