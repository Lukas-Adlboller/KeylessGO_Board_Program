#include "boardprog.h"
#include <cstdint>
#include <cstdio>

BoardProgram::BoardProgram(void) : spiFlash(A6, A5, A4, A3)
{
  tempDeviceSettings = (uint8_t*)malloc(sizeof(uint8_t) * MT25Q_PAGE_SIZE);
}

/*
  uint8_t initialize(void) checks and initializes all modules and performs firstRunSetup routine if necessary.

  Error Return Values:
    (1) -> Not enough memory capacity (RAM) to operate
    (2) -> SPI Flash is not available
*/
uint8_t BoardProgram::initialize(void)
{
  if(tempDeviceSettings == NULL)
  {
    printf("[Error] Not enough memory (RAM) to operate!\n");
    return 1;
  }

  if(spiFlash.isAvailable() != 0)
  {
    printf("[Error] SPI Flash is not available!\n");
    return 2;
  }

  reset();  // For testing purposes
  spiFlash.readBytes(0x00, tempDeviceSettings, MT25Q_PAGE_SIZE);  // Read 1st Page of flash (contains device settings)

  uint8_t masterPassword[32] = {""};
  cryptoEngine = new CryptoEngine(masterPassword);

  uint8_t returnValue = 0;
  if(tempDeviceSettings[0x00] != 0x01)  // 1st byte of device settings tells if firstRunSetup routine was already executed.
  {
    printf("[Info] Starting firstRunSetup routine...\n");
    returnValue = firstRunSetup();
    printf("[Info] Finished firstRunSetup routine!\n");

    if(returnValue != 0)
    {
      return 3;
    }
  }
  else
  {
    uint8_t generatedSalt[16];
    for(auto i = MAX_SALT_LENGTH; i < 16 + MAX_SALT_LENGTH; i++)
    {
      generatedSalt[i - MAX_SALT_LENGTH] = tempDeviceSettings[i];
    }

    cryptoEngine->setSalt(generatedSalt);
  }
  
  cryptoEngine->generateAesKeyAndIV();

  return 0;
}

/*
  uint8_t start(void)
*/
uint8_t BoardProgram::start(void)
{
  uint8_t title[16] = {"This is a title"};
  uint8_t userName[32] = {"This is a username"};
  uint8_t email[64] = {"omgThisSucks@bruh.ligma"};
  uint8_t password[32] = {"Benni_ist_der_echte_Modex"};
  uint8_t url[24] = {"lolomaten.com"};

  addUserCredential(title, userName, email, password, url);

  uint8_t buffer[256];
  readAndDecrypt(buffer, 0x1000);

  return 0;
}

/*
  uint8_t firstRunSetup(void) initializes flash memory for use. And creates master password for secure access.
*/
uint8_t BoardProgram::firstRunSetup(void)
{
  reset();
  updateEntryCount(0x00);

  uint8_t hashOutput[32];
  uint8_t plainPassword[32] = {""};
  cryptoEngine->hashWithSha256(plainPassword, hashOutput);

  for(auto i = PWD_HASH_START_ADDRESS; i < 32 + PWD_HASH_START_ADDRESS; i++)
  {
    tempDeviceSettings[i] = hashOutput[i - PWD_HASH_START_ADDRESS];
  }

  uint8_t saltOutput[16];
  cryptoEngine->generateRandomSalt(saltOutput);

  for(auto i = RANDOM_SALT_START_ADDRESS; i < 16 + RANDOM_SALT_START_ADDRESS; i++)
  {
    tempDeviceSettings[i] = saltOutput[i - RANDOM_SALT_START_ADDRESS];
  }

  tempDeviceSettings[0x00] = 0x01; // Tell the device that firstRunSetup routine was executed.

  spiFlash.updatePage(BOARD_SETTINGS_ADDRESS, tempDeviceSettings);

  return 0;
}

/*
  void reset(void) resets device (SPI Flash)
*/
void BoardProgram::reset(void)
{
  spiFlash.eraseChip();
  spiFlash.readBytes(0x00, tempDeviceSettings, MT25Q_PAGE_SIZE);
}

/*
  void updateEntryCount(uint16_t) writes (updates) amount of saved entries to the temporary device settings.
*/
void BoardProgram::updateEntryCount(uint16_t entryCount)
{
  tempDeviceSettings[0x01] = (uint8_t)((entryCount >> 0x08) & 0xFF);
  tempDeviceSettings[0x02] = (uint8_t)(entryCount & 0xFF);
}

/*
  uint16_t getEntryCount(void) returns the amount of saved entries from temporary device settings.
*/
uint16_t BoardProgram::getEntryCount(void)
{
  return (uint16_t)(tempDeviceSettings[0x01] << 0x08) & tempDeviceSettings[0x02];
}

/*
  void BoardProgram::addUserCredential(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*) creates a new user credential
*/
void BoardProgram::addUserCredential(uint8_t* title, uint8_t* userName, uint8_t* email, uint8_t* password, uint8_t* url)
{
  uint8_t pageBuffer[MT25Q_PAGE_SIZE];
  uint16_t entryCount = getEntryCount();

  uint32_t credentialAddress = ((uint32_t)(entryCount) << 0x10) + 0x1000;  // Left shift entry count by 16 bits (0x10) and add 0x10 to start from 2nd page

  pageBuffer[0x00] = (uint8_t)((entryCount >> 0x08) & 0xFF);
  pageBuffer[0x01] = (uint8_t)(entryCount & 0xFF);

  // Write title (16 byte) starting from 3rd byte (0x02)
  for(auto i = 0; i < 16; i++)
  {
    pageBuffer[0x02 + i] = title[i];
  }

  // Write user name (32 byte) starting from 18th byte (0x12)
  for(auto i = 0; i < 32; i++)
  {
    pageBuffer[0x12 + i] = userName[i];
  }

  // Write email (64 byte) starting from 50th byte (0x32)
  for(auto i = 0; i < 64; i++)
  {
    pageBuffer[0x32 + i] = email[i];
  }

  // Write password (32 byte) starting from 114th byte (0x72)
  for(auto i = 0; i < 32; i++)
  {
    pageBuffer[0x72 + i] = password[i];
  }

  // Write url (24 byte) starting from 146th byte (0x92)
  for(auto i = 0; i < 24; i++)
  {
    pageBuffer[0x92 + i] = url[i];
  }

  encryptAndWrite(pageBuffer, credentialAddress);
}

/*
  uint8_t encryptAndWrite(uint8_t*, uint32_t) encrypts page bytes and writes it on the flash.
*/
uint8_t BoardProgram::encryptAndWrite(uint8_t* inputPage, uint32_t pageAddress)
{
  uint8_t output[256];
  if(cryptoEngine->cryptWithAes256(inputPage, output, MBEDTLS_AES_ENCRYPT) != 0)
  {
    printf("[Error] Could not encrypt data!\n");
    return 1;
  }

  spiFlash.updatePage(pageAddress, output);

  return 0;
}

/*
  uint8_t readAndDecrypt(uint8_t*, uint32_t) reads page from flash and decrypts it.
*/
uint8_t BoardProgram::readAndDecrypt(uint8_t* outputPage, uint32_t pageAddress)
{
  uint8_t pageBuffer[256];
  spiFlash.readBytes(pageAddress, pageBuffer, MT25Q_PAGE_SIZE);

  if(cryptoEngine->cryptWithAes256(pageBuffer, outputPage, MBEDTLS_AES_DECRYPT) != 0)
  {
    printf("[Error] Could not decrypt data!\n");
    return 1;
  }

  return 0;
}