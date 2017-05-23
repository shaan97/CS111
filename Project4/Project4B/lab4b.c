#include <stdio.h>
#include "mraa/aio.h"
#include <math.h>
#include <unistd.h>
#include <time.h>

const int B = 4275; // B value of the thermistor
const int R0 = 100000; // R0 = 100k

float read_temperature(mraa_aio_context* temp){
  if(*temp == NULL){
    fprintf(stderr, "MRAA structure NULL.\n");
    exit(1);
  }

  int a = mraa_aio_read(*temp);
  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  //float R = 1023.0/read - 1.0;
  //R *= R0;

  //float temperature = 1.0/(log(R/R0)/B + 1/298.15)-273.15;

  return temperature;

}

void init(mraa_aio_context* t_sensor){
  *t_sensor = mraa_aio_init(0);
  if(t_sensor == NULL){
    fprintf(stderr, "Failed to initialize sensor.\n");
    exit(1);
  }
}

int main(){
  unsigned int seconds = 1;  // COMMAND LINE ARGUMENT

  mraa_aio_context t_sensor;
  init(&t_sensor);

  time_t rawtime;
  struct tm* timeinfo;

  
  while(1){
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    printf("%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, read_temperature(&t_sensor));
    sleep(seconds);
  }
  
  return 0;
}