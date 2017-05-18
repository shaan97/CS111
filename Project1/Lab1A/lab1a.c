//NAME: Shaan Mathur
//EMAIL: shaankaranmathur@gmail.com
//ID: 904606576

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

// Gets current attributes
void getattr(int fd, struct termios* attr){
  // Get old terminal attributes
  int err = tcgetattr(0, attr);
  if(err){
    fprintf(stderr, "Failed to receive terminal attributes.\n");
    exit(1);
  }
}

void setattr(int fd, struct termios* attr){
 // Now change the attributes
  int err = tcsetattr(0, TCSANOW, attr);
  if(err){
    fprintf(stderr, "Failed to update terminal attributes. Error #%d\n", err);
    exit(1);
  }
}

static int shell = 0;

int main(int argc, char** argv){
  static struct option l_options[] = {
    {"shell", no_argument, &shell, 1}
  };

  while(1){
    int i, c;
    i = 0;
    c = getopt_long(argc, argv, "", l_options, &i);
    if(c == -1)
      {
	break;
      }
    switch(c){
    case '?':
      {
	fprintf(stderr, "Incorrect option - not recognized\n");
	fprintf(stderr, "Correct usage: lab1a [--shell] ... \n");
	exit(1);
	break;
      }
    }
  }




  // Creates structure to hold terminal attributes
  struct termios old_attr, new_attr;

  getattr(0, &old_attr);
  // Use second copy to modify the attributes
  getattr(0, &new_attr);
  new_attr.c_iflag = ISTRIP;
  new_attr.c_oflag = 0;
  new_attr.c_lflag = 0;

  setattr(0, &new_attr);

  if(!shell){
    
  
    char buffer[1024];
    long size;
    int complete = 0;
    while( (size = read(0, buffer, 1024)) >= 0){
      int i = 0;
      for(i = 0; i < size; i++){
	if(buffer[i] == '\004'){
	  complete = 1;
	  break;
	}

	if(buffer[i] == '\r' || buffer[i] == '\n'){
	  write(1, "\r\n", 2);
	}else{
	  write(1, buffer + i, 1);
	}
      }

      if(complete == 1)
	break;
    }

    setattr(0, &old_attr);

    return 0;
    
  }else{
    // shell option specified
    pid_t pid;
    int childread[2], childwrite[2];

    if(pipe(childread) == -1){
      fprintf(stderr, "Pipe Failure.\n");
      exit(1);
    }
    if(pipe(childwrite) == -1){
      fprintf(stderr, "Pipe Failure.\n");
      exit(1);
    }
    
    pid = fork();
    if(pid == -1){
      fprintf(stderr, "Forking process failure.\n");
      exit(1);
    }
    if(pid == 0){
      // Child process
      close(childread[1]);
      close(childwrite[0]);
      if( dup2(childwrite[1], 1) == -1){
	fprintf(stderr, "Failed to form pipe.\n");
	exit(1);
      }
      if(dup2(childread[0],0) == -1){
	fprintf(stderr, "Failed to form pipe.\n");
	exit(1);
      }
      if(dup2(childwrite[1],2) == -1){
	fprintf(stderr, "Failed to form pipe.\n");
	exit(1);
      }

      execv("/bin/bash", NULL);
      
    }else{
      // Parent process
      close(childread[0]);
      close(childwrite[1]);
      
      char buffer[1024];
      ssize_t size;
      int i, closed_r = 0, closed_w = 0;

      
      
      while(1){
	struct pollfd fds[2];
	fds[0].fd = 0;
	fds[1].fd = childwrite[0];
	
	fds[0].events = POLLIN | POLLERR | POLLHUP;
	fds[1].events = POLLIN | POLLERR | POLLHUP;
	
	poll(fds, 2, 0);
	if(fds[0].revents == POLLIN && closed_r == 0){

	  size = read(0, buffer, 1024);
	  for(i = 0; i < size; i++){
	    if(buffer[i] == 0x03){
	      kill(pid, SIGINT);
	      break;
	    }
	    else if(buffer[i] == 0x04){
	      close(childread[1]);
	      closed_r = 1;
	      break;
	    }
	    else if(buffer[i] == '\r' || buffer[i] == '\n'){
	      write(1, "\r\n", 2);
	      write(childread[1], "\n", 1);
	    }
	    else{
	      write(1, buffer + i, 1);
	      write(childread[1], buffer + i, 1);
	    }
	  }
	}
	if((fds[1].revents & POLLIN) != 0){
	  size = read(childwrite[0], buffer, 1024);
	  int i = 0, exit = 0;
	  for(i = 0; i < size; i++){
	    if(buffer[i] == '\n')
	      write(1, "\r\n", 2);
	    else if(buffer[i] == SIGPIPE){
	      exit = 1;
	      break;
	    }
	    else
	      write(1, buffer + i, 1);
	  }
	  if(exit == 1)
	    break;
	}else if((fds[1].revents & (POLLERR | POLLHUP)) != 0){
	  
	  break;
	}

      }
      if(closed_r == 0)
	close(childread[1]);
      if(closed_w == 0)
	close(childwrite[0]);
      int status;
      pid_t p = waitpid(0, &status, 0);
      fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WEXITSTATUS(status), WTERMSIG(status));
      setattr(0, &old_attr);
    }
    
  }
  
  
  
  return 0;
}
