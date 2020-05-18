#include "../../lib/checkpoint.hpp"
#include <iostream>
#include <fstream>
#include <string>

std::string g_cpt1 = "cpt1.tmp";
std::string g_cpt2 = "cpt2.tmp";

struct arr_data {
  int* arr;
  std::size_t size;
};

void invert_array_in_place(void* data){
  arr_data local_data = *((arr_data*) data);
  std::size_t half_size = local_data.size / 2UL;
  int tmp;
  int* arr = local_data.arr;
  for(std::size_t i=0; i < half_size; i++){
    tmp = arr[i];
    arr[i] = arr[local_data.size - i - 1];
    arr[local_data.size - i - 1] = tmp;
  }
}



struct checkpoint_arr_data {
  bool load_first;
  int* arr;
  std::size_t size;
  std::size_t* iterator;
};


void checkpoint_invert_array_in_place(void* data, Checkpoint* cpt){
  checkpoint_arr_data local_data = *((checkpoint_arr_data*) data);
  int* arr = local_data.arr;
  std::size_t size = local_data.size;
  bool load_first = local_data.load_first;
  std::size_t* i = local_data.iterator;
  std::size_t half_size = size / 2UL;
  int tmp;
  if(load_first){
    cpt->check();
    /* uncomment to resume computation after restart
    while(*i < half_size){
      tmp = arr[*i];
      arr[*i] = arr[size - *i - 1];
      arr[size - *i - 1] = tmp;
      (*i)++;
      }
    */
  }else{
    for(*i=0; *i < half_size; (*i)++){
      if((half_size / 2) == *i){
	cpt->check();
	break;
      }
      tmp = arr[*i];
      arr[*i] = arr[size - *i - 1];
      arr[size - *i - 1] = tmp;
    }
  }
  
}



void populate_array(int* arr, std::size_t size, int seed){
  srand(seed);
  for(std::size_t i=0; i < size; i++) arr[i] = rand();
}


int main(){
  const unsigned int number_of_tests = 10;
  const unsigned int iterations_per_test = 100;
  const unsigned int threadTotal = 4;
  const int seed = 5;
  std::size_t ints_in_array_at_iteration[number_of_tests] = {100000, 200000, 300000, 400000,
							     500000, 600000, 700000, 800000,
							     900000, 1000000};
  std::size_t res_ctrl[number_of_tests];
  std::size_t res_checkpoint[number_of_tests];

  std::thread threads_ctrl[threadTotal];
  
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  CheckpointOrganizer* cptOrg;
  std::size_t array_length;
  std::size_t allocation_size_required;
  void* tmp_alloc;
  checkpoint_arr_data func_init_args[threadTotal];
  arr_data _func_init_args[threadTotal];
  for(unsigned int i=0; i<number_of_tests; i++){
    std::cout << "Now running test " << i << " of " << number_of_tests << "\n";
    array_length = ints_in_array_at_iteration[i];
    allocation_size_required = threadTotal *
      (20 + (array_length * sizeof(int)) + sizeof(std::size_t));
    cptOrg = new CheckpointOrganizer(g_cpt1, g_cpt2);
    cptOrg->initCheckpointOrganizer(threadTotal, false, allocation_size_required);
    
    for(unsigned int q=0; q<threadTotal; q++){
      tmp_alloc = cptOrg->check_alloc(array_length * sizeof(int));
      populate_array((int*) tmp_alloc, array_length, seed);
      func_init_args[q].arr = (int*)tmp_alloc;
      func_init_args[q].size = array_length;
      func_init_args[q].load_first = false;
      func_init_args[q].iterator = (std::size_t*) cptOrg->check_alloc(sizeof(std::size_t));
    }

    for(unsigned int q=0; q<threadTotal; q++){
      cptOrg->startThread(checkpoint_invert_array_in_place, &func_init_args[q]);
    }
    cptOrg->joinAll();
    
    delete cptOrg;
    cptOrg = new CheckpointOrganizer(g_cpt1, g_cpt2);
    cptOrg->initCheckpointOrganizer(threadTotal, true, allocation_size_required);
    for(unsigned int q=0; q<threadTotal; q++){
      tmp_alloc = cptOrg->check_alloc(array_length * sizeof(int));
      populate_array((int*) tmp_alloc, array_length, seed);
      func_init_args[q].arr = (int*)tmp_alloc;
      func_init_args[q].size = array_length;
      func_init_args[q].load_first = true;
      func_init_args[q].iterator = (std::size_t*) cptOrg->check_alloc(sizeof(std::size_t));
    }

    res_checkpoint[i] = 0;
    for(unsigned int x=0; x < iterations_per_test; x++){
      if((x % 100) == 0) std::cout << "Running test " << x << " of " << iterations_per_test << "\n"; 
      t1 = std::chrono::high_resolution_clock::now();
      for(unsigned int q=0; q<threadTotal; q++){
	cptOrg->startThread(checkpoint_invert_array_in_place, &func_init_args[q]);
      }
      cptOrg->joinAll();
      t2 = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
      res_checkpoint[i] += duration;
    }
    res_checkpoint[i] = res_checkpoint[i] / iterations_per_test;
    delete cptOrg;


    /*
    for(unsigned int q=0; q<threadTotal; q++){
      tmp_alloc = malloc(array_length * sizeof(int));
      populate_array((int*) tmp_alloc, array_length, seed);
      _func_init_args[q].arr = (int*)tmp_alloc;
      _func_init_args[q].size = array_length;
    }
    
    res_ctrl[i] = 0;
    for(unsigned int x=0; x < iterations_per_test; x++){
      t1 = std::chrono::high_resolution_clock::now();
      for(unsigned int q=0; q<threadTotal; q++) threads_ctrl[q] = std::thread(invert_array_in_place, &_func_init_args[q]);
      for(unsigned int q=0; q<threadTotal; q++) threads_ctrl[q].join();
      t2 = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
      res_ctrl[i] += duration;
    }
    res_ctrl[i] = res_ctrl[i] / iterations_per_test;
    
    for(unsigned int q=0; q<threadTotal; q++) free(_func_init_args[q].arr);
    */
  }

  for(int t=0; t<10;t++) std::cout << ", " << res_checkpoint[t];
  std::cout << "\n\n";
  
  //for(int t=0; t<10;t++) std::cout << ", " << res_ctrl[t];
  //std::cout << "\n\n";
  return 0;
}



/*
	for(unsigned int q=0; q<array_length; q++) std::cout << ", " << func_init_args[1].arr[q];
	std::cout << ", " << "\n\n";
      */
