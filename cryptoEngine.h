#include <cstdint>

#define MASTER_PASSWORD_LENGTH  32
#define MAX_SALT_LENGTH         16

class CryptoEngine
{
  public:
    CryptoEngine(uint8_t* password);
    uint8_t hashWithSha256(uint8_t* input, uint8_t* output);
    uint8_t generateRandomSalt(uint8_t* output);
    uint8_t cryptWithAes256(uint8_t* input, uint8_t* output, int mode);
    uint8_t generateAesKeyAndIV(void);
    void setSalt(uint8_t* salt);

  private:
    uint8_t masterPassword[32];
    uint8_t generatedAesKey[256];
    uint8_t generatedAesIV[16];
    uint8_t generatedSalt[16];
};