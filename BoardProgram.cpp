#include "BoardProgram.h"
#include <cstdint>

/*
  BoardProgram(void) initializes class, display spi, touch spi and flash spi interface.
*/
BoardProgram::BoardProgram(void) : flashMemory(D11, D12, D13, D7)
{
  cryptoEngine = new CryptoEngine();
  entryManager = new EntryManager(&flashMemory, cryptoEngine);
}

/*
  void reset(void) resets device. Flash memory will be ereased where settings are stored.
*/
void BoardProgram::reset(void)
{
}

uint8_t BoardProgram::initialize(void)
{
  if(!flashMemory.isAvailable())
  {
    return -1;
  }

  uint8_t masterPwd[MASTER_PASSWORD_LENGTH] = {"1234"};
  uint8_t rngSalt[MAX_SALT_LENGTH];

  cryptoEngine->setMasterPassword(masterPwd);
  cryptoEngine->generateRandomSalt(rngSalt);
  cryptoEngine->setSalt(rngSalt);
  cryptoEngine->generateAesKeyAndIV();

  
  return 0;
}

uint8_t BoardProgram::runFirstStartupRoutine(void)
{
  return 0;
}

uint8_t BoardProgram::run(void)
{
  return 0;
}