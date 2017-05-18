#ifndef CRYPT_H
#define CRYPT_H

int encrypt(void* buffer, int buff_len, char* iv, char* key, int key_len);
int decrypt(void* buffer, int buff_len, char* iv, char* key, int key_len);





#endif
