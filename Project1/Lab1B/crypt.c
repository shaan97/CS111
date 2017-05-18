#include <mcrypt.h>


int encrypt(void* buffer, int buff_len, char* iv, char* key, int key_len){
  MCRYPT td = mcrypt_module_open("twofish", 0, "cfb", 0);
  int b_size = mcrypt_enc_get_block_size(td);
  //printf("%d\n", b_size);
  //if(buff_len % b_size != 0)
  //return 1;

  mcrypt_generic_init(td, key, key_len, iv);
  mcrypt_generic(td, buffer, buff_len);
  mcrypt_generic_deinit(td);
  mcrypt_module_close(td);

  return 0;

}

  
int decrypt(void* buffer, int buff_len, char* iv, char* key, int key_len){
  MCRYPT td = mcrypt_module_open("twofish", 0, "cfb", 0);
  int b_size = mcrypt_enc_get_block_size(td);
  //printf("%d\n", b_size);
  
  //if(buff_len % b_size != 0)
  //return 1;

  mcrypt_generic_init(td, key, key_len, iv);
  mdecrypt_generic(td, buffer, buff_len);
  mcrypt_generic_deinit(td);
  mcrypt_module_close(td);

  return 0;
}

/*
int main(){
  MCRYPT td, td2;
  char* plaintext = "t";
  char* iv = "AAAAAAAAAAAAAAAA";
  char* key = "0123456789abcdefg";
  int keysize = 17;
  char* buffer;
  int buffer_len = 17;

  buffer = calloc(1, buffer_len);
  strncpy(buffer, plaintext, buffer_len);

  printf("==C==\n");
  printf("plain:   %s\n", plaintext);
  encrypt(buffer, 1, iv, key, keysize);
  printf("cipher:   ");
  int i;
  for(i = 0; i < buffer_len; i++){
    putchar(buffer[i]);
  }
  //display(buffer, buffer_len);
  decrypt(buffer, 1, iv, key, keysize);
  printf("decrypt: %s\n", buffer);

  return 0;
}


*/
