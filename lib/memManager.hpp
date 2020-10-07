/*! \file memManager.hpp 
 * This file contains the memManager class as well as functions for replacing the segfault handler 
 * and create/restore a checkpoint. 
*/
#ifndef MEMMANAGER_HPP
#define MEMMANAGER_HPP

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/mman.h> //for mmap()
#include <sys/sendfile.h> // for sendfile()
#include <stdint.h>

// for open()...
#include <sys/stat.h>
#include <fcntl.h>

#include "fileManager.hpp"
#include "first_fit.hpp"

#include <map>
#include <fstream>

// for threading
#include <thread>
#include <mutex>

#include <signal.h>
void* _protected_mem_startAddr = 0;
void* _protected_mem_stopAddr = 0;

#include <chrono>
std::chrono::milliseconds _PauseThreadCheckpoint(100); 


/*! \brief Custom segfault function.
 * If the segfault occurs within the address range of _protected_mem_startAddr and 
 * _protected_mem_stopAddr, the thread sleeps for a short while corresponding 
 * to _PauseThreadCheckpoint, whereas it retries the statement that caused the segfault.
 * If the address is outside the range of _protected_mem, the process simply exits.  
  \param signal Not used (void).
  \param arg Not used (void).
  \param si Containing the memory address where the segfault originated. 
 */
void segfault_sigaction(int signal, siginfo_t *si, void *arg){
  (void) signal;
  (void) arg;
  if((_protected_mem_startAddr<=si->si_addr) && (si->si_addr <= _protected_mem_stopAddr)){
    std::this_thread::sleep_for(_PauseThreadCheckpoint);
  }else{
    printf("\nCaught segfault outside protected memory area: %p \n", si->si_addr);
    exit(EXIT_FAILURE);
  }     
}


/*! \brief Save a checkpoint to disk.
 * Has two steps. First it marks all checkpoint memory as read-only. Next it writes the 
 * checkpoint memory to disk one PAGESIZE at the time and removes the read-only restriction
 * from the memory that is committed to disk.  
  \param lockCheck The function will hold this untill memory is marked read-only.
  \param barrier_flag All threads waits for this to be set to 0, this function will do so after memory is marked read-only.
  \param memAddr Start of checkpoint memory block.
  \param memSize Size of checkpoint memory.
  \param FM Points to the instance of FileManager responsible for I/O of the running instance of checkpointlib.
 */
void _save_checkpoint_gradually(std::mutex* lockCheck, std::uint8_t* barrier_flag,
			       void* memAddr, size_t memSize, FileManager* FM){
  lockCheck->lock();
  size_t pSize = sysconf(_SC_PAGESIZE);
  int prot_status = mprotect(memAddr, memSize, PROT_READ);
  if(prot_status) printf("something wrong in mprot init\n");
  *barrier_flag = 0; // set the threads free to compute, now that the mem-protection is set

  size_t tmp_offset = 0;
  ssize_t tmp_write_ret;
  int fd = FM->save_checkpoint_fd();
  if(fd != -1){
    
    while(tmp_offset < memSize){
      if((memSize - tmp_offset) < pSize){
	tmp_write_ret =  write(fd, ((uint8_t*) memAddr) + tmp_offset, (memSize - tmp_offset));
	prot_status = mprotect(((uint8_t*) memAddr) + tmp_offset,
			       (memSize - tmp_offset), PROT_READ | PROT_WRITE);
	if(prot_status)printf("something went wrong in mprot\n");
      
      }else{
	tmp_write_ret =  write(fd, ((uint8_t*) memAddr) + tmp_offset, pSize);
	prot_status = mprotect(((uint8_t*) memAddr) + tmp_offset, pSize, PROT_READ | PROT_WRITE);
	if(prot_status) printf("something went wrong in mprot\n");
      }
    
      if(tmp_write_ret < 1) printf("error -> tmp write ret-val: %ld in save()\n", tmp_write_ret); 
      tmp_offset += tmp_write_ret;
    
    }

    FM->save_checkpoint_finalize_save(fd);
  }

  lockCheck->unlock();
}

/*! \brief Load a checkpoint to disk.
 * Loads a checkpoint into checkpoint memory. This call blocks untill the checkpoint is 
 * fully loaded.  
  \param memAddr Start of checkpoint memory block.
  \param memSize Size of checkpoint memory.
  \param FM Points to the instance of FileManager responsible for I/O of the running instance of checkpointlib.
 */
void _load_checkpoint(void* memAddr, size_t memSize,
		      FileManager* FM){  
  if(FM->load_checkpoint(memAddr, memSize)){
    printf("ERROR LOADING\n");
  }
}

/*! \brief Manages the checpointlib's allocated memory, concisting of a single continuous block of memory.
 */
class MemManager{
private:
  void* mm = 0; //<! Points to the beginning of the mmapped memory.
  size_t tot_mem; //<! Size in bytes of mm
  FirstFit* mem_logic = 0; //<! Controls allocations into mm. For more, see the FirstFit class description.
  FileManager* file_manager = 0; //<! Controls the IO operations of the library.

  /*! \brief Returns the number of PAGESIZE required to represent the allocated memory (always rounded up) 
    \param new_size The allocate size in bytes that the multiple is to be calculated from.
    \return number of PAGESIZE needed, rouned up.  
   */
  size_t round_up_pagesize(size_t new_size){
    size_t pSize = sysconf(_SC_PAGESIZE);
    size_t multiple = ((size_t)(new_size / pSize));
    if(new_size % pSize) multiple += 1;
    multiple = multiple * pSize;
    return multiple;
  }

  /*! \brief Maps (or re-maps) file to memory.
   * raw_new_size must be the total desired size in bytes for the mapping.
   * If it is sucessfull, the reserved area is set and used as checkpoint memory for the 
   * MemManager.
   * If the files current size exeeds raw_new_size, only the part up to the point of raw_new_size of
   * the file is mapped (new_size is rounded up to nearest PAGESIZE multiple).
   \param raw_new_size The new size in bytes to be reserved.
   \return on sucess, returns true, else false.
  */
  bool reserve_memory(size_t raw_new_size){
    //round requested new size up to nearest page-multiple
    size_t new_size = round_up_pagesize(raw_new_size);

    // make new or attempt remapping of mmap
    bool sucess = true;
    std::string infoStr = "";
    if((this->mm != 0) && (new_size != this->tot_mem)){
      if(mremap(this->mm, this->tot_mem, new_size, 0) == 0){
	infoStr = "remapping failed";
	sucess = false;
      }
    }else{
      if(this->mm == 0){
	this->mm = mmap(0, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
	if(this->mm == 0){
	  infoStr = "mapping failed";
	  sucess = false;
	}
      }
    }
    if(sucess){
      this->tot_mem = new_size;
      _protected_mem_startAddr = this->mm;
      _protected_mem_stopAddr = ((uint8_t*) this->mm) + new_size;
    }else{
      printf("%s\n", infoStr.c_str());
      return sucess;
    }
    
    return sucess;
  }

  /*! \brief Registers the custom checkpointlib segfault handler as the default 
   */
  void _register_segfault_handler(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;
  
    sigaction(SIGSEGV, &sa, NULL);
  }
  
public:
  /*! \brief Initiate the MemManager class.
   * The class constructor itself cannot fail and does not initiate memory allocation or any other
   * recources. For that, initMemManager must be executed.
   \param init_fileNameA The name of one of two checkpoint files. 
   \param init_fileNameB The name of the other of two checkpoint files.
   \return A instance of the MemManager class.
  */
  MemManager(std::string init_fileNameA, std::string init_fileNameB){
    FileManager* FM = new FileManager(init_fileNameA,  init_fileNameB);
    this->file_manager = FM;
  }

  /*! \brief The deconstructor. It frees all system resources MemManager has possibly reserved.
   */
  ~MemManager(){    
    if(this->mm != 0) munmap(this->mm, this->tot_mem);
    if(this->mem_logic != 0) delete this->mem_logic;
    if(this->file_manager != 0) delete this->file_manager;
    _protected_mem_startAddr = 0;
    _protected_mem_stopAddr = 0;
  }

  /*! \brief Initiates the MemManager, allocating memory.
   * Asserts that all system resources required by MemManager (files as well as memory) is
   * accessible. If something required is not available, nothing will be reserved.
   \param expected_size The size initially attempted allocated (in bytes).
   \return on success 0 is returned, else -1.
*/
  int initMemManager(std::size_t expected_size){
    if(this->file_manager->initFileManager(expected_size)){
      printf("fileManager initiation failed\n");
      return -1;
    }

    if(!this->reserve_memory(expected_size)){
      printf("mmap initiation failed\n");
      return -1;
    }
  
    this->mem_logic = new FirstFit(this->mm, this->tot_mem);
    this->_register_segfault_handler();

    return 0;
  }
    
  
  
  /*! \brief allocate a piece of checkpoint memory.
    \param requested_size The size to be allocated.
    \return On success, a pointer to allocated memory is returned, else a NULL pointer.
   */
  void* allocate_memory(size_t requested_size){
    return this->mem_logic->ff_allocate(requested_size);
  }
  
  /*! \brief Frees memory allocated checkpoint memory.
   * Note, that addr must point to a block of memory previously allocated by allocate_memory().
   \param addr The start of the memory block that is to be freed.
   \return 0 on success, -1 on failure
  */
  int free_memory(void* addr){
    return this->mem_logic->ff_free(addr);
  }

  /*! \brief Starts a thread that gradually saves checkpoint memory to disk.
   * For details, see _save_checkpoint_gradually()
   \param lockCheck Mutex passed to _save_checkpoint_gradually()
   \param barrier_flag Flag passed to _save_checkpoint_gradually()
   \return The thread that was just started.
  */
  std::thread start_save_checkpoint(std::mutex* lockCheck, std::uint8_t* barrier_flag){

    std::thread th = std::thread(_save_checkpoint_gradually, lockCheck, barrier_flag,
				 this->mm, this->tot_mem, this->file_manager);
    return th;
  }

  /*! \brief Wrapper for _load_checkpoint(). 
   */
  void start_load_checkpoint(){
    _load_checkpoint(this->mm, this->tot_mem,
		     this->file_manager);
  }

  /*! \brief Returns pointer to the beginning of checkpoint memory. Only used for testing. 
   */
  void* ret_mm(){
    return mm;
  }

  /*! \brief size in bytes of checkpoint memory. Only used for testing. 
   */
  size_t ret_memSize(){
    return this->tot_mem;
  }

};

#endif //MEMMANAGER_HPP
