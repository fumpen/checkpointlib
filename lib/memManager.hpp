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
  
void _load_checkpoint(void* memAddr, size_t memSize,
		      FileManager* FM){  
  if(FM->load_checkpoint(memAddr, memSize)){
    printf("ERROR LOADING\n");
  }
}


class MemManager{
private:
  void* mm = 0;
  size_t tot_mem;
  FirstFit* mem_logic = 0;
  FileManager* file_manager = 0;
  
  size_t round_up_pagesize(size_t new_size){
    size_t pSize = sysconf(_SC_PAGESIZE);
    size_t multiple = ((size_t)(new_size / pSize));
    if(new_size % pSize) multiple += 1;
    multiple = multiple * pSize;
    return multiple;
  }
  
  // maps (or re-maps) file. 'new_size' must be the total desired size in bytes for the mapping
  // if the files current size exeeds 'new_size' only the part up to the point of 'new_size' of
  // the file is mapped (new_size is rounded up to nearest page_size'd multiple)
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

  void _register_segfault_handler(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;
  
    sigaction(SIGSEGV, &sa, NULL);
  }
  
public:
  MemManager(std::string init_fileNameA, std::string init_fileNameB){
    FileManager* FM = new FileManager(init_fileNameA,  init_fileNameB);
    this->file_manager = FM;
  }

  ~MemManager(){    
    if(this->mm != 0) munmap(this->mm, this->tot_mem);
    if(this->mem_logic != 0) delete this->mem_logic;
    if(this->file_manager != 0) delete this->file_manager;
    _protected_mem_startAddr = 0;
    _protected_mem_stopAddr = 0;
  }

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
    
  
  
  // returns NULL pointer on failure (undefined behavior)
  void* allocate_memory(size_t requested_size){
    return this->mem_logic->ff_allocate(requested_size);
  }

  // return 0 on sucess, -1 on failure
  int free_memory(void* addr){
    return this->mem_logic->ff_free(addr);
  }

  std::thread start_save_checkpoint(std::mutex* lockCheck, std::uint8_t* barrier_flag){

    std::thread th = std::thread(_save_checkpoint_gradually, lockCheck, barrier_flag,
				 this->mm, this->tot_mem, this->file_manager);
    return th;
  }

  void start_load_checkpoint(){
    _load_checkpoint(this->mm, this->tot_mem,
		     this->file_manager);
  }
  
  void* ret_mm(){
    return mm;
  }

  size_t ret_memSize(){
    return this->tot_mem;
  }

};

#endif //MEMMANAGER_HPP
