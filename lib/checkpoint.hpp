/*! \file fileManager.hpp
 * This file contains the Checkpoint and CheckpointOrganizer classes, which handles thread initiation, thread coordination
 * and acts as the outer layer of the checkpointlib.
*/
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


/*! \brief The Checkpoint class handles synchronization and creation of a checkpoint in each individual thread started.
 */
class Checkpoint{
private:
  // for threading
  std::uint8_t threadTotal; //<! The total number of initiated threads for this thread to synchronize with.
  std::uint8_t myId; //<! The local id of this thread.
  std::mutex* lockMem; //<! Mutex controlling memory access.
  
  std::mutex* lockCheck_in; //<! First mutex controlling barrier synchronization between threads.
  std::mutex* lockCheck_out; //<! Second mutex controlling barrier synchronization between threads.
  std::uint8_t* barrier_flag_in; //<! First flag controlling barrier synchronization between threads.
  std::uint8_t* barrier_flag_out; //<! Second flag controlling barrier synchronization between threads.

  std::thread local_thread; //<! tracks if this thread currently owns a checkpoint-taking thread.
  
  bool* load_first; //<! If true, load on first checkpoint encounter
  MemManager* mem; //<! Points to the memManager for this checkpoint thread.
  
  
public:
  /*! \brief The Checkpoint class sets the variables that is initialized by CheckpointOrganizer.
   * This constructor is mean to be implicitly cast by CheckpointOrganizer.
   \param init_threadTotal The total number of initiated threads for this thread to synchronize with.
   \param init_myId The local id of this thread.
   \param init_lockCheck_in First mutex controlling barrier synchronization between threads.
   \param init_lockCheck_out Second mutex controlling barrier synchronization between threads.
   \param init_barrier_flag_in First flag controlling barrier synchronization between threads.
   \param init_barrier_flag_out Second flag controlling barrier synchronization between threads.
   \param init_load_first If true, load on first checkpoint encounter
   \param init_mem Points to the memManager for this checkpoint thread.
   */
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

  /*! \brief When calling this, all threads are synchronized and a new checkpoint is made.
   * In practice, a barrier lock is put in place that will only lift when all checkpoints started
   * by the same instance of checkpointOrganizer has reached this call.
   * If the number of calls to check() is not equal among threads, a deadlock will occur.
     */
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


/*! \brief The CheckpointOrganizer initiates a number of threads along with a instance of the Checkpoint class.
 */
class CheckpointOrganizer{
private:

  std::uint8_t threadTotal; //<! The intended total number of initiated threads.
  std::uint8_t threadCount; //<! The current number of initiated threads.
  std::mutex* lockCheck_in; //<! First mutex controlling barrier synchronization between threads.
  std::mutex* lockCheck_out; //<! Second mutex controlling barrier synchronization between threads.
  std::uint8_t barrier_flag_in; //<! First flag controlling barrier synchronization between threads.
  std::uint8_t barrier_flag_out; //<! Second flag controlling barrier synchronization between threads.
  
  
  std::vector<Checkpoint*> trackCheckInner; //<! Tracking Checkpoint's started by this checkpointOrganizer.
  std::vector<std::thread*> trackThreads; //<! Tracking threads started by this checkpointOrganizer.
  
  bool load_first; //<! If true, the Checkpoint classes is set to load on first checkpoint encounter.
  MemManager* mem; //<! Points to the memManager for this checkpoint thread.
  
public:
  /*! \brief Constructs a  instance of the CheckpointOrganizer class.
  * Must be initialized before it is usable.
  \param init_fileNameA Path and filename of one of the files to be used as checkpoint-files (or to be created).
  \param init_fileNameB See init_fileNameA, should not be identical to init_fileNameA.
   */
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

  /*! \brief Initiates the checkpointOrganizer.
    * Must called before the checkpointOrganizer is usable. This class allocates memory and
    * ensures the checkpoint-files is usable and created.
    \param init_threadTotal The number of threads that this checkpointOrganizer is expected to initiate.
    \param init_load_first If true, the Checkpoint classes is set to load on first checkpoint encounter.
    \param expected_size The total size in bytes needed for checkpoint-memory, being the sum of all planned allocations.
    \return 0 on success, -1 on failure.
     */
  int initCheckpointOrganizer(std::uint8_t init_threadTotal, 
			      bool init_load_first, std::size_t expected_size){
    
    this->threadTotal = init_threadTotal;
    this->load_first = init_load_first; // if empty, do not load...
    return this->mem->initMemManager(expected_size);
  }
  
  /*! \brief Start a thread provided with a instance of Checkpoint.
  \param f Pointer to function that is to be started in the new thread.
  \param init_data pointer to the data that should be passed as argument to f.
  \return The pointer to the thread that just started is returned on success, a null-pointer on failure.
   */
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

  /*! \brief Put the calling thread to sleep untill all spawned threads has returned.
	\return Always returns 0.
 */
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

  /*! \brief Reserve a piece of the checkpoint memory.
   * All reservations made by this call should ideally sum up to "expected_size" from the initCheckpointOrganizer.
   \param requested_size The size in bytes to request.
  	\return A pointer to a free block of memory of size requested_size or a null-pointer on failure.
   */
  void* check_alloc(size_t requested_size){
    return this->mem->allocate_memory(requested_size);
  }

  /*! \brief Free a piece of the checkpoint memory.
	* A valid address to free should come from a call to check_alloc.
	\param addr The memory address to free.
	\return 0 on success, else -1.
 */
  int check_free(void* addr){
    return this->mem->free_memory(addr);
  }
};

#endif /* CHECKPOINT_HPP */
