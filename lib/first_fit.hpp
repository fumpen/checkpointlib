#ifndef FIRST_FIT_HPP
#define FIRST_FIT_HPP

#include <stddef.h>
#include <stdint.h>

#include <iostream>

//#include "checkpointTools.hpp"

class FirstFit{
private:
  std::size_t totalSize;
  void* buffer;
  std::uint8_t sizeof_size_t;
  std::uint8_t memBlockSize;
  
  std::uint8_t MIN_SIZE_FREE_BLOCK = 1;
  
  void* ret_pointer(std::size_t idx){ //pointing to the area after the bestfit data...
    return (((std::uint8_t*) this->buffer) + (idx + this->memBlockSize));
  }

  std::size_t ret_next(std::size_t idx){
    return *((std::size_t*)(((std::uint8_t*) this->buffer) + idx));
  }
  void set_next(std::size_t idx, std::size_t new_size){
    *((std::size_t*)(((std::uint8_t*) this->buffer) + idx)) = new_size;
  }

  
  std::uint8_t ret_free(std::size_t idx){
    return *((std::uint8_t*)(((std::uint8_t*) this->buffer) + (idx + this->sizeof_size_t)));
  }
  void set_free(std::size_t idx, std::uint8_t new_val){
    *((std::uint8_t*)(((std::uint8_t*) this->buffer) + (idx + this->sizeof_size_t))) = new_val;
  }
  
public:
  FirstFit(void* buff_addr, std::size_t init_totalSize){
    this->buffer = buff_addr;
    this->totalSize = init_totalSize;
    this->sizeof_size_t = ((std::uint8_t) sizeof(std::size_t)); 
    this->memBlockSize = this->sizeof_size_t + 1; // add 1 byte for "free"-flag
    set_next(0, this->totalSize);
    set_free(0, 1);
  }

  void* ff_allocate(std::size_t alloc_size){
    std::size_t size_req = alloc_size + this->memBlockSize;
    std::size_t offset = 0;
    std::uint8_t is_free = ret_free(0);
    std::size_t next_offset;
    while(offset < this->totalSize){
      if(is_free){
	next_offset = ret_next(offset);
	if((next_offset - offset) >= size_req){
	  if((next_offset - offset) > (size_req + MIN_SIZE_FREE_BLOCK)){
	    set_free(offset, 0);
	    set_next(offset, offset + size_req);
	    offset += size_req;
	    set_next(offset, next_offset);
	    set_free(offset, 1);
	    return ret_pointer(offset - size_req);
	  }else{
	    set_free(offset, 0);
	    return ret_pointer(offset);
	  }
	}
      }

      offset = ret_next(offset);
      is_free = ret_free(offset);
    }
    return 0;
  }

  // return 0 on sucess, -1 on failure
  int ff_free(void* freeMe){
    std::size_t freeMe_pos = (((std::uint8_t*)freeMe) - ((std::uint8_t*)this->buffer)) - this->memBlockSize;
    //std::cout << "free_pos: " << freeMe_pos << "\n";
    std::size_t offset = 0;
    std::size_t next_offset;
    std::size_t prev_offset = this->totalSize;
    while(offset < this->totalSize){
      if(freeMe_pos == offset){
	set_free(offset, 1);
	next_offset = ret_next(offset);
	if((ret_free(next_offset)) && (next_offset < this->totalSize)){ //free block after freeMe?
	  set_next(offset, ret_next(next_offset));
	}
	if((ret_free(prev_offset)) && (prev_offset < this->totalSize)){ //free block before freeMe?
	  set_next(prev_offset, ret_next(offset));
	}
	return 0;
      }

      prev_offset = offset;
      offset = ret_next(offset);
    }
    return -1;
  }

  // return 0 on sucess
  int ff_adjust_allocation_size(std::size_t new_size){
    std::size_t offset = 0;
    std::size_t prev_offset;
    if(new_size < this->totalSize){
      while((offset + MIN_SIZE_FREE_BLOCK) <= new_size){
	prev_offset = offset;
	offset = ret_next(offset);
	
      }
      this->totalSize = new_size;
       set_next(prev_offset, new_size);
    }else{
      
       while(offset < this->totalSize){
	 prev_offset = offset;
	 offset = ret_next(offset);
      }
       this->totalSize = new_size;
       set_next(prev_offset, new_size);
    }
    return 0;
  }
  
    
};


#endif //FIRST_FIT_HPP
