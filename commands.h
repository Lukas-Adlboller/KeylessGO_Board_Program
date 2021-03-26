#include <cstdint>

#ifndef KEYLESS_COMMS_LIST
#define KEYLESS_COMMS_LIST

//Lengths in bytes as char-array
const uint8_t MAX_TITLE_LEN     = 16;
const uint8_t MAX_UNAME_LEN     = 32;
const uint8_t MAX_EMAIL_LEN     = 64;
const uint8_t MAX_PASSWORD_LEN  = 32;
const uint8_t MAX_URL_LEN       = 24;

const uint8_t MAX_COMM_LEN = MAX_TITLE_LEN + MAX_UNAME_LEN + MAX_EMAIL_LEN + MAX_PASSWORD_LEN + MAX_URL_LEN + 11;



//General Commands
//begins command transmission
const char COMM_BEGIN  = 0x02;
//ends command transmission
const char COMM_END    = 0x03;
//Answer when the Command is invalid. Does not use COMM_BEGIN and COMM_END. Doubles as bool NO in command operation.
const char NACK        = 0x15;
//Answer when the Command is valid and no return value is provided. Does not use COMM_BEGIN and COMM_END. Doubles as bool YES in command operation.
const char ACK         = 0x06;
//unit seperator
const char US          = 0x1F;

//PC Commands
const char COMM_AUTH_BEGIN   = 0x21;
const char COMM_AUTH_CODE    = 0x22;
const char COMM_AUTH_AUTO    = 0x23;
const char COMM_GET_ACC_NUM	 = 0x24;
const char COMM_GET_ACC      = 0x25;
const char COMM_ADD_ACC      = 0x26;
const char COMM_REM_ACC      = 0x27;
const char COMM_EDIT_ACC     = 0x28;

//PC and Device commands
const char COMM_DISCONNECT = 0x35;

//Device Commands
const char COMM_SEND_ACC_NUM  = 0x40;
const char COMM_SEND_ACC			= 0x41;

//Internal Control Commands
const char CTRL_TYPE_KB = '2';

#endif
