#include "KeylessComm_STM32F746.h"
#include "mbed.h"
#include "commands.h"
#include <cstdint>

static BufferedSerial Serial(COM_SERIAL_TX, COM_SERIAL_RX);

KeylessCom::KeylessCom(int speed, EntryManager* entryManager)
{
  Serial.set_baud(speed);
  this->entryManager = entryManager;
}

void KeylessCom::process()
{
  if(Serial.readable())
  {
    char serialBuffer;
    Serial.read(&serialBuffer, 1);

    if(ignoreCommandIdx == 0)
    {
      switch(serialBuffer)
      {
        case COMM_BEGIN:
          inCommand = true;
          commandBufferIdx = 0;
          break;

        case COMM_GET_ACC:
          commandBuffer[commandBufferIdx] = serialBuffer;
          commandBufferIdx++;
          ignoreCommandIdx = 2;
          break;

        case COMM_REM_ACC:
          commandBuffer[commandBufferIdx] = serialBuffer;
          commandBufferIdx++;
          ignoreCommandIdx = 2;
          break;

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
  if(commandBuffer[0] == COMM_AUTH_BEGIN)
  {
    Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_AUTH_CODE)
  {
    Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_AUTH_AUTO)
  {
    Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_DISCONNECT)
  {
    Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_GET_ACC_NUM)
  {
    uint16_t accountNumber = entryManager->getEntryCount();

    switch(this->sendAccountNumber(accountNumber))
    {
      case STATUS_WRONG_PARAMETER:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_WRONG_PARAMETER\n\n");
    		break;
    	case STATUS_TIMEOUT:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_TIMEOUT\n\n");
    		break;
    	case	STATUS_NACK:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_NACK\n\n");
    		break;
    	case	STATUS_INVALID_RESPONSE:
    		printf("sendAccountNumber in getAccountNum failed with STATUS_INVALID_RESPONSE\n\n");
    		break;
    }

    /*uint8_t accountNumberLSB = accountNumber & 0b0000000011111111;
    uint8_t accountNumberMSB = (accountNumber & 0b1111111100000000) >> 8;
    char outBuffer[5] = {
      COMM_BEGIN,
      COMM_SEND_ACC_NUM,
      accountNumberMSB,
      accountNumberLSB,
      COMM_END
    };
    Serial.write(outBuffer, 5);*/
  }

  else if(commandBuffer[0] == COMM_GET_ACC)
  {
    // Deleted check if getAccount callback function is existing because now the functions from EntryManager will be used instead.
    // Now this part gets the information of an entry and sends it directly if possible instead of letting the getAccount function do that.

    uint16_t msb = commandBuffer[1];
    uint16_t lsb = commandBuffer[2];
    uint16_t id = (msb << 8) | lsb;

    uint8_t title[MAX_TITLE_LEN];
    uint8_t usr[MAX_UNAME_LEN];
    uint8_t email[MAX_EMAIL_LEN];
    uint8_t pwd[MAX_PASSWORD_LEN];
    uint8_t url[MAX_URL_LEN];

    if(entryManager->getEntry(id, title, usr, email, pwd, url) != true)
    {
      Serial.write(&NACK, 1);
    }
    else
    {
      sendAccount(id, (char*)title, (char*)usr, (char*)email, (char*)pwd, (char*)url);
    }
  }

  else if (commandBuffer[0] == COMM_ADD_ACC)
  {
    char Title[MAX_TITLE_LEN];
    char Uname[MAX_UNAME_LEN];
    char Email[MAX_EMAIL_LEN];
    char Passwd[MAX_PASSWORD_LEN];
    char Url[MAX_URL_LEN];

    memset(Title,0,sizeof(Title));
    memset(Uname,0,sizeof(Uname));
    memset(Email,0,sizeof(Email));
    memset(Passwd,0,sizeof(Passwd));
    memset(Url,0,sizeof(Url));

    uint8_t currentStringLen = 0;
    enum procStates {title = 0, uname = 1, email = 2, passwd = 3, url = 4};
    procStates procState = title;

    for(uint8_t i = 1; i < commandBufferIdx; i++)
    {
      if(commandBuffer[i] == US)
      {
        procState = static_cast<procStates>(static_cast<int>(procState) + 1);
        i++;
        currentStringLen = 0;
      }

      switch(procState)
      {
        case title:
          Title[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case uname:
          Uname[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case email:
          Email[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case passwd:
          Passwd[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case url:
          Url[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
      }
    }

    if(entryManager->addEntry(Title, Uname, Email, Passwd, Url)) Serial.write(&ACK, 1);
    else Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_REM_ACC)
  {
    uint16_t msb = commandBuffer[1];
    uint16_t lsb = commandBuffer[2];
    uint16_t id = (msb << 8) | lsb;
    
    if(entryManager->removeEntry(id)) Serial.write(&ACK, 1);
    else Serial.write(&NACK, 1);
  }

  else if(commandBuffer[0] == COMM_EDIT_ACC)
  {
    char Title[MAX_TITLE_LEN];
    char Uname[MAX_UNAME_LEN];
    char Email[MAX_EMAIL_LEN];
    char Passwd[MAX_PASSWORD_LEN];
    char Url[MAX_URL_LEN];

    memset(Title,0,sizeof(Title));
    memset(Uname,0,sizeof(Uname));
    memset(Email,0,sizeof(Email));
    memset(Passwd,0,sizeof(Passwd));
    memset(Url,0,sizeof(Url));

    uint8_t currentStringLen = 0;
    enum procStates {title = 0, uname = 1, email = 2, passwd = 3, url = 4};
    procStates procState = title;

    for(uint8_t i = 4; i < commandBufferIdx; i++)
    {
      if(commandBuffer[i] == US)
      {
        procState = static_cast<procStates>(static_cast<int>(procState) + 1);
        i++;
        currentStringLen = 0;
      }

      switch(procState)
      {
        case title:
          Title[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case uname:
          Uname[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case email:
          Email[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case passwd:
          Passwd[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
        case url:
          Url[currentStringLen] = commandBuffer[i];
          currentStringLen++;
          break;
      }
    }

    uint16_t msb = commandBuffer[1];
    uint16_t lsb = commandBuffer[2];
    uint16_t id = (msb << 8) | lsb;

    if(entryManager->editEntry(id, Title, Uname, Email, Passwd, Url)) Serial.write(&ACK, 1);
    else Serial.write(&NACK, 1);
  }
}

STATUS KeylessCom::sendAccountNumber(uint16_t accountNumber)
{

  char buffer[5] = {
    COMM_BEGIN,
    COMM_SEND_ACC_NUM,
    char((accountNumber & 0b1111111100000000) >> 8),
    char((accountNumber & 0b0000000011111111)),
    COMM_END
  };

  Serial.write(buffer, 5);

  timeoutTimer.reset();
  timeoutTimer.start();
  bool timeout = false;

  while((!Serial.readable()) && (!timeout))
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

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

  if(serialBuffer == ACK) return STATUS_OK;
  else if(serialBuffer == NACK) return STATUS_NACK;
  else return STATUS_INVALID_RESPONSE;
}

STATUS KeylessCom::sendAccount(unsigned short int ID, char Title[MAX_TITLE_LEN], char Uname[MAX_UNAME_LEN], char Email[MAX_EMAIL_LEN], char Passwd[MAX_PASSWORD_LEN], char Url[MAX_URL_LEN])
{
  char buffer[MAX_COMM_LEN];

  buffer[0] = COMM_BEGIN;
  buffer[1] = COMM_SEND_ACC;

  buffer[2] = char((ID & 0xFF00) >> 8);
  buffer[3] = char(ID & 0x00FF);

  uint8_t bufferi = 4;
  uint8_t i = 0;

  for(i = 0; i < MAX_TITLE_LEN; i++)
  {
    buffer[bufferi] = Title[i];
    bufferi++;
    if(Title[i + 1] == '\0')
    {
      break;
    }
  }

  buffer[bufferi] = US;
  bufferi++;

  for(i = 0; i < MAX_UNAME_LEN; i++)
  {
    buffer[bufferi] = Uname[i];
    bufferi++;
    if(Uname[i + 1] == '\0')
    {
      break;
    }
  }

  buffer[bufferi] = US;
  bufferi++;

  for(i = 0; i < MAX_EMAIL_LEN; i++)
  {
    buffer[bufferi] = Email[i];
    bufferi++;
    if(Email[i + 1] == '\0')
    {
      break;
    }
  }

  buffer[bufferi] = US;
  bufferi++;

  for(i = 0; i < MAX_PASSWORD_LEN; i++)
  {
    buffer[bufferi] = Passwd[i];
    bufferi++;
    if(Passwd[i + 1] == '\0')
    {
      break;
    }
  }

  buffer[bufferi] = US;
  bufferi++;

  for(i = 0; i < MAX_URL_LEN; i++)
  {
    buffer[bufferi] = Url[i];
    bufferi++;
    if(Url[i + 1] == '\0')
    {
      break;
    }
  }

  buffer[bufferi] = COMM_END;
  bufferi++;

  Serial.write(buffer, bufferi);

  timeoutTimer.reset();
  timeoutTimer.start();
  bool timeout = false;

  while((!Serial.readable()) && (!timeout))
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

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

  if(serialBuffer == ACK) return STATUS_OK;
  else if(serialBuffer == NACK) return STATUS_NACK;
  else return STATUS_INVALID_RESPONSE;

}

STATUS KeylessCom::typeKeyboard(char Keys[128], uint8_t size)
{
  if(size > 128)
  {
    return STATUS_WRONG_PARAMETER;
  }
  char buffer[MAX_COMM_LEN];
  uint8_t bufferi = 0;

  buffer[0] = COMM_BEGIN;
  buffer[1] = CTRL_TYPE_KB;

  bool endOnNull = false;

  if(size == 0)
  {
    endOnNull = true;
    size = 128;
  }

  bufferi = 2;

  for(uint8_t i = 0; i < size; i++)
  {
    buffer[bufferi] = Keys[i];
    bufferi++;

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

  buffer[bufferi] = COMM_END;
  bufferi++;

  Serial.write(buffer, bufferi);

  timeoutTimer.reset();
  timeoutTimer.start();
  bool timeout = false;

  while((!Serial.readable()) && (!timeout))
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

  char serialBuffer;
  Serial.read(&serialBuffer, 1);

  if(serialBuffer == ACK) return STATUS_OK;
  else if(serialBuffer == NACK) return STATUS_NACK;
  else return STATUS_INVALID_RESPONSE;

  return STATUS_OK;
}
