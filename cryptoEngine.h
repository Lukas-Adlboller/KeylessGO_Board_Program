#include "mbed.h"
#include "pkcs5.h"
#include <cstdint>

#define MASTER_PASSWORD_LENGTH  6
#define MAX_SALT_LENGTH         16

#ifndef CRYPTO_ENGINE_H
#define CRYPTO_ENGINE_H

class CryptoEngine
{
  public:
    CryptoEngine(void);
    uint8_t hashWithSha256(uint8_t* input, uint8_t* output);
    uint8_t generateRandomSalt(uint8_t* output);
    uint8_t cryptWithAesCBC(uint8_t* input, uint8_t* output, int mode);
    uint8_t generateAesKeyAndIV(void);
    void setSalt(uint8_t* salt);
    void setMasterPassword(uint8_t* pwd);

  private:
    uint8_t masterPassword[MASTER_PASSWORD_LENGTH];
    uint8_t generatedAesKey[128];
    uint8_t generatedAesIV[16];
    uint8_t generatedSalt[MAX_SALT_LENGTH];
};

#endif