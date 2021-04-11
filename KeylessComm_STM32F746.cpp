#include "KeylessComm_STM32F746.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>

BufferedSerial KeylessCom::Serial(COM_SERIAL_TX, COM_SERIAL_RX, 115200);
Mutex KeylessCom::serialComMutex;

KeylessCom::KeylessCom(int speed, EntryManager* entryManager)
{
  this->entryManager = entryManager;
}

STATUS KeylessCom::checkForTimeout()
{
  timeoutTimer.reset();
  timeoutTimer.start();
  bool timeout = false;

  while(!Serial.readable() && !timeout)
  {
    if(std::chrono::duration_cast<std::chrono::milliseconds>(timeoutTimer.elapsed_time()).count() >= TIMEOUT_TIME)
    {
      timeout = true;
    }
  }

  timeoutTimer.stop();

  if(timeout)
  {
    return STATUS_TIMEOUT;
  }

  return STATUS_OK;
}

char KeylessCom::getByte()
{
  char serialByte;
  Serial.read(&serialByte, 1);
  return serialByte;
}

void KeylessCom::writeResponse(const char response)
{
  Serial.write(&response, 1);
}

STATUS KeylessCom::getResponse()
{
  char serialBuffer = getByte();
  switch(serialBuffer)
  {
    case ACK:
      return STATUS_OK;
    case NACK:
      return STATUS_NACK;
    default:
      return STATUS_INVALID_RESPONSE;
  }
}

void KeylessCom::copyArray(char* arrA, char* arrB, uint8_t& arrAIdx, uint8_t maxLength)
{
  for(auto i = 0; i < maxLength; i++)
  {
    arrA[arrAIdx++] = arrB[i];
    if(arrB[i + 1] == '\0')
    {
      break;
    }
  }
}

void KeylessCom::parseEntryData(char* title, char* usr, char* email, char* pwd, char* url, uint8_t startFromIndex)
{
  memset(title, 0, MAX_TITLE_LEN);
  memset(usr, 0, MAX_UNAME_LEN);
  memset(email, 0, MAX_EMAIL_LEN);
  memset(pwd, 0, MAX_PASSWORD_LEN);
  memset(url, 0, MAX_URL_LEN);

  uint8_t currentStringLen = 0;
  enum procStates {Title, Username, Email, Password, Url};
  uint8_t procState = Title;

  for(uint8_t i = startFromIndex; i < commandBufferIdx; i++)
  {
    if(commandBuffer[i] == US)
    {
      procState++;
      i++;
      currentStringLen = 0;
    }

    switch(procState)
    {
      case Title:
        title[currentStringLen] = commandBuffer[i];
        break;
      case Username:
        usr[currentStringLen] = commandBuffer[i];
        break;
      case Email:
        email[currentStringLen] = commandBuffer[i];
        break;
      case Password:
        pwd[currentStringLen] = commandBuffer[i];
        break;
      case Url:
        url[currentStringLen] = commandBuffer[i];
        break;
    }

    currentStringLen++;
  }
}

void KeylessCom::process()
{
  if(Serial.readable())
  {
    char serialBuffer = getByte();

    if(ignoreCommandIdx == 0)
    {
      switch(serialBuffer)
      {
        case COMM_BEGIN:
          inCommand = true;
          commandBufferIdx = 0;
          break;
        case COMM_GET_ACC:
        case COMM_REM_ACC:
        case COMM_EDIT_ACC:
          commandBuffer[commandBufferIdx] = serialBuffer;
          commandBufferIdx++;
          ignoreCommandIdx = 2;
          break;
        case COMM_END:
          if(inCommand)
          {
            inCommand = false;
            processCommand();
          }
          break;
        default:
          if(inCommand)
          {
            commandBuffer[commandBufferIdx] = serialBuffer;
            commandBufferIdx++;
          }
          break;
      }
    }
    else
    {
      if(inCommand)
      {
        commandBuffer[commandBufferIdx] = serialBuffer;
        commandBufferIdx++;
      }
      ignoreCommandIdx--;
    }
  }
}

void KeylessCom::processCommand()
{
  char response = NACK;
  if(commandBuffer[0] == COMM_GET_ACC_NUM)
  {
    uint16_t accountNumber = entryManager->getEntryCount();

    switch(sendAccountNumber(accountNumber))
    {
      case STATUS_WRONG_PARAMETER:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_WRONG_PARAMETER\n\n");
    		return;
    	case STATUS_TIMEOUT:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_TIMEOUT\n\n");
    		return;
    	case STATUS_NACK:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_NACK\n\n");
    		return;
    	case STATUS_INVALID_RESPONSE:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_INVALID_RESPONSE\n\n");
    		return;
      case STATUS_OK:
        return;
    }
  }
  else if(commandBuffer[0] == COMM_GET_ACC)
  {
    uint8_t title[MAX_TITLE_LEN];
    uint8_t usr[MAX_UNAME_LEN];
    uint8_t email[MAX_EMAIL_LEN];
    uint8_t pwd[MAX_PASSWORD_LEN];
    uint8_t url[MAX_URL_LEN];

    uint16_t id = (commandBuffer[1] << 8) | commandBuffer[2];
    serialComMutex.lock();
    bool retVal = entryManager->getEntry(id, title, usr, email, pwd, url);
    serialComMutex.unlock();

    if(retVal)
    {
      sendAccount(id, (char*)title, (char*)usr, (char*)email, (char*)pwd, (char*)url);    
      return;  
    }
  }
  else if (commandBuffer[0] == COMM_ADD_ACC)
  {
    char title[MAX_TITLE_LEN];
    char usr[MAX_UNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char pwd[MAX_PASSWORD_LEN];
    char url[MAX_URL_LEN];

    parseEntryData(title, usr, email, pwd, url, 1);

    serialComMutex.lock();
    response = entryManager->addEntry(title, usr, email, pwd, url) ? ACK : NACK;
    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
    serialComMutex.unlock();
  }
  else if(commandBuffer[0] == COMM_REM_ACC)
  {
    uint16_t id = (commandBuffer[1] << 8) | commandBuffer[2];

    serialComMutex.lock();
    response = entryManager->removeEntry(id) ? ACK : NACK;
    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
    serialComMutex.unlock();
  }
  else if(commandBuffer[0] == COMM_EDIT_ACC)
  {
    char title[MAX_TITLE_LEN];
    char usr[MAX_UNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char pwd[MAX_PASSWORD_LEN];
    char url[MAX_URL_LEN];

    parseEntryData(title, usr, email, pwd, url, 4);

    uint16_t id = (commandBuffer[1] << 8) | commandBuffer[2];

    serialComMutex.lock();
    response = entryManager->editEntry(id, title, usr, email, pwd, url) ? ACK : NACK;
    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
    serialComMutex.unlock();
  }
  else if(commandBuffer[0] == COMM_GET_UNIQUE_ID)
  {
    uint16_t uniqueId = entryManager->getUniqueId();
    const char dataToSend[5] =
    {
      COMM_BEGIN,
      COMM_SEND_UNIQUE_ID,
      (char)((uniqueId & 0xFF00) >> 8),
      (char)(uniqueId & 0xFF),
      COMM_END
    };

    serialComMutex.lock();
    Serial.write(dataToSend, 5);
    serialComMutex.unlock();
    return;
  }
  else if(commandBuffer[0] == COMM_GET_ALL_ENTRIES)
  {
    for(auto entry : entryManager->credentialInfo)
    {
      uint8_t title[MAX_TITLE_LEN];
      uint8_t usr[MAX_UNAME_LEN];
      uint8_t email[MAX_EMAIL_LEN];
      uint8_t pwd[MAX_PASSWORD_LEN];
      uint8_t url[MAX_URL_LEN];

      uint16_t id = get<0>(entry);
      serialComMutex.lock();
      bool retVal = entryManager->getEntry(id, title, usr, email, pwd, url);
      serialComMutex.unlock();

      if(retVal)
      {
        sendAccount(id, (char*)title, (char*)usr, (char*)email, (char*)pwd, (char*)url);
      }
    }
    return;
  }
  
  writeResponse(response);
}

STATUS KeylessCom::sendAccountNumber(uint16_t accountNumber)
{
  char buffer[5] =
  {
    COMM_BEGIN,
    COMM_SEND_ACC_NUM,
    char((accountNumber & 0xFF00) >> 8),
    char((accountNumber & 0xFF)),
    COMM_END
  };

  serialComMutex.lock();
  Serial.write(buffer, 5);
  serialComMutex.unlock();

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  return getResponse();
}

STATUS KeylessCom::sendAccount(uint16_t id, char title[MAX_TITLE_LEN], char usr[MAX_UNAME_LEN], char email[MAX_EMAIL_LEN], char pwd[MAX_PASSWORD_LEN], char url[MAX_URL_LEN])
{
  char buffer[MAX_COMM_LEN] =
  {
    COMM_BEGIN,
    COMM_SEND_ACC,
    char((id & 0xFF00) >> 8),
    char(id & 0x00FF)
  };

  uint8_t bufferIdx = 4;

  copyArray(buffer, title, bufferIdx, MAX_TITLE_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, usr, bufferIdx, MAX_UNAME_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, email, bufferIdx, MAX_EMAIL_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, pwd, bufferIdx, MAX_PASSWORD_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, url, bufferIdx, MAX_URL_LEN);
  buffer[bufferIdx++] = COMM_END;

  serialComMutex.lock();
  Serial.write(buffer, bufferIdx);
  serialComMutex.unlock();

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  return getResponse();
}

STATUS KeylessCom::typeKeyboard(char keys[128], uint8_t size)
{
  if(size > 128)
  {
    return STATUS_WRONG_PARAMETER;
  }
  else if(size == 0)
  {
    size = 128;
  }

  char buffer[MAX_COMM_LEN] =
  {
    COMM_BEGIN,
    CTRL_TYPE_KB
  };

  uint8_t bufferIdx = 2;
  copyArray(buffer, keys, bufferIdx, size);

  buffer[bufferIdx++] = COMM_END;
  serialComMutex.lock();
  Serial.write(buffer, bufferIdx);
  serialComMutex.unlock();

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  return getResponse();
}