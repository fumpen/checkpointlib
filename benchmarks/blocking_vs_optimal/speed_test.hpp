#ifndef CHECKPOINTTESTS_HPP
#define CHECKPOINTTESTS_HPP

#include <stdio.h>
#include "../../lib/checkpoint.hpp"
#include "../alt_lib/checkpoint2.hpp"
#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <time.h>

struct testDat {
 unsigned long int iterations;
 unsigned long int index;
 char* mem;
 };

void simple_test_write_10_checkpoints_normal(void* data, Checkpoint* check){
  testDat my_dat = *((testDat*) data);
  char tmp_char = 0; 
  for(int x=0; x<10; x++){
    for(unsigned long int i=0; i<my_dat.iterations; i++){
      memcpy(&tmp_char, &i, 1);
      my_dat.mem[my_dat.index] = tmp_char;
    }
    check->check();
  }
}

void simple_test_write_10_checkpoints_blocking(void* data, Checkpoint2* check){
  testDat my_dat = *((testDat*) data);
  char tmp_char = 0; 
  for(int x=0; x<10; x++){
    for(unsigned long int i=0; i<my_dat.iterations; i++){
      memcpy(&tmp_char, &i, 1);
      my_dat.mem[my_dat.index] = tmp_char;
    }
    check->check();
  }
}

void simple_test_write_10_no_checkpoint(void* data){
  testDat my_dat = *((testDat*) data);
  char tmp_char = 0; 
  for(int x=0; x<10; x++){
    for(unsigned int i=0; i<my_dat.iterations; i++){
      memcpy(&tmp_char, &i, 1);
      my_dat.mem[my_dat.index] = tmp_char;
    }
  }
}

//returns duration in microseconds
std::size_t speed_write_beginning_checkpoint_normal(std::size_t allocation_size,
						    unsigned int threadTotal,
						    unsigned long int iterations){
  bool load_first = false;
  std::string LABEL1 = "/home/frederik/kandidat/test_speed_impact/testFile_1";
  std::string LABEL2 = "/home/frederik/kandidat/test_speed_impact/testFile_2";
  std::size_t each_alloc = allocation_size - 50;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  if(checkOrg.initCheckpointOrganizer(threadTotal, load_first, allocation_size))
    return 0;

  void (*test_func)(void *, Checkpoint*) = simple_test_write_10_checkpoints_normal;

  char* test_dat = (char*)checkOrg.check_alloc(each_alloc);
  
  testDat tmp_test_dat;
  tmp_test_dat.iterations = iterations;
  tmp_test_dat.mem = test_dat;
  auto t1 = std::chrono::high_resolution_clock::now();
  for(unsigned int i=0; i<threadTotal; i++){
    tmp_test_dat.index = i;
    checkOrg.startThread(test_func, &tmp_test_dat);
  } 
  checkOrg.joinAll();
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  return duration;
}

std::size_t speed_write_end_checkpoint_normal(std::size_t allocation_size,
						    unsigned int threadTotal,
						    unsigned long int iterations){
  bool load_first = false;
  std::string LABEL1 = "/home/frederik/kandidat/test_speed_impact/testFile_1";
  std::string LABEL2 = "/home/frederik/kandidat/test_speed_impact/testFile_2";
  std::size_t each_alloc = allocation_size - 50;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  if(checkOrg.initCheckpointOrganizer(threadTotal, load_first, allocation_size))
    return 0;

  void (*test_func)(void *, Checkpoint*) = simple_test_write_10_checkpoints_normal;

  char* test_dat = (char*)checkOrg.check_alloc(each_alloc);
  
  testDat tmp_test_dat;
  tmp_test_dat.iterations = iterations;
  tmp_test_dat.mem = test_dat;
  auto t1 = std::chrono::high_resolution_clock::now();
  for(unsigned int i=0; i<threadTotal; i++){
    tmp_test_dat.index = each_alloc - (i + 1);
    checkOrg.startThread(test_func, &tmp_test_dat);
  } 
  checkOrg.joinAll();
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  return duration;
}

std::size_t speed_write_checkpoint_blocking(std::size_t allocation_size,
						    unsigned int threadTotal,
						    unsigned long int iterations){
  bool load_first = false;
  std::string LABEL1 = "/home/frederik/kandidat/test_speed_impact/testFile2_1";
  std::string LABEL2 = "/home/frederik/kandidat/test_speed_impact/testFile2_2";
  std::size_t each_alloc = allocation_size - 50;
  CheckpointOrganizer2 checkOrg(LABEL1, LABEL2);
  if(checkOrg.initCheckpointOrganizer(threadTotal, load_first, allocation_size))
    return 0;

  void (*test_func)(void *, Checkpoint2*) = simple_test_write_10_checkpoints_blocking;

  char* test_dat = (char*)checkOrg.check_alloc(each_alloc);
  
  testDat tmp_test_dat;
  tmp_test_dat.iterations = iterations;
  tmp_test_dat.mem = test_dat;
  auto t1 = std::chrono::high_resolution_clock::now();
  for(unsigned int i=0; i<threadTotal; i++){
    tmp_test_dat.index = i;
    checkOrg.startThread(test_func, &tmp_test_dat);
  } 
  checkOrg.joinAll();
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  return duration;
}

//returns duration in microseconds
std::size_t speed_control(std::size_t allocation_size,
					  unsigned int threadTotal,
					  unsigned long int iterations){
  
  std::vector<std::thread*> track_threads;
  void (*test_func)(void *) = simple_test_write_10_no_checkpoint;
  char* test_dat = (char*) malloc(allocation_size);
  testDat tmp_test_dat;
  tmp_test_dat.iterations = iterations;
  tmp_test_dat.mem = test_dat;
  
  auto t1 = std::chrono::high_resolution_clock::now();
  for(unsigned int i=0; i<threadTotal; i++){
    tmp_test_dat.index = i;
    std::thread* tmp_thread = new std::thread(test_func, &tmp_test_dat);
    track_threads.push_back(tmp_thread);
  }
  
  for(unsigned int x=0; x<track_threads.size(); x++){
    if(track_threads[x]->joinable()) track_threads[x]->join();
  }
  
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  for(unsigned int x=0; x<track_threads.size(); x++){
    delete track_threads[x];
  }
  
  free(test_dat);
  return duration;
}


void _delete_test_files(std::string fName1, std::string fName2){
  if(remove(fName1.c_str())){
    printf("err while deleting file\n");
  }
  if(remove(fName2.c_str())){
    printf("err while deleting file\n");
  }
}

#endif //CHECKPOINTTESTS_HPP
