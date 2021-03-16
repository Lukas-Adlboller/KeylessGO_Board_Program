#include "CryptoEngine.h"
#include "mbed.h"
#include <cstdint>
#include "pkcs5.h"

/*
  CryptoEngine(void) initializes class.
  It also initializes True Random Number Generator.
*/
CryptoEngine::CryptoEngine(void)
{
  // Initialize TRNG
  __HAL_RCC_RNG_CLK_ENABLE();
}

/*
  uint8_t hashWithSha256(uint8_t*, uint8_t*) hashes an input byte array with SHA-256.

  Error Return Values:
    (1) -> SHA-256 is not available
    (2) -> Error while hashing data
*/
uint8_t CryptoEngine::hashWithSha256(uint8_t *input, uint8_t *output)
{
  const mbedtls_md_info_t* sha256_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  if(sha256_info == NULL)
  {
    printf("[Error] SHA-256 is not available!\n");
    return 1;
  }

  char c = ' ';
  uint8_t stringSize = 0;
  while((c = input[stringSize]) != '\0')
  {
    stringSize++;
  }

  if(mbedtls_md(sha256_info, input, stringSize, output) != 0)
  {
    printf("[Error] Could not hash data array!\n");
    return 2;
  }

  printf("[Info] \'%s\' was hashed into \'", input);
  for(auto i = 0; i < 32; i++)
  {
    printf("%x", output[i]);
  }
  printf("\'\n");

  return 0;
}

/*
  uint8_t generateRandomSalt(uint8_t*) generates a salt out of random numbers from TRNG.

  Error Return Values:
    (1) -> Error while initializing HAL_RNG instance
    (2) -> Error while generating random values
*/
uint8_t CryptoEngine::generateRandomSalt(uint8_t *output)
{
  RNG_HandleTypeDef rngInstance;
  rngInstance.Instance = RNG;

  if(HAL_RNG_Init(&rngInstance) != HAL_OK)
  {
    return 1;
  }

  uint32_t randomData = 0xFF;
  for(auto i = 0; i < MAX_SALT_LENGTH; i++)
  {
    if(HAL_RNG_GenerateRandomNumber(&rngInstance, &randomData) != HAL_OK)
    {
      HAL_RNG_DeInit(&rngInstance);
      return 2;
    }

    output[i] = (uint8_t)(randomData & 0xFF);
  }

  HAL_RNG_DeInit(&rngInstance);

  printf("[Info] Random generated salt \'");
  for(auto i = 0; i < MAX_SALT_LENGTH; i++)
  {
    printf("%x", output[i]);
  }
  printf("\'\n");

  return 0;
}

/*
  uint8_t generateAesKeyAndIvV(void) generates AES Key (128 byte) and IV (16 Byte) with PBKDF2-HMAC algorithm.

  Error Return Values:
    (1) -> SHA-256 is not available
    (2) -> Error while hashing data
*/
uint8_t CryptoEngine::generateAesKeyAndIV(void)
{
  mbedtls_md_context_t sha256_context;
  const mbedtls_md_info_t* sha256_info;

  mbedtls_md_init(&sha256_context);
  sha256_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  if(sha256_info == NULL)
  {
    printf("[Error] SHA-256 is not available!\n");
    return 1;
  }

  if(mbedtls_md_setup(&sha256_context, sha256_info, 1) != 0)
  {
    printf("[Error] Could not setup encryption!\n");
    return 2;
  }

  mbedtls_pkcs5_pbkdf2_hmac(&sha256_context, masterPassword, 32, generatedSalt, 16, 512, 128, generatedAesKey);
  mbedtls_pkcs5_pbkdf2_hmac(&sha256_context, masterPassword, 32, generatedSalt, 16, 512, 16, generatedAesIV);

  return 0;
}

/*
  uint8_t cryptWithAesCBC(void) en- or decrypts a 128 byte array using AES-CBC algorithm.

  Error Return Values:
    (1) -> Parameter error
*/
uint8_t CryptoEngine::cryptWithAesCBC(uint8_t* input, uint8_t* output, int mode)
{
  mbedtls_aes_context aes_context;
  
  switch(mode)
  {
    case MBEDTLS_AES_ENCRYPT:
      mbedtls_aes_setkey_enc(&aes_context, generatedAesKey, 128);
      break;
    case MBEDTLS_AES_DECRYPT:
      mbedtls_aes_setkey_dec(&aes_context, generatedAesKey, 128);
      break;
    default:
      return 1;
  }

  uint8_t iv[16];
  for(auto i = 0; i < 16; i++)
  {
    iv[i] = generatedAesIV[i];
  }

  mbedtls_aes_crypt_cbc(&aes_context, mode, 128, iv, input, output);

  return 0;
}

/*
  void setSalt(uint8_t*) sets salt.
*/
void CryptoEngine::setSalt(uint8_t* salt)
{
  for(auto i = 0; i < MAX_SALT_LENGTH; i++)
  {
    generatedSalt[i] = salt[i];
  }
}

/*
  void setMasterPassword(uint8_t*) sets the master password.
*/
void CryptoEngine::setMasterPassword(uint8_t* pwd)
{
  for(auto i = 0; i < MASTER_PASSWORD_LENGTH; i++)
  {
    masterPassword[i] = pwd[i];
  }
}