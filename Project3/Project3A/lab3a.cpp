#include <iostream>
#include <fstream>
#include <string>
#include "lab3a.h"

using namespace std;

static ofstream file;
static bool init = false;

void Print(std::string message){
  if (init == false)
    {
      std::ofstream file ("lab3a.csv", std::ofstream::trunc);
      bool init = true;
    }
  file << message << endl;
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
