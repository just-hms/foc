#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#define SYMMLEN 256

//stores key and iv for AES cipher
struct sessionKey {
    unsigned char key[SYMMLEN/8], iv[SYMMLEN/8];
} typedef sessionKey;

class SymCrypt {
    unordered_map<uint8_t, sessionKey> session_keys;

    public:
        SymCrypt(uint8_t);
        void refresh(uint8_t);
        unsigned char* encrypt(uint8_t, unsigned char*);
        unsigned char* decrypt(uint8_t, unsigned char*);
        ~SymCrypt();
};

SymCrypt::SymCrypt(uint8_t userID) {
    this->refresh(userID);
}

//creates (or updates if already exists) a new key and iv to communicate with userID
void SymCrypt::refresh(uint8_t userID) {
    sessionKey k;
    RAND_bytes(k.key, sizeof(k.key));
    RAND_bytes(k.iv, sizeof(k.iv));
    this->session_keys[userID] = k;
}

//encrypts pt by using the userID's session key
unsigned char* SymCrypt::encrypt(uint8_t userID, unsigned char *pt) {
    EVP_CIPHER_CTX *ctx;
    int ptlen = strlen((char*)pt)+1, ctlen, len;
    unsigned char *ct;
    sessionKey k = this->session_keys[userID];

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, k.key, k.iv) <= 0) {
        cerr<<"Unable to initialize a context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    ct = new unsigned char[ptlen+16];

    if(EVP_EncryptUpdate(ctx, ct, &len, pt, ptlen) <= 0) {
        cerr<<"Unable to encrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] ct;
        return NULL;
    }
    ctlen = len;

    if(EVP_EncryptFinal_ex(ctx, ct + len, &len) <= 0)
    ctlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return ct;
    
}

//decrypts ct by using the userID's session key
unsigned char* SymCrypt::decrypt(uint8_t userID, unsigned char *ct) {
    EVP_CIPHER_CTX *ctx;
    int ctlen = strlen((char*)ct)+1, ptlen, len;
    unsigned char *pt;
    sessionKey k = this->session_keys[userID];

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        cerr<<"Unable to create a context for AES"<<endl;
        return NULL;
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, k.key, k.iv) <= 0) {
        cerr<<"Unable to initialize a context for AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    pt = new unsigned char[ctlen];

    if(EVP_DecryptUpdate(ctx, pt, &len, ct, ctlen) <= 0) {
        cerr<<"Unable to decrypt with AES"<<endl;
        EVP_CIPHER_CTX_free(ctx);
        delete[] pt;
        return NULL;
    }
    ptlen = len;

    if(EVP_DecryptFinal_ex(ctx, pt + len, &len) <= 0)
    ptlen += len;

    EVP_CIPHER_CTX_free(ctx);
    return pt;
}

SymCrypt::~SymCrypt() {    
    this->session_keys.clear();
}

/*EXAMPLE

//AES
SymCrypt b(0);
unsigned char *enc = b.encrypt(0, (unsigned char*)"Lorem ipsum dolor sit amet\n");
cout<<string((char*)b.decrypt(0, enc))<<endl;
*/
