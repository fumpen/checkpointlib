#include "../../lib/checkpoint.hpp"
#include <stdlib.h> 
#include <iostream>
#include <unistd.h>


void fail_halfway(void* raw_data, Checkpoint* cpt){
  std::thread::id this_id = std::this_thread::get_id();
  unsigned int mean_time_to_failure = 5;
  unsigned int count_to_mttf = 0;
  int* data = (int*)raw_data;
  for(int* i=&data[0]; (*i)<10000; (*i)++){
    data[1] += 2;
    if((*i % 1000) == 0) {
      std::cout << "thread_id: " << this_id
		<< " - iteration: "
		<< *i << "\n";
      cpt->check();
      count_to_mttf += 1;
      if(mean_time_to_failure <= count_to_mttf){
	_exit(10);
      }
    }
  }
}


int main(int argc, char** argv){
  if(argc != 2) return 0;
  bool restart = 0;
  if(atoi(argv[1])) restart = 1; 
  unsigned int tot_num_threads = 2;
  CheckpointOrganizer cptOrg("tmp1.chpt", "tmp2.chpt");
  cptOrg.initCheckpointOrganizer(tot_num_threads, (bool)restart, 1000);

  void (*foo)(void *, Checkpoint*);
  foo = &fail_halfway;
  
  int* var1 = (int*)cptOrg.check_alloc(2*sizeof(int));
  var1[0] = 0;
  var1[1] = 0;
  int* var2 = (int*)cptOrg.check_alloc(2*sizeof(int));
  var2[0] = 0;
  var2[1] = 0;
  cptOrg.startThread(foo, var1);
  cptOrg.startThread(foo, var2);
  
  cptOrg.joinAll();
  std::cout << "Process finished\n";
  return 0;
}
