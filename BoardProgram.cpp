#include "BoardProgram.h"
#include <chrono>
#include <cstdint>

Mutex BoardProgram::threadMutex;
bool BoardProgram::resetConfirmed = false;
Window* BoardProgram::templateWindow;
uint16_t BoardProgram::scrollIndex = 0;
uint16_t BoardProgram::currentEntry = 0;
uint8_t BoardProgram::masterPassword[6];
WindowType BoardProgram::currentWindow = Login;

/*
  BoardProgram(void) initializes class, display spi, touch spi and flash spi interface.
*/
BoardProgram::BoardProgram(void) : flashMemory(PB_15, PB_14, PB_13, PF_13), displayDriver(PF_9, PF_8, PF_7, PE_11, PF_15, PF_14), touchDriver(PE_6, PE_5, PE_12, PE_9)
{
  cryptoEngine = new CryptoEngine();
  entryManager = new EntryManager(&flashMemory, cryptoEngine);
  serialCommunication = new KeylessCom(9600, entryManager);
  templateWindow = new Window(&displayDriver, &touchDriver);
}

uint8_t BoardProgram::initialize(void)
{
  printf(BOARD_SOFTWARE_VERSION);
  printf("\n");

  if(!flashMemory.isAvailable())
  {
    return 1;
  }

  displayDriver.initialize();
  displayDriver.setRotation(3);

  touchDriver.initialize();
  touchDriver.setRotation(3);

  // Test Debug Section ----------------------------------------
  //flashMemory.eraseChip();
  //currentWindow = SendEntry;
  //currentEntry = 0;
  //masterPassword[0] = '0';
  //masterPassword[1] = '0';
  //masterPassword[2] = '0';
  //masterPassword[3] = '0';
  //masterPassword[4] = '0';
  //masterPassword[5] = '0';
  // -----------------------------------------------------------

  if(entryManager->needsToBeInitialized())
  {
    int returnValue = 0;
    do
    {
      flashMemory.eraseChip();
      returnValue = runFirstStartupRoutine();

      if(returnValue != 0)
      {
        printf("[Error] Error while executing FirstStartUpRoutine\n");
      }
    } while(returnValue != 0);
  }

  while(currentWindow == Login)
  {
    templateWindow->Load(loginForm_onLoad);
    currentWindow = entryManager->comparePassword(masterPassword) ? MainWindow : Login;
  }

  cryptoEngine->setMasterPassword(masterPassword);
  entryManager->loadSalt();
  cryptoEngine->generateAesKeyAndIV();
  
  return 0;
}

uint8_t BoardProgram::runFirstStartupRoutine(void)
{
  // Create new master password
  currentWindow = CreateLogin;

  while(currentWindow == CreateLogin)
  {
    templateWindow->Load(createLoginForm_onLoad);
    templateWindow->Load(repeatLoginForm_onLoad);
  }

  entryManager->savePassword(masterPassword);
  cryptoEngine->setMasterPassword(masterPassword);
  
  uint8_t generatedRandomSalt[MAX_SALT_LENGTH];
  if(cryptoEngine->generateRandomSalt(generatedRandomSalt) != 0)
  {
    printf("[Error] Error while generating Random Salt!\n");
    return 1;
  }

  entryManager->saveSalt(generatedRandomSalt);
  entryManager->setAsInitialized();

  entryManager->saveSettings();

  return 0;
}

uint8_t BoardProgram::run(void)
{
  Thread serialComThread;
  serialComThread.start([this]()
  {
    while(true)
    {
      serialCommunication->process();
    }
  });
  

  while(true)
  {
    serialCommunication->serialComMutex.lock();
    EntryManager::credentialInfo = vector<tuple<uint16_t, string>>(entryManager->getEntriesTitleInfo());
    serialCommunication->serialComMutex.unlock();

    templateWindow->Load(mainWindow_onLoad);

    if(currentWindow == SendEntry)
    {
      uint8_t kbData[128];
      uint8_t email[MAX_EMAIL_LEN];
      uint8_t pwd[MAX_PASSWORD_LEN];

      serialCommunication->serialComMutex.lock();
      entryManager->getEntry(currentEntry, NULL, NULL, email, pwd, NULL);
      serialCommunication->serialComMutex.unlock();

      uint8_t idx = 0;
      uint8_t dataIdx = 0;
      while(email[idx] != 0xFF && email[idx] != 0x00)
      {
        kbData[dataIdx++] = email[idx++];
      }

      kbData[dataIdx++] = US; // Unit Seperator for Tab

      idx = 0;
      while(pwd[idx] != 0xFF && pwd[idx] != 0x00)
      {
        kbData[dataIdx++] = pwd[idx++];
      }

      serialCommunication->serialComMutex.lock();
      serialCommunication->typeKeyboard((char*)kbData, dataIdx);
      serialCommunication->serialComMutex.unlock();

      currentWindow = MainWindow;
    }
    if(currentWindow == LogOff)
    {
      serialComThread.terminate();
      entryManager->saveSettings();
      break;
    }
    else if(currentWindow == ResetConfirm)
    {
      templateWindow->Load(resetConfirmWindow_onLoad);

      if(resetConfirmed)
      {
        serialComThread.terminate();
        flashMemory.eraseChip();
        break;
      }
    }
  }

  NVIC_SystemReset(); // Reset Board to restart software
  return 0;
}