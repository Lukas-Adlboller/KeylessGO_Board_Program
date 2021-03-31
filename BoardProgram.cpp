#include "BoardProgram.h"
#include <cstdint>

Mutex BoardProgram::threadMutex;
bool BoardProgram::resetConfirmed = false;
Window* BoardProgram::templateWindow;
uint16_t BoardProgram::scrollIndex = 0;
uint8_t BoardProgram::masterPassword[6];
WindowType BoardProgram::currentWindow = Login;

/*
  BoardProgram(void) initializes class, display spi, touch spi and flash spi interface.
*/
BoardProgram::BoardProgram(void) : flashMemory(PE_6, PE_5, PE_12, D3), displayDriver(D11, D12, D13, D7, D6, D5), touchDriver(D63, D61, D62, D4)
{
  cryptoEngine = new CryptoEngine();
  entryManager = new EntryManager(&flashMemory, cryptoEngine);
  serialCommunication = new KeylessCom(9600, entryManager);
  templateWindow = new Window(&displayDriver, &touchDriver);
}

/*
  void reset(void) resets device. Flash memory will be ereased where settings are stored.
*/
void BoardProgram::reset(void)
{
  flashMemory.eraseChip();
}

uint8_t BoardProgram::initialize(void)
{
  if(!flashMemory.isAvailable())
  {
    return -1;
  }

  displayDriver.initialize();
  touchDriver.initialize();

  displayDriver.setRotation(3);
  touchDriver.setRotation(3);

  // Test Debug Section ----------------------------------------
  //reset(); // For testing purposes
  //entryManager->reloadSettings();
  //templateWindow->Load(resetConfirmWindow_onLoad);
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

  // printf("[Info] Login successfull!\n");
  
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

  // printf("[Info] New Master Password is: %d %d %d %d %d %d\n", masterPassword[0], masterPassword[1], masterPassword[2], masterPassword[3], masterPassword[4], masterPassword[5]);

  uint8_t generatedRandomSalt[MAX_SALT_LENGTH];
  entryManager->savePassword(masterPassword);
  cryptoEngine->setMasterPassword(masterPassword);
  
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
  /* Test Debug Section
  entryManager->addEntry("Twitter", "UserA", "no", "1234", "no");
  entryManager->addEntry("Twitch", "UserB", "no", "1234", "no");
  entryManager->addEntry("Google", "UserC", "no", "1234", "no");
  entryManager->addEntry("Steam", "UserD", "no", "1234", "no");
  entryManager->addEntry("Bulme", "UserE", "no", "1234", "no");
  entryManager->addEntry("Reddit", "UserF", "no", "1234", "no");
  // ------------------- */

  Thread serialComThread;
  serialComThread.start([this]()
  {
    while(true)
    {
      threadMutex.lock();
      serialCommunication->process();
      threadMutex.unlock();
    }
  });

  while(true)
  {
    EntryManager::credentialInfo = vector<tuple<uint16_t, string>>(entryManager->getEntriesTitleInfo());
    templateWindow->Load(mainWindow_onLoad);

    if(currentWindow == LogOff)
    {
      entryManager->saveSettings();
      break;
    }
    else if(currentWindow == ResetConfirm)
    {
      templateWindow->Load(resetConfirmWindow_onLoad);

      if(resetConfirmed)
      {
        serialComThread.terminate();
        reset();
        entryManager->reloadSettings();
        resetConfirmed = false;
        break;
      }
    }
  }

  NVIC_SystemReset(); // Reset Board to restart software
  return 0;
}