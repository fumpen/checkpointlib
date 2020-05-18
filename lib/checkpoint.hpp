#ifndef CHECKPOINT_HPP
#define CHECKPOINT_HPP

#include "memManager.hpp"

#include <cstring>
#include <stdlib.h>
#include <stddef.h>
#include <string>
#include <string.h>
#include <vector>
#include <tuple>
#include <iostream>


#include <sys/mman.h> // for mmap

// for native linux open()
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h> // for sendfile()
#include <unistd.h>


// for threading
#include <thread>
#include <mutex>
#include <chrono>

class Checkpoint{
private:
  // for threading
  std::uint8_t threadTotal;
  std::uint8_t myId;
  std::mutex* lockMem;
  
  std::mutex* lockCheck_in;
  std::mutex* lockCheck_out;
  std::uint8_t* barrier_flag_in;
  std::uint8_t* barrier_flag_out;

  std::thread local_thread;
  
  // other stuff
  bool* load_first;                  // if true, load on first checkpoint encounter
  MemManager* mem;                  // pointer to 'active_checkpoint' file
  
  
public:
  
  Checkpoint(std::uint8_t init_threadTotal, std::uint8_t init_myId, std::mutex* init_lockCheck_in,
	     std::mutex* init_lockCheck_out, std::uint8_t* init_barrier_flag_in,
	     std::uint8_t* init_barrier_flag_out, bool* init_load_first, MemManager* init_mem){
    this->threadTotal = init_threadTotal;
    this->myId = init_myId;
    this->lockCheck_in = init_lockCheck_in;
    this->lockCheck_out = init_lockCheck_out;
    this->barrier_flag_in = init_barrier_flag_in;
    this->barrier_flag_out = init_barrier_flag_out;
    
    // other stuff
    this->load_first = init_load_first; 
    this->mem = init_mem;
  }

  ~Checkpoint(){
    if(this->local_thread.joinable()){
      this->local_thread.join();
    }
  }

  void check(){
    //ensure all threads left last checkpoint
    if(this->local_thread.joinable()) this->local_thread.join();
    while(*this->barrier_flag_out){
      std::this_thread::sleep_for(_PauseThreadCheckpoint);
    }
    this->lockCheck_in->lock();
    *this->barrier_flag_in += 1;
    if(*this->barrier_flag_in >= this->threadTotal){
      *this->barrier_flag_out = this->threadTotal; // last thread in set out-barrier
      if(*this->load_first){
	*this->load_first = false;
	this->mem->start_load_checkpoint();
	*this->barrier_flag_in = 0;
      }else{
	this->local_thread =
	  this->mem->start_save_checkpoint(this->lockCheck_in, this->barrier_flag_in);
      }
    }
    this->lockCheck_in->unlock();
    //ensure no threads leave this checkpoint before the critical section is finished
    while(*this->barrier_flag_in){
      std::this_thread::sleep_for(_PauseThreadCheckpoint);
    }
    this->lockCheck_out->lock();
    *this->barrier_flag_out -= 1;
    this->lockCheck_out->unlock();
  }
};


class CheckpointOrganizer{
private:
  // for threading
  std::uint8_t threadTotal;
  std::uint8_t threadCount;
  std::mutex* lockCheck_in;
  std::mutex* lockCheck_out;
  std::uint8_t barrier_flag_in;
  std::uint8_t barrier_flag_out;
  
  
  std::vector<Checkpoint*> trackCheckInner;
  std::vector<std::thread*> trackThreads;
  
  bool load_first;                  // if true, load on first checkpoint encounter
  MemManager* mem;                  // pointer to 'memManager' file
  
public:
  CheckpointOrganizer(std::string init_fileNameA, std::string init_fileNameB){
    //for threading
    this->threadCount = 0;
    this->barrier_flag_in = 0;
    this->barrier_flag_out = 0;
    this->lockCheck_in = new std::mutex();
    this->lockCheck_out = new std::mutex();

    this->mem = new MemManager(init_fileNameA, init_fileNameB);
  }

  ~CheckpointOrganizer(){
    // for threading
    //this->lockCheck_in->lock(); // ensure that no checkpoint is still being saved
    //this->lockCheck_in->unlock();
    for(std::uint8_t i=0; i<this->trackCheckInner.size(); i++){
      if(this->trackCheckInner[i] != 0)
	delete this->trackCheckInner[i];
    }
    for(std::uint8_t i=0; i<this->trackThreads.size(); i++){
      if(this->trackThreads[i] != 0)
	delete this->trackThreads[i];
    }
    if(this->mem != 0){
      delete this->mem;
    }
    delete lockCheck_in;
    delete lockCheck_out;
  }

  int initCheckpointOrganizer(std::uint8_t init_threadTotal, 
			      bool init_load_first, std::size_t expected_size){
    
    this->threadTotal = init_threadTotal;
    this->load_first = init_load_first; // if empty, do not load...
    return this->mem->initMemManager(expected_size);
  }
  
  std::thread* startThread(void (*f)(void *, Checkpoint*), void* init_data){
    this->threadCount += 1;
    Checkpoint* newCheck = new Checkpoint(this->threadTotal, this->threadCount, this->lockCheck_in,
					  this->lockCheck_out, &this->barrier_flag_in,
					  &this->barrier_flag_out, &this->load_first, this->mem);
    this->trackCheckInner.push_back(newCheck);
    std::thread* th = new std::thread(f, init_data, newCheck);
    trackThreads.push_back(th);
    return th;
  }

  int joinAll(){
    for(std::uint8_t i=0; i<this->trackThreads.size(); i++){
      if(this->trackThreads[i]->joinable()){
	this->trackThreads[i]->join();
	
      }
    }
    for(std::uint8_t i=0; i<this->trackThreads.size(); i++){
      if(this->trackThreads[i] != 0)
	delete this->trackThreads[i];
    }
    this->trackThreads.clear();
    
    for(std::uint8_t i=0; i<this->trackCheckInner.size(); i++){
      if(this->trackCheckInner[i] != 0)
	delete this->trackCheckInner[i];
    }
    this->trackCheckInner.clear();
    return 0;
  }

  void* check_alloc(size_t requested_size){
    return this->mem->allocate_memory(requested_size);
  }

  int check_free(void* addr){
    return this->mem->free_memory(addr);
  }
  
};




#endif /* CHECKPOINT_HPP */
