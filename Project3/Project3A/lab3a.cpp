#include <iostream>
#include <fstream>
#include <string>
#include "lab3a.h"
#include <errno.h>

using namespace std;




void Print(std::string message){
  static ofstream file;
  static bool init = false;
  if (init == false)
    {
      std::ofstream file ("lab3a.csv", std::ofstream::trunc);
      bool init = true;
    }
  file << message << endl;
}

ssize_t Pread(int fd, void *buf, size_t count, off_t offset)
{
  if (pread(fd,buf,count,offset) < 0)
    {
      fprintf(stderr, strerror(errno));
      exit(1);
    }
}

string get_file_name(int argc, char *argv[]){
  if(argc != 2){
    fprintf(stderr, "Illegal number of arguments.\n");
    exit(1);
  }

  return string(argv[1]);
}

int main(int argc, char *argv[])
{
  string file = get_file_name(argc, argv);
  
  
  return 0;
}
