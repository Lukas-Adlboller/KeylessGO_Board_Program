#include "mbed.h"
#include "commands.h"
#include "EntryManager.h"
#include <cstdint>

#ifndef KEYLESS_COM_STM
#define KEYLESS_COM_STM

#define COM_SERIAL_TX PG_14
#define COM_SERIAL_RX PG_9
#define TIMEOUT_TIME 2000

typedef enum STATUS
{
	STATUS_OK,
	STATUS_WRONG_PARAMETER,
	STATUS_TIMEOUT,
	STATUS_NACK,
	STATUS_INVALID_RESPONSE
} STATUS;

class KeylessCom
{
	public:
		/*+
		 * KeylessCom() is the Constructor and specifies the baud rate of the connection.
		 *
		 * Inputs:
		 *	speed - baudrate for the Serial UART connection.
     *  entryManager - pointer to class where entry functions are located.
		 *
		 * returns:
		 * 	None.
		 */
		KeylessCom(int speed, EntryManager* entryManager);

		/*+
		 * process() processes the incoming data and should be called periodically in the loop() section of the code
		 *
		 * Inputs:
		 *	None.
		 *
		 * returns:
		 * 	None.
		 */
		void process();

		/*+
		 * sendAccountNumber sends the number of saved accounts to the PC.
		 * It is used internally and USUALLY does not need to be used manually.
		 *
		 * Inputs:
		 *	accountNumber - The number of accounts as a 16-bit unsigned int.
		 *
		 * returns:
		 * 	STATUS - The status of the transmission as enum.
		 */
		STATUS sendAccountNumber(uint16_t accountNumber);

		/*+
		 * sendAccount sends an account information dataset to the PC.
		 *
		 * Inputs:
		 * 	id - The ID of the account as 16 bit unsigned int.
		 *	title - The Title of the account.
		 *	usr - The Username of the account.
		 *	email - The Email address of the account.
		 *	pwd - The Password of the account.
		 *	url - The URL of the website the account is for.
		 *
		 * returns:
		 *	STATUS - The status of the transmission as enum.
		 */
		STATUS sendAccount(uint16_t id, char title[MAX_TITLE_LEN], char usr[MAX_UNAME_LEN], char email[MAX_EMAIL_LEN], char pwd[MAX_PASSWORD_LEN], char url[MAX_URL_LEN]);

		/*+
		 * typeKeyboard makes the ATMEGA32U4 type something on the PC via USB HID Keyboard emulation.
		 * It accepts any combination of standard ASCII keys with a maximum of 128 sequential keystrokes.
		 *
		 * Inputs:
		 *	keys - The keys to press.
		 *	size - the length of the Keys-Array. Not neccesary if the Array is Null-Terminated.
		 *
		 * returns:
		 *	STATUS - The status of the transmission as enum.
		 */
		STATUS typeKeyboard(char keys[128], uint8_t size = 0);

    static BufferedSerial Serial;
    static Mutex serialComMutex;
    
	private:
    char getByte();
    void writeResponse(const char response);
    void processCommand();
    void copyArray(char* arrA, char* arrB, uint8_t& arrAIdx, uint8_t maxLength);
    void parseEntryData(char* title, char* usr, char* email, char* pwd, char* url, uint8_t startFromIndex);
    STATUS checkForTimeout();
    STATUS getResponse();

		Timer timeoutTimer;
		char commandBuffer[MAX_COMM_LEN];
		uint8_t commandBufferIdx = 0;
		bool inCommand = false;
		uint8_t ignoreCommandIdx = 0;
    EntryManager* entryManager;
};

#endif