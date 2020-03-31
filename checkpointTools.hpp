#ifndef CHECKPOINTTOOLS_HPP
#define CHECKPOINTTOOLS_HPP
#include <iostream>
#include <cstring> //for strerror
#include <errno.h> // for errorno...

//for ftruncate
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h> // for FILE*, fseek, fopen(),...


// I'm too lazy to type this shit out every time I print something...
void printWarning(std::string warning){
  std::cout << "Checkpoint-WARNING: " + warning + "\n";
}
void printWarning(std::string warning, int errorNumber){
  std::cout << "Checkpoint-WARNING: " + warning + "\n"
	    << "with errno: " << errorNumber << " and strerror(errno): "
	    << std::strerror(errorNumber) << "\n";
}
void printInfo(std::string info){
  std::cout << "Checkpoint-INFO: " + info + "\n";
}

std::size_t ret_filesize_bytes(FILE *open_file){
  if(fseek(open_file, 0, SEEK_SET) != 0){
    printWarning("fseek() could not set pointer to start of file", errno);
  }
  long int l_start = ftell(open_file);
  
  if(fseek(open_file, 0, SEEK_END) != 0){
    printWarning("fseek() could not set pointer to end of file", errno);
  }
  
  long int l_stop = ftell(open_file);
  if((l_stop < 0) || (l_start < 0)){
    printWarning("ftell() returned negative", errno);
  }
  std::size_t fileSize = l_stop - l_start;
  return fileSize;
}

// Wrapper for ftruncate, resizing file (in bytes)
int adjust_file_size(FILE *open_file, unsigned int size_in_bytes){
  int res = ftruncate(fileno(open_file), size_in_bytes);
  if(res < 0){
    printWarning("ftruncate() error", errno);
  }
  return res;
}


bool does_file_exist(std::string fileName){
  if(FILE *file = fopen(fileName.c_str(), "r")){
    fclose(file);
    return true;
  }else{
    return false;
  } 
}



#endif //CHECKPOINTTOOLS_HPP
