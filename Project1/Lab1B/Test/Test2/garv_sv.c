//NAME: GARVIT PUGALIA
//EMAIL: garvitpugalia@gmail.com
//ID: 504628127

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mcrypt.h>

//pid_t child_process;
//void sighandler(int signum)
//{ 
//fprintf(stderr, "ENTERS SIGNAL HANDLER");
// int stat;
// if (waitpid(child_process, &stat, 0) < 0)
//    {
//      fprintf(stderr, strerror(errno));
//    exit(1);
//    }
//  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n",WEXITSTATUS(stat), WTERMSIG(stat));
//  exit(1);
//}

int keysize;
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

void usage()
{
  fprintf(stderr, "Usage message");
}

int encrypt (void *buffer, int buffer_length, char* IV, char* key, int key_length)
{
  MCRYPT TD = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (TD == MCRYPT_FAILED)
    {
      fprintf(stderr, "The encryption module couldn't be opened");
      exit(1);
    }
  if (mcrypt_generic_init(TD, key, key_length, IV) < 0)
    {
      fprintf(stderr, "The encryption couldn't be initialized");
      exit(1);
    }
  if (key_length > mcrypt_enc_get_key_size(TD))
    {
      fprintf(stderr, "Key size is too big for the algorithm");
      exit(1);
    }
  if (mcrypt_generic(TD, buffer, buffer_length) != 0)
    {
      fprintf(stderr, "Encryption failed");
      exit(1);
    }
  if (mcrypt_generic_deinit(TD) < 0)
    {
      fprintf(stderr, "Deinitialization of encryption alg failed");
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
      exit(1);
    }
  if (mcrypt_generic_init(TD, key, key_length, IV) < 0)
    {
      fprintf(stderr, "The encryption couldn't be initialized");
      exit(1);
    }
  if (key_length > mcrypt_enc_get_key_size(TD))
    {
      fprintf(stderr, "Key size is too big for the algorithm");
      exit(1);
    }
  if (mdecrypt_generic(TD, buffer, buffer_length) != 0)
    {
      fprintf(stderr, "Encryption failed");
      exit(1);
    }
  if (mcrypt_generic_deinit(TD) < 0)
    {
      fprintf(stderr, "Deinitialization of encryption alg failed");
      exit(1);
    }
  mcrypt_module_close(TD);
  return 0;
}

int main (int argc, char * argv[])
{
  //OPTION PARSING
  int c, nport;
  int encrypt_flag = 0;
  int port_flag = 0;
  char* encrypt_file;
  
  struct option long_opts[] = {
    {"port", required_argument, NULL, 'p'},
    {"encrypt", required_argument, NULL, 'e'},
    {0,0,0,0}
  };

  while ((c= getopt_long(argc, argv, "p:e:", long_opts, NULL)) != -1){
    switch(c) {
    case 'p':
      port_flag = 1;
      nport = atoi(optarg);
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

  int server_sockfd, client_sockfd;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  
  //unlink("server_socket"); //removes any old socket
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_sockfd < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }

  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(nport);
  
  if (bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  
  listen(server_sockfd, 5);
  int client_length = sizeof(client_addr);
  //fprintf(stderr, "THERE");
  client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_length);
  //fprintf(stderr, "THERE");
  
  if (client_sockfd < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }

  char* key;
  char* IV = "AAAAAAAAAAAAAAAA";
  
  if (encrypt_flag == 1)
    {
      key = get_key(encrypt_file);
      //keysize = sizeof(key);
    }
  
  struct pollfd fds[2];
  
  int pipetoshell[2];
  int pipefromshell[2];
  
  if (pipe(pipetoshell) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }

  if (pipe(pipefromshell) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
  
  char* shell[2];
  shell[0] = "/bin/bash";
  shell[1] = 0;
  pid_t child_process;
  int poll_r;
  int index, numBytes;
  char buffer[1024];
  char newline[2];
  newline[0] = 10;
  newline[1] = 13;
  
  if (encrypt_flag == 1)
    {
      encrypt(&newline[0], 1, IV, key, keysize);
      encrypt(&newline[1], 1, IV, key, keysize);
    }
  
  child_process = fork();
  if (child_process < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }

  else if (child_process == 0)
    {
      close(pipefromshell[0]);
      close(pipetoshell[1]);
      if (dup2(pipetoshell[0], 0) < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      close(pipetoshell[0]);
      if (dup2(pipefromshell[1], 1) < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      if (dup2(pipefromshell[1], 2) < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
      close(pipefromshell[1]);

      if (execvp(shell[0], shell) < 0)
	{
	  fprintf(stderr, strerror(errno));
	  exit(1);
	}
    }
  else
    {
      close(pipefromshell[1]);
      close(pipetoshell[0]);
      fds[0].fd = client_sockfd;
      fds[0].events = POLLIN;
      fds[1].fd = pipefromshell[0];
      fds[1].events = POLLIN | POLLHUP | POLLERR;
      char temp[1];
      int stat;
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
		  bzero(buffer,1024);
		  numBytes = read(client_sockfd, buffer, 1024);
		  if (numBytes < 0)
		    {
		      fprintf(stderr, strerror(errno));
		      //kill(child_process, SIGTERM);
		      exit(1);
		    }
		  for (index = 0; index < numBytes; index++)
		    {
		      if (encrypt_flag == 1)
			{
			  decrypt(&buffer[index], 1, IV, key, keysize);
			}
		      if (buffer[index] == 4)
			close(pipetoshell[1]);
		      else if (buffer[index] == 3)
			kill(child_process, SIGINT);
		      else
			write(pipetoshell[1], &buffer[index], 1);
		    }
		}
	      if ((fds[1].revents & POLLIN) != 0)
		{
		  int nBytes;
		  bzero(buffer,1024);
		  numBytes = read(pipefromshell[0], buffer, 1024);
		  if (numBytes < 0)
		    {
		      fprintf(stderr, strerror(errno));
		      exit(0);
		    }
		  nBytes = numBytes;
		  for (index = 0; index < numBytes; index++)
		    {
		      if (buffer[index] == 10)
			{
			  write(client_sockfd, newline,sizeof(newline));
			}
		      else
			{
			  if (encrypt_flag == 1)
			    encrypt(&buffer[index], 1, IV, key,keysize);
			  write(client_sockfd, &buffer[index], 1);
			}
		      //if (encrypt_flag == 1 && buffer[index] != 10)
		      //		encrypt(&buffer[index], 1, IV, key, keysize);
		      //else if (buffer[index] == 10)
		      //		{
		      //	  write(client_sockfd, newline, 2);
		      ///	  nBytes = nBytes + 1;
		      //	}
		      // se
		      //	write(client_sockfd, &buffer[index], 1);
		    }
		}
	      else if ((fds[1].revents & (POLLHUP | POLLERR)) != 0)
		{
		  poll_r = waitpid(child_process, &stat, 0);
		  if (poll_r < 0)
		    {
		      fprintf(stderr, strerror(errno));
		      exit(1);
		    }
		  flg = 1;
		  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n",WEXITSTATUS(stat), WTERMSIG(stat));
		  close(pipefromshell[0]);
		  if (close(client_sockfd) < 0)
		    {
		      fprintf(stderr,strerror(errno));
		      exit(1);
		    }
		  exit(0);
		}
	      if (recv(client_sockfd, temp, sizeof(temp), MSG_PEEK | MSG_DONTWAIT) == 0)
		{
		  //fprintf(stderr, "ENTERS RECV");
		  //signal(SIGTERM, sighandler);
		  kill(child_process, SIGTERM);
		  //exit(0);
		}     
	    }
	}
    }
  
  /*int n;
  char buffer[256];
  bzero(buffer, 256);
  n = read(client_sockfd, buffer, 255);
  if (n < 0)
    fprintf(stderr, "Error reading from socket");
  printf("Here is the message: %s\n", buffer);
  n = write(client_sockfd, "I got your message", 18);
  if (n < 0)
  fprintf(stderr, "Error writing to socket");*/ //test code
  exit(0);
}
