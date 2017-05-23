#include <stdio.h>
#include "mraa/aio.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <mraa.h>
#include <getopt.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

const int B = 4275; // B value of the thermistor
const int R0 = 100000; // R0 = 100k

#define FAHR 0
#define CELS 1

double read_temperature(mraa_aio_context* temp, int mode){
  if(*temp == NULL){
    fprintf(stderr, "MRAA structure NULL.\n");
    exit(1);
  }

  int a = mraa_aio_read(*temp);
  double R = 1023.0/a-1.0;
  R = R0*R;

  double temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

  if(mode == FAHR){
    temperature = temperature * 1.5 + 32;

  }
  //float R = 1023.0/read - 1.0;
  //R *= R0;

  //float temperature = 1.0/(log(R/R0)/B + 1/298.15)-273.15;

  return temperature;

}

void init(mraa_aio_context* t_sensor, mraa_gpio_context * button){
  *t_sensor = mraa_aio_init(0);
  if(*t_sensor == NULL){
    fprintf(stderr, "Failed to initialize sensor.\n");
    exit(1);
  }

  *button = mraa_gpio_init(3);
  if(*button == NULL){
    fprintf(stderr, "Failed to initialize button.\n");
    exit(1);
  }

  mraa_gpio_dir(*button, MRAA_GPIO_IN);
}

void print_usage(){
  fprintf(stderr, "Usage: [--period=NUMBER] [--scale=[C or F]] [--log=FILENAME]\n");

}

void shutdown(int logfd){
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  printf("%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  if(logfd != -1)
    dprintf(logfd, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  
  exit(1);
}

void bad_input(){
  fprintf(stderr, "Bad input.\n");
  exit(1);
}

int main(int argc, char** argv){
  unsigned int seconds = 1;  // COMMAND LINE ARGUMENT
  int mode = FAHR;           // COMMAND LINE ARGUMENT
  int logfd = -1;
  static struct option l_options[] = {
    {"period", required_argument, NULL, 'p'},
    {"scale", required_argument, NULL, 's'},
    {"log", required_argument, NULL, 'l'},
    {0, 0, 0, 0}
  };

  
  char c;
  
  while( (c = getopt_long(argc, argv, "p:s:l:", l_options, NULL)) != -1 ){
    switch(c){
    case 'p':
      seconds = atoi(optarg);
      break;
    case 's':
      if(strlen(optarg) != 1){
	print_usage();
	exit(1);
      }
      char m = optarg[0];
      switch(m){
      case 'C':
	mode = CELS;
	break;
      case 'F':
	mode = FAHR;
	break;
      default:
	print_usage();
	exit(1);
      }
      break;
    case 'l':
      logfd = open(optarg, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR |   S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
      break;
    }
  }
  mraa_aio_context t_sensor;
  mraa_gpio_context button;
  init(&t_sensor, &button);

  time_t rawtime;
  struct tm* timeinfo;

  int running = 1;
  struct pollfd fd;
  fd.fd = 0;
  fd.events = POLLIN | POLLHUP | POLLERR;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  time_t prev;
  float tmp = read_temperature(&t_sensor, mode);

  // Run first initially to make sure at least one message is printed.
  if(tmp >= 10.0)
    printf("%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
  else
    printf("%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min,timeinfo->tm_sec, tmp);
  if(logfd != -1){
    if(tmp >= 10.0)
      dprintf(logfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
    else
      dprintf(logfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
  }
  prev = rawtime;
 
  while(1){
    if(mraa_gpio_read(button))
      shutdown(logfd);

    
    
    if(poll(&fd, 1, 0) < 0){
      fprintf(stderr, "Error #:%d Error Message:%s\n", errno, strerror(errno));
      exit(1);
    }

    

    if(fd.revents & POLLIN){
      // Put input into buffer
      int size = 0;
      int capacity = 1024;
      char * buff = (char *) malloc(sizeof(char) * capacity);
      do{
	int rc = read(fd.fd, (void *) buff, capacity - size);
	if(rc <= 0)
	  break;
	size += rc;
	
	if(size >= capacity)
	  buff = (char *) realloc(buff, (capacity *= 2));

	if(poll(&fd, 1, 0) < 0){
	  fprintf(stderr, "Error #:%d Error Message:%s\n", errno, strerror(errno));
	  exit(1);
	}      
      }while(fd.revents & POLLIN);
      
      buff = (char *) realloc(buff, size + 1);
      buff[size] = 0;      

      // Break input into token seperated by newline
      const char nl[2] = "\n";
      char * token = strtok(buff, nl);
      while(token != NULL){
	if(strcmp(token, "OFF") == 0){
	  if(logfd != -1)
	    dprintf(logfd, "OFF\n");

	  shutdown(logfd);
	}else if(strcmp(token, "STOP") == 0){
	  running = 0;
	}else if(strcmp(token, "START") == 0){
	  running = 1;
	}else if(strncmp(token, "SCALE=", 6) == 0 && strlen(token) == 7){
	  if(token[6] == 'F')
	    mode = FAHR;
	  else if(token[6] == 'C')
	    mode = CELS;
	  else{
	    bad_input();
	  }
	}else if(strncmp(token, "PERIOD=", 7) == 0 && strlen(token) > 7){
	  seconds = atoi(token + 7);
	  if(seconds <= 0)
	    bad_input();
	}else{
	  bad_input();
	}
	
	if(logfd != -1)
	  dprintf(logfd, "%s\n", token);
	token = strtok(NULL, nl);
      }
    }


    // Update time if needed
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    tmp = read_temperature(&t_sensor, mode);
    
    if(running && rawtime - prev >= seconds){
      	if(tmp >= 10.0)
	    printf("%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
	else
	    printf("%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min,timeinfo->tm_sec, tmp);
      if(logfd != -1){
      	if(tmp >= 10.0)
	  dprintf(logfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
      	else
	  dprintf(logfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
      }
      prev = rawtime;
    }
  }
  
  return 0;
}
