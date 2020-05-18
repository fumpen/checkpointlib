#ifndef CHECKPOINTTESTS_HPP
#define CHECKPOINTTESTS_HPP

#include <stdio.h>
#include "../../lib/checkpoint.hpp"
#include "randArr.hpp"
#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <time.h>



struct test_thread_struct{
  arrTestDat _arrTest;
  bool initiate_thread = false;
};

void almost_empty_function(int iterations){
  for(int i=0; i<iterations; i++){
    // do nothing...
  }
}

// only one of the threads has tmp_struct.initiate_thread==true, simulating checkpointlib...  
void array_test_start_thread(void* data){
  srand(time(NULL));
  std::thread tmp_thread;
  test_thread_struct* tmp_struct = (test_thread_struct*) data;
  arrTestDat my_dat = tmp_struct->_arrTest;
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);

  if(tmp_struct->initiate_thread) tmp_thread = std::thread(almost_empty_function, 5);
  
  
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);

  if(tmp_struct->initiate_thread) tmp_thread.join();
}

std::chrono::milliseconds global_PauseThread(100); 
std::uint8_t global_total_threads = 0;
std::uint8_t global_flag_in = 0;
std::uint8_t global_flag_out = 0;
std::mutex global_lock_in;
std::mutex global_lock_out;

void mock_barrier(){
  while(global_flag_out){
    std::this_thread::sleep_for(global_PauseThread);
  }
  global_lock_in.lock();
  global_flag_in += 1;
  if(global_flag_in >= global_total_threads){ // start of critical section
    global_flag_out = global_total_threads;
    global_flag_in = 0;
  }
  global_lock_in.unlock();
  
  while(global_flag_in){
    std::this_thread::sleep_for(global_PauseThread);
  }
  global_lock_out.lock();
  global_flag_out -= 1;
  global_lock_out.unlock();
}



void array_test_lock(void* data){
  srand(time(NULL));
  arrTestDat my_dat = *((arrTestDat*) data);
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  mock_barrier();
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
}


void array_test_checkpoint(void* data, Checkpoint* check){
  srand(time(NULL));
  arrTestDat my_dat = *((arrTestDat*) data);
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  check->check();
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
}


void array_test_control(void* data){
  srand(time(NULL));
  arrTestDat my_dat = *((arrTestDat*) data);
  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);

  add_multiples_to_all(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
  refresh_prime(my_dat.prime, my_dat.mem, my_dat.segment, my_dat.num_segments);
}



std::size_t test_array_scramble_normal(unsigned int rep){
  srand(time(NULL));
  bool load_first = false;
  std::string LABEL1 = "testFile_1.tmp";
  std::string LABEL2 = "testFile_2.tmp";

  unsigned int threadTotal = 4;
  const std::size_t segment = 200000;
  std::size_t num_segments = 100; 
  std::size_t each_alloc = ((segment * sizeof(int)) + 9) ;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  if(checkOrg.initCheckpointOrganizer(threadTotal, load_first, each_alloc * threadTotal)){
    std::cout << "err in speedtest\n";
    return 0;
  }
  
  int* buff = (int*)malloc((segment * num_segments * sizeof(int)) * threadTotal);
  if(buff == 0) std::cout << "error!\n";

  void (*test_func)(void *, Checkpoint*) = array_test_checkpoint;
  arrTestDat atd1;
  int* tmp1 = (int*)checkOrg.check_alloc(segment * sizeof(int));
  init_arr(tmp1, segment);
  atd1.segment = segment;
  atd1.num_segments = num_segments;
  atd1.prime = tmp1;
  atd1.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd2;
  int* tmp2 = (int*)checkOrg.check_alloc(segment * sizeof(int));
  init_arr(tmp2, segment);
  atd2.segment = segment;
  atd2.num_segments = num_segments;
  atd2.prime = tmp2;
  atd2.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd3;
  int* tmp3 = (int*)checkOrg.check_alloc(segment * sizeof(int));
  init_arr(tmp3, segment);
  atd3.segment = segment;
  atd3.num_segments = num_segments;
  atd3.prime = tmp3;
  atd3.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd4;
  int* tmp4 = (int*)checkOrg.check_alloc(segment * sizeof(int));
  init_arr(tmp4, segment);
  atd4.segment = segment;
  atd4.num_segments = num_segments;
  atd4.prime = tmp4;
  atd4.mem = (int**)malloc(sizeof(int*) * num_segments);

  std::size_t pointer = 0;
  for(unsigned long int i=0; i<num_segments; i++){
    atd1.mem[i] = &buff[pointer];
    pointer += segment;
    atd2.mem[i] = &buff[pointer];
    pointer += segment;
    atd3.mem[i] = &buff[pointer];
    pointer += segment;
    atd4.mem[i] = &buff[pointer];
    pointer += segment; 
  }
  
  init_all(atd1.mem, segment, num_segments);
  init_all(atd2.mem, segment, num_segments);
  init_all(atd3.mem, segment, num_segments);
  init_all(atd4.mem, segment, num_segments);
  
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Initiation done chkpt\n";
  std::size_t duration = 0;
  for(unsigned long int i=0; i<rep; i++){
    std::cout << "iteration "<< i << " of "<< rep << "\n";
    t1 = std::chrono::high_resolution_clock::now();
    checkOrg.startThread(test_func, &atd1);
    checkOrg.startThread(test_func, &atd2);
    checkOrg.startThread(test_func, &atd3);
    checkOrg.startThread(test_func, &atd4);
    checkOrg.joinAll();
    t2 = std::chrono::high_resolution_clock::now();
    duration += std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  }

  duration = duration / rep;

  free(atd1.mem);
  free(atd2.mem);
  free(atd3.mem);
  free(atd4.mem);
  free(buff);
  return duration;
}

std::size_t test_array_scramble_control(unsigned int rep){
  srand(time(NULL));
  
  unsigned int threadTotal = 4;
  const std::size_t segment = 200000;
  std::size_t num_segments = 100; // 1000;
  std::size_t each_alloc = segment * num_segments * sizeof(int);
  int* buff = (int*)malloc(each_alloc * threadTotal);
  if(buff == 0) std::cout << "error!\n";

  void (*test_func)(void *) = array_test_control;
  arrTestDat atd1;
  int tmp1[segment];
  init_arr(tmp1, segment);
  atd1.segment = segment;
  atd1.num_segments = num_segments;
  atd1.prime = tmp1;
  atd1.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd2;
  int tmp2[segment];
  init_arr(tmp2, segment);
  atd2.segment = segment;
  atd2.num_segments = num_segments;
  atd2.prime = tmp2;
  atd2.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd3;
  int tmp3[segment];
  init_arr(tmp3, segment);
  atd3.segment = segment;
  atd3.num_segments = num_segments;
  atd3.prime = tmp3;
  atd3.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd4;
  int tmp4[segment];
  init_arr(tmp4, segment);
  atd4.segment = segment;
  atd4.num_segments = num_segments;
  atd4.prime = tmp4;
  atd4.mem = (int**)malloc(sizeof(int*) * num_segments);
  std::size_t pointer =0;
  for(unsigned long int i=0; i<num_segments; i++){
    atd1.mem[i] = &buff[pointer];
    pointer += segment;
    atd2.mem[i] = &buff[pointer];
    pointer += segment;
    atd3.mem[i] = &buff[pointer];
    pointer += segment;
    atd4.mem[i] = &buff[pointer];
    pointer += segment; 
  }
  init_all(atd1.mem, segment, num_segments);
  init_all(atd2.mem, segment, num_segments);
  init_all(atd3.mem, segment, num_segments);
  init_all(atd4.mem, segment, num_segments);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Initiation done control\n";
  std::size_t duration = 0;
  for(unsigned long int i=0; i<rep; i++){
    std::cout << "iteration "<< i << " of "<< rep << "\n";
    t1 = std::chrono::high_resolution_clock::now();
    std::thread tmp_thread1(test_func, &atd1);
    std::thread tmp_thread2(test_func, &atd2);
    std::thread tmp_thread3(test_func, &atd3);
    std::thread tmp_thread4(test_func, &atd4);
    tmp_thread1.join();
    tmp_thread2.join();
    tmp_thread3.join();
    tmp_thread4.join();
    t2 = std::chrono::high_resolution_clock::now();
    duration += std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  }

  duration = duration / rep;

  free(atd1.mem);
  free(atd2.mem);
  free(atd3.mem);
  free(atd4.mem);
  free(buff);
  return duration;
}



std::size_t test_array_scramble_barrier(unsigned int rep){
  srand(time(NULL));
  
  unsigned int threadTotal = 4;
  global_total_threads = threadTotal; // Necessity for the mock barrier
  const std::size_t segment = 200000;
  std::size_t num_segments = 100; // 1000;
  std::size_t each_alloc = segment * num_segments * sizeof(int);
  int* buff = (int*)malloc(each_alloc * threadTotal);
  if(buff == 0) std::cout << "error!\n";

  void (*test_func)(void *) = array_test_lock;
  arrTestDat atd1;
  int tmp1[segment];
  init_arr(tmp1, segment);
  atd1.segment = segment;
  atd1.num_segments = num_segments;
  atd1.prime = tmp1;
  atd1.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd2;
  int tmp2[segment];
  init_arr(tmp2, segment);
  atd2.segment = segment;
  atd2.num_segments = num_segments;
  atd2.prime = tmp2;
  atd2.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd3;
  int tmp3[segment];
  init_arr(tmp3, segment);
  atd3.segment = segment;
  atd3.num_segments = num_segments;
  atd3.prime = tmp3;
  atd3.mem = (int**)malloc(sizeof(int*) * num_segments);
  arrTestDat atd4;
  int tmp4[segment];
  init_arr(tmp4, segment);
  atd4.segment = segment;
  atd4.num_segments = num_segments;
  atd4.prime = tmp4;
  atd4.mem = (int**)malloc(sizeof(int*) * num_segments);
  std::size_t pointer =0;
  for(unsigned long int i=0; i<num_segments; i++){
    atd1.mem[i] = &buff[pointer];
    pointer += segment;
    atd2.mem[i] = &buff[pointer];
    pointer += segment;
    atd3.mem[i] = &buff[pointer];
    pointer += segment;
    atd4.mem[i] = &buff[pointer];
    pointer += segment; 
  }
  init_all(atd1.mem, segment, num_segments);
  init_all(atd2.mem, segment, num_segments);
  init_all(atd3.mem, segment, num_segments);
  init_all(atd4.mem, segment, num_segments);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Initiation done barrier\n";
  std::size_t duration = 0;
  for(unsigned long int i=0; i<rep; i++){
    std::cout << "iteration "<< i << " of "<< rep << "\n";
    t1 = std::chrono::high_resolution_clock::now();
    std::thread tmp_thread1(test_func, &atd1);
    std::thread tmp_thread2(test_func, &atd2);
    std::thread tmp_thread3(test_func, &atd3);
    std::thread tmp_thread4(test_func, &atd4);
    tmp_thread1.join();
    tmp_thread2.join();
    tmp_thread3.join();
    tmp_thread4.join();
    t2 = std::chrono::high_resolution_clock::now();
    duration += std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  }

  duration = duration / rep;

  free(atd1.mem);
  free(atd2.mem);
  free(atd3.mem);
  free(atd4.mem);
  free(buff);
  return duration;
}


std::size_t test_array_scramble_thread(unsigned int rep){
  srand(time(NULL));
  
  unsigned int threadTotal = 4;
  global_total_threads = threadTotal; // Necessity for the mock barrier
  const std::size_t segment = 200000;
  std::size_t num_segments = 100; // 1000;
  std::size_t each_alloc = segment * num_segments * sizeof(int);
  int* buff = (int*)malloc(each_alloc * threadTotal);
  if(buff == 0) std::cout << "error!\n";

  test_thread_struct _tts1, _tts2, _tts3, _tts4;
  
  void (*test_func)(void *) = array_test_start_thread;
  arrTestDat atd1;
  int tmp1[segment];
  init_arr(tmp1, segment);
  atd1.segment = segment;
  atd1.num_segments = num_segments;
  atd1.prime = tmp1;
  atd1.mem = (int**)malloc(sizeof(int*) * num_segments);
  _tts1._arrTest = atd1;
  _tts1.initiate_thread = true;
  arrTestDat atd2;
  int tmp2[segment];
  init_arr(tmp2, segment);
  atd2.segment = segment;
  atd2.num_segments = num_segments;
  atd2.prime = tmp2;
  atd2.mem = (int**)malloc(sizeof(int*) * num_segments);
  _tts2._arrTest = atd2;
  arrTestDat atd3;
  int tmp3[segment];
  init_arr(tmp3, segment);
  atd3.segment = segment;
  atd3.num_segments = num_segments;
  atd3.prime = tmp3;
  atd3.mem = (int**)malloc(sizeof(int*) * num_segments);
  _tts3._arrTest = atd3;
  arrTestDat atd4;
  int tmp4[segment];
  init_arr(tmp4, segment);
  atd4.segment = segment;
  atd4.num_segments = num_segments;
  atd4.prime = tmp4;
  atd4.mem = (int**)malloc(sizeof(int*) * num_segments);
  _tts4._arrTest = atd4;
  std::size_t pointer =0;
  for(unsigned long int i=0; i<num_segments; i++){
    atd1.mem[i] = &buff[pointer];
    pointer += segment;
    atd2.mem[i] = &buff[pointer];
    pointer += segment;
    atd3.mem[i] = &buff[pointer];
    pointer += segment;
    atd4.mem[i] = &buff[pointer];
    pointer += segment; 
  }
  init_all(atd1.mem, segment, num_segments);
  init_all(atd2.mem, segment, num_segments);
  init_all(atd3.mem, segment, num_segments);
  init_all(atd4.mem, segment, num_segments);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Initiation done thread\n";
  std::size_t duration = 0;
  for(unsigned long int i=0; i<rep; i++){
    std::cout << "iteration "<< i << " of "<< rep << "\n";
    t1 = std::chrono::high_resolution_clock::now();
    std::thread tmp_thread1(test_func, &_tts1);
    std::thread tmp_thread2(test_func, &_tts2);
    std::thread tmp_thread3(test_func, &_tts3);
    std::thread tmp_thread4(test_func, &_tts4);
    tmp_thread1.join();
    tmp_thread2.join();
    tmp_thread3.join();
    tmp_thread4.join();
    t2 = std::chrono::high_resolution_clock::now();
    duration += std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  }

  duration = duration / rep;

  free(atd1.mem);
  free(atd2.mem);
  free(atd3.mem);
  free(atd4.mem);
  free(buff);
  return duration;
}

#endif //CHECKPOINTTESTS_HPP
