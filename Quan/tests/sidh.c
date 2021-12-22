/********************************************************************************************
* SIDH: an efficient supersingular isogeny cryptography library
*
* Abstract: benchmarking/testing isogeny-based key exchange
*********************************************************************************************/ 

#include "SIDHp751_compressed.c"

unsigned char SharedSecretA[SIDH_BYTES], SharedSecretB[SIDH_BYTES];
int cryptotest_kex()
{ // Testing key exchange
    unsigned char PrivateKeyA[SIDH_SECRETKEYBYTES_A], PrivateKeyB[SIDH_SECRETKEYBYTES_B];
    unsigned char PublicKeyA[SIDH_PUBLICKEYBYTES], PublicKeyB[SIDH_PUBLICKEYBYTES];
    bool passed = true;

    printf("\n\nTESTING EPHEMERAL ISOGENY-BASED KEY EXCHANGE SYSTEM %s\n", SCHEME_NAME);
    printf("--------------------------------------------------------------------------------------------------------\n\n");

    random_mod_order_A(PrivateKeyA);
    random_mod_order_B(PrivateKeyB);

    EphemeralKeyGeneration_A(PrivateKeyA, PublicKeyA);                            // Get some value as Alice's secret key and compute Alice's public key
    EphemeralKeyGeneration_B(PrivateKeyB, PublicKeyB);                            // Get some value as Bob's secret key and compute Bob's public key
    EphemeralSecretAgreement_A(PrivateKeyA, PublicKeyB, SharedSecretA);           // Alice computes her shared secret using Bob's public key
    EphemeralSecretAgreement_B(PrivateKeyB, PublicKeyA, SharedSecretB);           // Bob computes his shared secret using Alice's public key

    if (memcmp(SharedSecretA, SharedSecretB, SIDH_BYTES) != 0) {
        passed = false;
        return 1;
    }

    if (passed == true) printf("  Key exchange tests (188 bytes) ............................... PASSED\n");
    else { printf("  Key exchange tests ... FAILED"); printf("\n"); return FAILED; }
    printf("\n"); 

    return PASSED;
}


// Функция для теста и бенчмаркинга SIDH
int testSIDH()
{
    int Status = PASSED;

    // Время запуска теста
    unsigned int start_time = clock();

    // Запуск теста
    Status = cryptotest_kex();
    if (Status != PASSED) {
        printf("\n\n   Error detected: KEX_ERROR_SHARED_KEY \n\n");
        return FAILED;
    }

    // Время окончания теста
    unsigned int end_time = clock();
    // Время выполнения теста
    printf("\n  Key exchanged in %dms\n\n", end_time - start_time);

    return Status;
}


// Irl генерация ключа
unsigned char PrivateKeyA[SIDH_SECRETKEYBYTES_A];
void generateKey(unsigned char PublicKeyA[SIDH_PUBLICKEYBYTES])
{
    //unsigned char PrivateKeyB[SIDH_SECRETKEYBYTES_B], PublicKeyB[SIDH_PUBLICKEYBYTES];

    random_mod_order_A(PrivateKeyA);
    //random_mod_order_B(PrivateKeyB);

    EphemeralKeyGeneration_A(PrivateKeyA, PublicKeyA);                            // Get some value as Alice's secret key and compute Alice's public key
    //EphemeralKeyGeneration_B(PrivateKeyB, PublicKeyB);                         // Get some value as Bob's secret key and compute Bob's public key
}
void agreeKeys(unsigned char SharedSecret[SIDH_BYTES], unsigned char PublicKey[SIDH_PUBLICKEYBYTES])
{
    EphemeralSecretAgreement_A(PrivateKeyA, PublicKey, SharedSecret);            // Alice computes her shared secret using Bob's public key
}