#include <iostream>
#include <fstream>
#include <string>

using namespace std;

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
