#include "KeylessComm_STM32F746.h"

BufferedSerial KeylessCom::Serial(COM_SERIAL_TX, COM_SERIAL_RX, 9600);

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

void KeylessCom::process()
{
  if(Serial.readable())
  {
    char serialBuffer;
    Serial.read(&serialBuffer, 1);

    printf("[Info] Debug value: %x\n", serialBuffer);

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
  if(commandBuffer[0] == COMM_GET_ACC_NUM)
  {
    uint16_t accountNumber = entryManager->getEntryCount();

    switch(sendAccountNumber(accountNumber))
    {
      case STATUS_WRONG_PARAMETER:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_WRONG_PARAMETER\n\n");
    		break;
    	case STATUS_TIMEOUT:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_TIMEOUT\n\n");
    		break;
    	case STATUS_NACK:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_NACK\n\n");
    		break;
    	case STATUS_INVALID_RESPONSE:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_INVALID_RESPONSE\n\n");
    		break;
      case STATUS_OK:
        break;
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
    if(entryManager->getEntry(id, title, usr, email, pwd, url))
    {
      sendAccount(id, (char*)title, (char*)usr, (char*)email, (char*)pwd, (char*)url);      
    }
    else
    {
      Serial.write(&NACK, 1);
    }
  }
  else if (commandBuffer[0] == COMM_ADD_ACC)
  {
    char title[MAX_TITLE_LEN];
    char usr[MAX_UNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char pwd[MAX_PASSWORD_LEN];
    char url[MAX_URL_LEN];

    memset(title, 0, MAX_TITLE_LEN);
    memset(usr, 0, MAX_UNAME_LEN);
    memset(email, 0, MAX_EMAIL_LEN);
    memset(pwd, 0, MAX_PASSWORD_LEN);
    memset(url, 0, MAX_URL_LEN);

    uint8_t currentStringLen = 0;
    enum procStates {Title, Username, Email, Password, Url};
    uint8_t procState = Title;

    for(uint8_t i = 1; i < commandBufferIdx; i++)
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

    const char response = entryManager->addEntry(title, usr, email, pwd, url) ? ACK : NACK;
    Serial.write(&response, 1);

    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
  }
  else if(commandBuffer[0] == COMM_REM_ACC)
  {
    uint16_t id = (commandBuffer[1] << 8) | commandBuffer[2];
    
    const char response = entryManager->removeEntry(id) ? ACK : NACK;
    Serial.write(&response, 1);

    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
  }
  else if(commandBuffer[0] == COMM_EDIT_ACC)
  {
    char title[MAX_TITLE_LEN];
    char usr[MAX_UNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char pwd[MAX_PASSWORD_LEN];
    char url[MAX_URL_LEN];

    memset(title, 0, MAX_TITLE_LEN);
    memset(usr, 0, MAX_UNAME_LEN);
    memset(email, 0, MAX_EMAIL_LEN);
    memset(pwd, 0, MAX_PASSWORD_LEN);
    memset(url, 0, MAX_URL_LEN);

    uint8_t currentStringLen = 0;
    enum procStates {Title, Username, Email, Password, Url};
    uint8_t procState = Title;

    for(uint8_t i = 1; i < commandBufferIdx; i++)
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


    uint16_t id = (commandBuffer[1] << 8) | commandBuffer[2];
    const char response = entryManager->editEntry(id, title, usr, email, pwd, url) ? ACK : NACK;
    Serial.write(&response, 1);

    EntryManager::credentialInfo = entryManager->getEntriesTitleInfo();
  }
  else
  {
    Serial.write(&NACK, 1);
  }
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

  Serial.write(buffer, 5);

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

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

STATUS KeylessCom::sendAccount(unsigned short int ID, char Title[MAX_TITLE_LEN], char Uname[MAX_UNAME_LEN], char Email[MAX_EMAIL_LEN], char Passwd[MAX_PASSWORD_LEN], char Url[MAX_URL_LEN])
{
  char buffer[MAX_COMM_LEN] =
  {
    COMM_BEGIN,
    COMM_SEND_ACC,
    char((ID & 0xFF00) >> 8),
    char(ID & 0x00FF)
  };

  uint8_t bufferIdx = 4;

  copyArray(buffer, Title, bufferIdx, MAX_TITLE_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, Uname, bufferIdx, MAX_UNAME_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, Email, bufferIdx, MAX_EMAIL_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, Passwd, bufferIdx, MAX_PASSWORD_LEN);
  buffer[bufferIdx++] = US;
  copyArray(buffer, Url, bufferIdx, MAX_URL_LEN);
  buffer[bufferIdx++] = COMM_END;

  Serial.write(buffer, bufferIdx);

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

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

STATUS KeylessCom::typeKeyboard(char Keys[128], uint8_t size)
{
  bool endOnNull = false;
  if(size > 128)
  {
    return STATUS_WRONG_PARAMETER;
  }
  else if(size == 0)
  {
    endOnNull = true;
    size = 128;
  }

  char buffer[MAX_COMM_LEN] =
  {
    COMM_BEGIN,
    CTRL_TYPE_KB
  };

  uint8_t bufferIdx = 2;
  for(uint8_t i = 0; i < size; i++)
  {
    buffer[bufferIdx++] = Keys[i];

    if(Keys[i + 1] == '\0')
    {
      if(endOnNull)
      {
        break;
      }
      else
      {
        return STATUS_WRONG_PARAMETER;
      }
    }
  }

  buffer[bufferIdx++] = COMM_END;
  Serial.write(buffer, bufferIdx);

  timeoutTimer.reset();
  timeoutTimer.start();
  bool timeout = false;

  if(checkForTimeout() == STATUS_TIMEOUT)
  {
    return STATUS_TIMEOUT;
  }

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

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