/* NAME: SHAAN MATHUR
   UID: 904606576
   EMAIL: SHAANKARANMATHUR@GMAIL.COM
*/


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

void segfault(){
  char * pointer = NULL;
  *pointer = 1;
}
void sighandler(int signum){
  fprintf(stderr, "Caught Segmentation Fault. Terminating program.\n");
  exit(4);
}

int main(int argc, char ** argv){
  int input = 0;
  int output = 1;

  struct option longopts[] = {
    { "input", required_argument, NULL, 'i' },
    { "output", required_argument, NULL, 'o' },
    { "segfault", no_argument, NULL, 's'},
    { "catch", no_argument, NULL, 'c'},
    { 0, 0, 0, 0}
  };

  int c, temp;
  int make_seg = 0;
  while( (c = getopt_long(argc, argv, "i:o:sc", longopts, NULL)) != -1){
    switch(c){
    case 'i':
      input = open(optarg, O_RDONLY);
      if(input == -1){
	fprintf(stderr, "Errno: %d, Error Message:\"%s\"\n", errno, strerror(errno));
	exit(2);
      }

      close(0);
      temp = dup(input); // input will be assigned 0
      close(input);
      input = temp;
      
      break;
    case 'o':
      output = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if(output == -1){
	fprintf(stderr, "Errno: %d, Error Message:\"%s\"\n", errno, strerror(errno));
	exit(3);
      }

      close(1);
      temp = dup(output); // output will be assigned 1
      close(output);
      output = temp;
      
      break;
    case 's':
      make_seg = 1;
      break;
    case 'c':
      signal(SIGSEGV, sighandler);
      break;
    default:
      fprintf(stderr, "usage: lab0 [--input=filename] [--output=filename] [--segfault] [--catch]\n");
      exit(1);
      break;
    }
    
  }

  if(make_seg == 1){
    segfault();
  }



  char buffer[BUFF_SIZE];
  ssize_t size;
  while( (size = read(input, buffer, BUFF_SIZE)) > 0){
    

    ssize_t err = write(output, buffer, size);

    if(err == -1){
      fprintf(stderr, "Errno: %d, Error Message:\"%s\"\n", errno, strerror(errno));
      exit(1);
    }
  }
  
  if(size == -1){
      fprintf(stderr, "Errno: %d, Error Message:\"%s\"\n", errno, strerror(errno));
      exit(1);
  }

  close(input);
  close(output);

  return 0;
}
