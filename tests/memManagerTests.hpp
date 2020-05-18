#ifndef MEMMANAGERTESTS_HPP
#define MEMMANAGERTESTS_HPP

#include "../lib/memManager.hpp"
#include "testHelp.hpp"
#include <iostream>
#include <vector>
#include <stdio.h> // remove()

// open()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // close()
#include <sys/mman.h> // mmap()

#include <errno.h>
#include <string.h>

std::string mm_cleanup_test(std::string msg){
  _delete_test_files(LABEL1, LABEL2);
  return cleanup_test(msg);
}

std::string test0_make_memManager_instance(){
  size_t memSize = 200;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
    
  _delete_test_files(LABEL1, LABEL2);
  return mm_cleanup_test(RET_SUCESS);
}


std::string test1_allocate_MM(){
  // alloc only one page
  size_t memSize = round_up_page(1); 
  //dividing free mem into 2 allocation chunks, firstfit datastructure included, as to fill the allocated memory completely
  size_t alloc_1 = memSize / 2;
  size_t alloc_2 = alloc_1;
  if(memSize % 2) alloc_1 += 1; 
  alloc_1 -= ff_alloc_size;
  alloc_2 -= ff_alloc_size;

  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  void* p1 = mm.allocate_memory(alloc_1);
  if(p1 == 0) return mm_cleanup_test("p1 == 0");

  void* p2 = mm.allocate_memory(alloc_2 + 1);
  if(p2 != 0) return mm_cleanup_test("p2 != 0");
  p2 = mm.allocate_memory(alloc_2);
  if(p2 == 0) return mm_cleanup_test("p2 == 0");

  return mm_cleanup_test(RET_SUCESS);
}

std::string test2_free_MM(){
  size_t memSize = 200;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  
  void* p1 = mm.allocate_memory(100);
  mm.allocate_memory(50);
  if(mm.free_memory(p1))
    return mm_cleanup_test("mm.free_memory(p1)");
  p1 = mm.allocate_memory(100);
  if(p1 == 0) return mm_cleanup_test("p1 == 0");

  return mm_cleanup_test(RET_SUCESS);
}

std::string test3_save_allocation(){
  std::mutex memLock;
  std::uint8_t barrier = 1;
  size_t memSize = 20000;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  int numInts = 1000;
  int* p1 = (int*)mm.allocate_memory(numInts * sizeof(int));
  
  for(int i=0; i<numInts; i++){
    p1[i] = i * 2;
  }
  
  std::thread th = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  th.join();

  void* fileContent = malloc(memSize);
  vec_malloc.push_back(fileContent);
  
  
  int fd = open(LABEL2.c_str(), O_RDONLY);
  if(fd == -1) return mm_cleanup_test("fd == -1");
  
  if(read(fd, fileContent, memSize) == -1)
    return mm_cleanup_test("error in read");
  close(fd);
  
  // + sizeof(std::size_t) (file counter), + ff_alloc_size (size of single allocation)
  int* testInts = (int*) (((std::uint8_t*)fileContent) + ff_alloc_size + sizeof(std::size_t)); 
  for(int i=0; i<numInts; i++){
    if(testInts[i] != (i * 2)){
      return mm_cleanup_test("testInts[i] != (i * 2)");
    }
  }
  
  return mm_cleanup_test(RET_SUCESS);
}

//size_t _page_size
std::string test4_load_allocation(){
  printf("asd4\n");
  std::mutex memLock;
  std::uint8_t barrier = 1;
  
  size_t memSize = 20000;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  int numInts = 2000;
  int* p1 = (int*)mm.allocate_memory(numInts * sizeof(int));
  
  for(int i=0; i<numInts; i++){
    p1[i] = i * 2;
  }
  
  std::thread th = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  barrier = 1;
    
  for(int i=0; i<numInts; i++){
    p1[i] = 0;
  }
  
  barrier = 1;
  memLock.lock();
  mm.start_load_checkpoint();
  memLock.unlock();
  
  for(int i=0; i<numInts; i++){
    if(p1[i] != (i * 2)) return mm_cleanup_test("p1[i] != (i * 2)");
  }

  if(th.joinable()){
      th.join();
    }else{
      return mm_cleanup_test("th.joinable()");
    }
  return mm_cleanup_test(RET_SUCESS);
}

std::string test5_visual_confirmation_that_signalcatching_is_working(){
  std::mutex memLock;
  std::uint8_t barrier = 1;
  
  int number_of_allocations = 5;
  size_t alloc_size = 200000;
  size_t memSize = (alloc_size + ff_alloc_size) * number_of_allocations + 1000; //leave a couple of pages pr allocation
  
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  std::vector<void*> allocations;
  for(int i=0; i<number_of_allocations; i++){
    allocations.push_back(mm.allocate_memory(alloc_size));
  }
  
  std::thread th1 = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  barrier = 1;
  
  for(int i=0; i<number_of_allocations; i++){
    *(int*)allocations[i] = 0;
    printf("wrote to alloc number %d \n", i);
  }

  std::thread th2 = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  
  for(int i=(number_of_allocations -1); i>-1; i--){
    *(int*)allocations[i] = 0;
    printf("wrote to alloc number %d (reverse order)\n", i);
  }

  if(th1.joinable()){
      th1.join();
    }else{
      return mm_cleanup_test("th1.joinable()");
    }
  
  if(th2.joinable()){
      th2.join();
    }else{
      return mm_cleanup_test("th2.joinable()");
    }
  return mm_cleanup_test(RET_SUCESS);
}


//size_t _page_size
std::string test6_save_allocation_advanced(){
  std::mutex memLock;
  std::uint8_t barrier = 1;
  
  size_t memSize = round_up_page(10000);
  size_t string_length = memSize - ff_alloc_size;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  
  char* allocated_pointer = (char*)mm.allocate_memory(string_length);
  if(allocated_pointer == 0) return mm_cleanup_test("allocated_pointer == 0");

  char* validation_buff = (char*)malloc(string_length);
  vec_malloc.push_back(validation_buff);
  if(validation_buff == 0) return mm_cleanup_test("validation_buff == 0");
  
  for(size_t i=0; i<string_length; i++){
    memcpy(&allocated_pointer[i], &i, 1);
    memcpy(&validation_buff[i], &i, 1);
  }
  
  if(memcmp(allocated_pointer, validation_buff, string_length))
    return mm_cleanup_test("[1]memcmp(allocated_pointer, validation_buff, string_length)");

  std::thread th = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  
  barrier = 1;
  memLock.lock();
  mm.start_load_checkpoint();
  memLock.unlock();

  if(memcmp(allocated_pointer, validation_buff, string_length) != 0)
    return mm_cleanup_test("[2]memcmp(allocated_pointer, validation_buff, string_length)");


  if(th.joinable()){
    th.join();
  }else{
    return mm_cleanup_test("th1.joinable()");
  }
  return mm_cleanup_test(RET_SUCESS);
}


std::string test7_load_allocation_advanced(){
  // continuation of test 6..
  std::mutex memLock;
  std::uint8_t barrier = 1;
  
  size_t memSize = round_up_page(10000);
  size_t string_length = memSize - ff_alloc_size;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  
  char* p = (char*)mm.allocate_memory(string_length);
  if(p == 0) return mm_cleanup_test("p == 0");

  char* validation_buff = (char*)malloc(string_length);
  vec_malloc.push_back(validation_buff);
  if(validation_buff == 0) return mm_cleanup_test("validation_buff == 0");
  
  for(size_t i=0; i<string_length; i++){
    memcpy(&p[i], &i, 1);
    memcpy(&validation_buff[i], &i, 1);
  }
  
  if(memcmp(p, validation_buff, string_length) != 0)
    return mm_cleanup_test("[1]memcmp(p, validation_buff, string_length) != 0");

  std::thread th = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  barrier = 1;
  // clear memory to preare for load
  char tmp_dat = 0;
  for(size_t i=0; i<string_length; i++){
    memcpy(&p[i], &tmp_dat, 1);
  }
  if(memcmp(p, validation_buff, string_length) == 0)
    return mm_cleanup_test("memcmp(p, validation_buff, string_length) == 0");
  
  barrier = 1;
  memLock.lock();
  mm.start_load_checkpoint();
  memLock.unlock();
  
  if(memcmp(p, validation_buff, string_length) != 0)
    return mm_cleanup_test("[2]memcmp(p, validation_buff, string_length) != 0");

  if(th.joinable()){
    th.join();
  }else{
    return mm_cleanup_test("th1.joinable()");
  }
  return mm_cleanup_test(RET_SUCESS);
}


std::string test8_final(){
  std::mutex memLock;
  std::uint8_t barrier = 1;
  
  size_t memSize = round_up_page(10000);
  size_t string_length = memSize - ff_alloc_size;
  MemManager mm(LABEL1, LABEL2);
  if(mm.initMemManager(memSize))
    return mm_cleanup_test("mm.initMemManager(memSize)"); 
  
  char* p = (char*)mm.allocate_memory(string_length);
  if(p == 0) return mm_cleanup_test("p == 0");

  char* validation_buff = (char*)malloc(string_length);
  vec_malloc.push_back(validation_buff);
  if(validation_buff == 0) return mm_cleanup_test("validation_buff == 0");
  
  for(size_t i=0; i<string_length; i++){
    memcpy(&p[i], &i, 1);
  }
  
  std::thread th1 = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  barrier = 1;

  std::size_t tmp_val = 0;
  for(size_t i=0; i<string_length; i++){
    tmp_val = ~i;
    memcpy(&p[i], &tmp_val, 1);
    memcpy(&validation_buff[i], &tmp_val, 1);
  }

  std::thread th2 = mm.start_save_checkpoint(&memLock, &barrier);
  while(barrier){}
  barrier = 1;
  
  // clear memory to preare for load
  tmp_val = 0;
  for(size_t i=0; i<string_length; i++){
    memcpy(&p[i], &tmp_val, 1);
  }
  if(memcmp(p, validation_buff, string_length) == 0)
    return mm_cleanup_test("memcmp(p, validation_buff, string_length) == 0");
  
  barrier = 1;
  memLock.lock();
  mm.start_load_checkpoint();
  memLock.unlock();
  
  if(memcmp(p, validation_buff, string_length) != 0)
    return mm_cleanup_test("[2]memcmp(p, validation_buff, string_length) != 0");

  if(th1.joinable()){
    th1.join();
  }else{
    return mm_cleanup_test("th1.joinable()");
  }
  if(th2.joinable()){
    th2.join();
  }else{
    return mm_cleanup_test("th2.joinable()");
  }
  return mm_cleanup_test(RET_SUCESS);
}

int run_all_memManagerTests(){
  std::vector<std::string> test_results;
  std::cout << "\n--- Initiating memManager tests: --- \n";
  
  cleanup_test(" ");
  try{
    std::string tmp_res = test0_make_memManager_instance();
    test_results.push_back(tmp_res);
  }catch(...){
    test_results.push_back("test0_make_memManager_instance crashed");
  }
  cleanup_test(" ");

  
  test_results.push_back(test1_allocate_MM());
  cleanup_test(" ");
  
  test_results.push_back(test2_free_MM());
  cleanup_test(" ");
  
  test_results.push_back(test3_save_allocation()); 
  cleanup_test(" ");
  
  test_results.push_back(test4_load_allocation());
  cleanup_test(" ");
  
  test_results.push_back(test5_visual_confirmation_that_signalcatching_is_working());
  cleanup_test(" ");
  
  test_results.push_back(test6_save_allocation_advanced());
  cleanup_test(" ");

  test_results.push_back(test7_load_allocation_advanced());
  cleanup_test(" ");

  test_results.push_back(test8_final());
  cleanup_test(" ");
  
  std::cout << "\n--- memManager test results: --- \n";
  
  for(std::size_t i=0; i<test_results.size(); i++){
    if(RET_SUCESS.compare(test_results[i])){
      std::cout << "test " << i << " finished with: '" << "ERROR -> " + test_results[i] << "' \n";
    }else{
      std::cout << "test " << i << " finished with: '" << test_results[i] << "' \n";
    }
  }
  std::cout << "\n--- finished with memManager tests --- \n";
  
  return 0;
}

#endif //MEMMANAGERTESTS_HPP


/*
  std::size_t tres = round_up_page(10000);
  std::cout << "res: " << tres << "\n\n";
  tres = round_up_page(12288);
  std::cout << "res: " << tres << "\n\n";
  tres = round_up_page(12289);
  std::cout << "res: " << tres << "\n\n";
  tres = round_up_page(0);
  std::cout << "res: " << tres << "\n\n";
  */
