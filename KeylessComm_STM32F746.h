#include "mbed.h"
#include "commands.h"
#include "EntryManager.h"
#include <cstdint>

#ifndef KEYLESS_COM_STM
#define KEYLESS_COM_STM

#define COM_SERIAL_TX PG_14
#define COM_SERIAL_RX PG_9
#define TIMEOUT_TIME 5000

typedef enum STATUS {
	STATUS_OK,
	STATUS_WRONG_PARAMETER,
	STATUS_TIMEOUT,
	STATUS_NACK,
	STATUS_INVALID_RESPONSE
} STATUS;

class KeylessCom {
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
		 * 	ID - The ID of the account as 16 bit unsigned int.
		 *	Title - The Title of the account.
		 *	Uname - The Username of the account.
		 *	Email - The Email address of the account.
		 *	Passwd - The Password of the account.
		 *	char URL - The URL of the website the account is for.
		 *
		 * returns:
		 *	STATUS - The status of the transmission as enum.
		 */
		STATUS sendAccount(uint16_t ID, char Title[MAX_TITLE_LEN], char Uname[MAX_UNAME_LEN], char Email[MAX_EMAIL_LEN], char Passwd[MAX_PASSWORD_LEN], char Url[MAX_URL_LEN]);

		/*+
		 * typeKeyboard makes the ATMEGA32U4 type something on the PC via USB HID Keyboard emulation.
		 * It accepts any combination of standard ASCII keys with a maximum of 128 sequential keystrokes.
		 *
		 * Inputs:
		 *	Keys - The keys to press.
		 *	size - the length of the Keys-Array. Not neccesary if the Array is Null-Terminated.
		 *
		 * returns:
		 *	STATUS - The status of the transmission as enum.
		 */
		STATUS typeKeyboard(char Keys[128], uint8_t size = 0);

    static BufferedSerial Serial;
    
	private:
    void processCommand();
    void copyArray(char* arrA, char* arrB, uint8_t& arrAIdx, uint8_t maxLength);
    STATUS checkForTimeout();

		Timer timeoutTimer;

		char commandBuffer[MAX_COMM_LEN];
		uint8_t commandBufferIdx = 0;

		bool inCommand = false;
		uint8_t ignoreCommandIdx = 0;

    EntryManager* entryManager;
};

#endif