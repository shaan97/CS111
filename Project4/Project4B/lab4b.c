#include <stdio.h>
#include "mraa/aio.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <mraa.h>
#include <getopt.h>
#include <stdlib.h>

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

void init(mraa_aio_context* t_sensor){
  *t_sensor = mraa_aio_init(0);
  if(*t_sensor == NULL){
    fprintf(stderr, "Failed to initialize sensor.\n");
    exit(1);
  }
}

void print_usage(){
  fprintf(stderr, "Usage: [--period=NUMBER] [--scale=[C or F]] [--log=FILENAME]\n");

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
  init(&t_sensor);

  time_t rawtime;
  struct tm* timeinfo;

  
  
  while(1){
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    float tmp = read_temperature(&t_sensor, mode);
    printf("%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);


    if(logfd != -1)
      dprintf(logfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
    
    sleep(seconds);
  }
  
  return 0;
}
