//NAME: GARVIT PUGALIA
//EMAIL: garvitpugalia@gmail.com
//ID: 504628127

#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <termios.h>
#include <netdb.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mcrypt.h>

struct termios old_termios_p;
int keysize;
void resettermios();

char* get_key(char* filename)
{

  struct stat st_key;
  int key_fd = open(filename, O_RDONLY);
  if(fstat(key_fd, &st_key) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  char* key = (char*) malloc(st_key.st_size * sizeof(char));
  if (read(key_fd, key, st_key.st_size) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  keysize = st_key.st_size;
  return key;
}

int encrypt (void *buffer, int buffer_length, char* IV, char* key, int key_length)
{
  MCRYPT TD = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (TD == MCRYPT_FAILED)
    {
      fprintf(stderr, "The encryption module couldn't be opened");
      resettermios();
      exit(1);
    }
  if (mcrypt_generic_init(TD, key, key_length, IV) < 0)
    {
      fprintf(stderr, "The encryption couldn't be initialized");
      resettermios();
      exit(1);
    }
  if (key_length > mcrypt_enc_get_key_size(TD))
    { 
      fprintf(stderr, "Key size is too big for the algorithm");
      resettermios();
      exit(1);
    }
  if (mcrypt_generic(TD, buffer, buffer_length) != 0)
    {
      fprintf(stderr, "Encryption failed");
      resettermios();
      exit(1);
    }
  if (mcrypt_generic_deinit(TD) < 0)
    {
      fprintf(stderr, "Deinitialization of encryption alg failed");
      resettermios();
      exit(1);
    }
  mcrypt_module_close(TD);
  return 0;
}

int decrypt (void *buffer, int buffer_length, char* IV, char* key, int key_length)
{
  MCRYPT TD = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (TD == MCRYPT_FAILED)
    {
      fprintf(stderr, "The encryption module couldn't be opened");
      resettermios();
      exit(1);
    }
  if (mcrypt_generic_init(TD, key, key_length, IV) < 0)
    {
      fprintf(stderr, "The encryption couldn't be initialized");
      resettermios();
      exit(1);
    }
  if (key_length > mcrypt_enc_get_key_size(TD))
    {
      fprintf(stderr, "Key size is too big for the algorithm");
      resettermios();
      exit(1);
    }
  if (mdecrypt_generic(TD, buffer, buffer_length) != 0)
    {
      fprintf(stderr, "Encryption failed");
      resettermios();
      exit(1);
    }
  if (mcrypt_generic_deinit(TD) < 0)
    {
      fprintf(stderr, "Deinitialization of encryption alg failed");
      resettermios();
      exit(1);
    }
  mcrypt_module_close(TD);
  return 0;
}

void settermios()
{
  if (tcgetattr(0, &old_termios_p) < 0)
    fprintf(stderr, strerror(errno));
  
  //Making changes to the current terminal mode
  struct termios new_termios_p = old_termios_p;
  new_termios_p.c_iflag = ISTRIP;
  new_termios_p.c_oflag = 0;
  new_termios_p.c_lflag = 0;
  if (tcsetattr(0,TCSANOW,&new_termios_p) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
}

void resettermios()
{
  if (tcsetattr(0,TCSANOW,&old_termios_p) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
}

void usage()
{
  fprintf(stderr, "Usage statement\n");
}

int main(int argc, char* argv[])
{
  //OPTION PARSING
  int c, nport;
  int log_flag = 0;
  int encrypt_flag = 0;
  int port_flag = 0;
  char* log_file;
  char* encrypt_file;
  
  struct option long_opts[] = {
    {"port", required_argument, NULL, 'p'},
    {"log", required_argument, NULL, 'l'},
    {"encrypt", required_argument, NULL, 'e'},
    {0,0,0,0}
  };

  while ((c= getopt_long(argc, argv, "p:l:e:", long_opts, NULL)) != -1){
    switch(c) {
    case 'p':
      port_flag = 1;
      nport = atoi(optarg);
      break;
    case 'l':
      log_flag = 1;
      log_file = optarg;
      break;
    case 'e':
      encrypt_flag = 1;
      encrypt_file = optarg;
      break;
    case ':':
      fprintf(stderr, "An argument is required with all the options\n");
      usage();
      exit(1);
      break;
    case '?':
      fprintf(stderr, "The option is invalid\n");
      usage();
      exit(1);
      break;
    }
  }

  if (port_flag != 1)
    {
      fprintf(stderr, "The port number has to be specified using the --port option\n");
      exit(1);
    }

  //fprintf(stderr, encrypt_file);
  int log_fd;
  int encrypt_fd;
  if (log_flag == 1)
    {
      if ((log_fd = open(log_file, O_RDWR|O_CREAT|O_TRUNC)) < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}  
      chmod(log_file, S_IRUSR);
    }

  //if (encrypt_flag == 1)
  //  {
  //    if ((encrypt_fd = open(encrypt_file, O_RDONLY) ) < 0)
  //	{
  	  //	  fprintf(stderr, strerror(errno));
  //	  exit(1);
  //	}
  // }

  char* key;
  char* IV;
  int sockfd;
  struct sockaddr_in server_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  
  bzero((char*) &server_addr, sizeof(server_addr));
  struct hostent *server;
  server = gethostbyname("localhost");
  
  server_addr.sin_family = AF_INET;
  //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  bcopy( (char*) server->h_addr, (char*) &server_addr.sin_addr.s_addr, server->h_length);
  server_addr.sin_port = htons(nport);
  //fprintf(stderr, "1\n");
  if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  //fprintf(stderr, "2\n");
  
  if (encrypt_flag == 1)
    {
      key = get_key(encrypt_file);
      //      fprintf(stderr, key);
      //fprintf(stderr, keysize);
      IV = "AAAAAAAAAAAAAAAA";
    }
  
  settermios();
  char temp[1];
  char newline[2];
  newline[0] = 10;
  newline[1] = 13;

  char linefeed[1];
  linefeed[0] = 10;
  
  if (encrypt_flag == 1)
    {
      //encrypt(newline, 2, IV, key, keysize);
      encrypt(linefeed, 1, IV, key, keysize);
    }
  
  char log_string[2048];
  char buffer[1024];
  int numBytes, index, poll_r;

  struct pollfd fds[2];
  fds[0].fd = 0;
  fds[0].events = POLLIN;
  fds[1].fd = sockfd;
  fds[1].events = POLLIN | POLLHUP | POLLERR;
  int flg = 0;

  while (flg == 0)
    {
      poll_r = poll(fds, 2, 0);
      if (poll_r < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      else if (poll_r > 0)
	{
	  if ((fds[0].revents & POLLIN) != 0)
	    {
	      bzero(buffer, 1024);
	      numBytes = read(0,buffer,1024);
	      if (numBytes < 0)
		{
		  fprintf(stderr, strerror(errno));
		  exit(1);
		}
	      for (index = 0; index < numBytes; index++)
		{
		  if (buffer[index] == 13 || buffer[index] == 10)
		    {
		      write(sockfd, linefeed,1);
		      write(1,newline,2);
		    }
		  else
		    {
		      write(1,&buffer[index], 1);
		      if (encrypt_flag == 1)
			{
			  encrypt(&buffer[index], 1,IV,key,keysize);
			}
		      write(sockfd, &buffer[index],1);
		    }
		  //if (buffer[index] != 10 && buffer[index] != 13)
		  //  {
		  //    write(1,&buffer[index],1);
		  //    if (encrypt_flag == 1)
		  //			encrypt(&buffer[index], 1, IV, key, keysize);
		  // }
		  //else if (buffer[index] == 13 || buffer[index] == 10)
		  //  {
		  //    write(sockfd, linefeed,1);
		  //    write(1, newline, 2);
		  //  }
		  //if (buffer[index] 
		  //  write(sockfd, &buffer[index], 1);
		  //  //write(1,&buffer[index], 1);
		  
		  if (log_flag == 1)
		    {
		      bzero(log_string, 2048);
		      if (index == 0)
			{
			  snprintf(log_string, 2048, "SENT %d bytes: ", numBytes);
			  if (write(log_fd, log_string, strlen(log_string)) < 0)
			    {
			      fprintf(stderr, strerror(errno));
			      exit(1);
			    }
			}
		      write(log_fd, &buffer[index], 1);
		      if (index == numBytes - 1)
			write(log_fd, "\n", 1);
		    }
		}
	    }
	  if ((fds[1].revents & POLLIN) != 0)
	    {
	      bzero(buffer,1024);
	      numBytes = read(sockfd, buffer, 1024);
	      if (numBytes < 0)
		{
		  fprintf(stderr, strerror(errno));
		  exit(1);
		}
	      for (index = 0; index < numBytes; index++)
		{
		  if (log_flag == 1)
                    {
                      bzero(log_string, 2048);
                      if (index == 0)
                        {
                          snprintf(log_string, 2048, "RECEIVED %d bytes: ", numBytes);
                          write(log_fd, log_string, strlen(log_string));
                        }
                      write(log_fd, &buffer[index], 1);
                      if (index == numBytes - 1)
                        write(log_fd, "\n", 1);
                    }
		  
		  if (encrypt_flag == 1)
		    decrypt(&buffer[index], 1, IV, key, keysize);
		  write(1, &buffer[index], 1);
		}
	    }
	  else if ((fds[1].revents & (POLLERR | POLLHUP)) != 0)
	    {
	      resettermios();
	      exit(0);
	    }
	  if (recv(sockfd, temp, sizeof(temp), MSG_PEEK | MSG_DONTWAIT) == 0)
	    {
	      resettermios();
	      exit(0);
	    }
	}
    }
  
  /*int n;
  char buffer[256];
  bzero(buffer,256);
  fgets(buffer,255,stdin);
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0)
    fprintf(stderr, "Error writing to socket");
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
  if (n < 0)
  fprintf(stderr, "Error reading from socket");*/ //testcode
  
  resettermios(); 
  exit(0);
}
