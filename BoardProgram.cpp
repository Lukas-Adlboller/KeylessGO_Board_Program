#include "BoardProgram.h"
#include <cstdint>

Mutex BoardProgram::threadMutex;
vector<Window> BoardProgram::programWindows;
uint16_t BoardProgram::scrollIndex = 0;
uint8_t BoardProgram::masterPassword[6];
bool BoardProgram::pwdSet = false;
bool BoardProgram::loginSuccessful = false;

/*
  BoardProgram(void) initializes class, display spi, touch spi and flash spi interface.
*/
BoardProgram::BoardProgram(void) : flashMemory(PE_6, PE_5, PE_12, D3), displayDriver(D11, D12, D13, D7, D6, D5), touchDriver(D63, D61, D62, D4)
{
  reset();
  cryptoEngine = new CryptoEngine();
  entryManager = new EntryManager(&flashMemory, cryptoEngine);
  serialCommunication = new KeylessCom(9600, entryManager);

  Window templateWindow = Window(&displayDriver, &touchDriver);
  programWindows.push_back(templateWindow); // Login Window
  programWindows.push_back(templateWindow); // Create Login Window
  programWindows.push_back(templateWindow); // Repeat Login Window
  programWindows.push_back(templateWindow); // Main Menue Window
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
  //programWindows[3].Load(mainWindow_onLoad);
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

  while(loginSuccessful == false)
  {
    programWindows[0].Load(loginForm_onLoad);
    loginSuccessful = entryManager->comparePassword(masterPassword);
  }

  printf("[Info] Login successfull!\n");
  
  return 0;
}

uint8_t BoardProgram::runFirstStartupRoutine(void)
{
  // Create new master password
  while(pwdSet == false)
  {
    programWindows[1].Load(createLoginForm_onLoad);
    programWindows[2].Load(repeatLoginForm_onLoad);
  }

  printf("[Info] New Master Password is: %d %d %d %d %d %d\n", masterPassword[0], masterPassword[1], masterPassword[2], masterPassword[3], masterPassword[4], masterPassword[5]);

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
  entryManager->addEntry("Twitter", "UserA", "no", "1234", "no");
  entryManager->addEntry("Twitch", "UserB", "no", "1234", "no");
  entryManager->addEntry("Google", "UserC", "no", "1234", "no");
  entryManager->addEntry("Steam", "UserD", "no", "1234", "no");
  entryManager->addEntry("Bulme", "UserE", "no", "1234", "no");
  entryManager->addEntry("Reddit", "UserF", "no", "1234", "no");
  EntryManager::credentialInfo = vector<tuple<uint16_t, string>>(entryManager->getEntriesTitleInfo());

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

  programWindows[3].Load(mainWindow_onLoad);

  return 0;
}