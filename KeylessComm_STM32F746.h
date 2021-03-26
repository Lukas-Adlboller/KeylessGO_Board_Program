#include "EntryManager.h"

#ifndef KEYLESS_COM_STM
#define KEYLESS_COM_STM

#define COM_SERIAL_TX D1
#define COM_SERIAL_RX D0

#define TIMEOUT_TIME 5000

#include "mbed.h"
#include "commands.h"

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

	private:
    void processCommand();

		Timer timeoutTimer;

		char commandBuffer[MAX_COMM_LEN];
		uint8_t commandBufferIdx = 0;

		bool inCommand = false;
		uint8_t ignoreCommandIdx = 0;

    EntryManager* entryManager;
};


/*
 * Callback Functions
 */

#if defined (__cplusplus)
extern "C" {
#endif

/*+
 * getAccountNum is called when the PC requests the Number of accounts.
 *
 * Inputs:
 *	None.
 *
 * returns:
 *	The number of accounts as unsigned 16-bit int.
 */
extern uint16_t getAccountNum() __attribute__((weak));

/*+
 * getAccount is called when the PC requests an account dataset.
 * Please call sendAccount in this function.
 *
 * Inputs:
 *	id - The ID of the account.
 *
 * returns:
 *	frue or false as bool. Return false if the dataset cannot be loaded for any reason.
 */
extern bool getAccount(uint16_t id) __attribute__((weak));

/*+
 * addAccount is called when the PC wants to add an account to the database.
 *
 * Inputs:
 *	Title - The Title of the account.
 *	Uname - The Username of the account.
 *	Email - The Email address of the account.
 *	Passwd - The Password of the account.
 *	char URL - The URL of the website the account is for.
 *
 * returns:
 *	frue of false as bool. return true if the account was sucessfully added to the database, otherwise return false.
 */
extern bool addAccount(char Title[], char Uname[], char Email[], char Passwd[], char Url[]) __attribute__((weak));

/*+
 * remAccount is called when the PC wants to remove an account from the database.
 * DO NOT change the IDs of any accounts for this session. Instead flag the account as deleted.
 *
 * Inputs:
 *	id - The ID of the account
 *
 * returns:
 *	true or false as bool. Return true when the account was sucessfully deleted, otherwise return false.
 */
extern bool remAccount(uint16_t id) __attribute__((weak));

/*+
 * editAccount is called when the PC wants to change an account.
 *
 * Inputs:
 *	id - The ID of the account
 *	Title - The Title of the account.
 *	Uname - The Username of the account.
 *	Email - The Email address of the account.
 *	Passwd - The Password of the account.
 *	char URL - The URL of the website the account is for.
 *
 * returns:
 *	true or false as bool. Return true if the account was sucessfully edited, otherwise return false.
 */
extern bool editAccount(uint16_t id, char Title[], char Uname[], char Email[], char Passwd[], char Url[]) __attribute__((weak));

#if defined (__cplusplus)
}
#endif

#endif
