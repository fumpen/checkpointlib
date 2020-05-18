#include "../../lib/checkpoint.hpp"
#include <stdlib.h> 
#include <iostream>
#include <unistd.h>
#include <time.h>

struct pBuff{
  unsigned int count;
  int prime[100];
};

void gen_prime(void* raw_data, Checkpoint* cpt){
  pBuff* lBuff = (pBuff*) raw_data;
  srand(time(0));
  int tmp, halfway;
  unsigned int i;
  bool flag;
  while(100 > lBuff->count){
    tmp = rand();
    flag = true;
    halfway = tmp / 2;
    for(i = 2; i <= halfway; i++){
      if((tmp % i) == 0){
	flag = false;
	break;
      }
    }
    if(flag){
      lBuff->prime[lBuff->count] = tmp;
      lBuff->count += 1;
      if((lBuff->count % 10) == 0){
	std::cout << "@ checkpoint\n";
	cpt->check();
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
  
  cptOrg.initCheckpointOrganizer(tot_num_threads, restart, sizeof(pBuff) * 2 + 100);

  void (*foo)(void *, Checkpoint*);
  foo = &gen_prime;
  
  pBuff* var1 = (pBuff*)cptOrg.check_alloc(sizeof(pBuff));
  var1->count = 0;
  pBuff* var2 = (pBuff*)cptOrg.check_alloc(sizeof(pBuff));
  var2->count = 0;
  cptOrg.startThread(foo, var1);
  cptOrg.startThread(foo, var2);
  
  cptOrg.joinAll();
  std::cout << "Process finish\n";
  return 0;
}
