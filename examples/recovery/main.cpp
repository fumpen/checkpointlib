#include <stdio.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(){

  std::string cmd_no_restart = "./testApp 0";
  std::string cmd_with_restart = "./testApp 1";

  FILE* recoverable_process = popen(cmd_no_restart.c_str(), "r");
  bool continue_execution = true;
  int status;
  int exit_status;
  while(continue_execution){
    wait(&status);

    if(WIFEXITED(status)){ 
      exit_status = WEXITSTATUS(status);         
      if(exit_status){ // if exit status is non-zero => error occured
	std::cout << "restart was called \n";
	pclose(recoverable_process);
	recoverable_process = popen(cmd_with_restart.c_str(), "r");
      }else{
	continue_execution = false;
      } 
    }
  }
  
  pclose(recoverable_process);
  std::cout << "sucess\n";
  return 0;

}
