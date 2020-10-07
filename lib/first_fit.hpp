/*! \file first_fit.hpp 
 * This file contains the FirstFit class, which purpose it is to govern a block of memory 
 * according to the first-fit strategy.
*/
#ifndef FIRST_FIT_HPP
#define FIRST_FIT_HPP

#include <stddef.h>
#include <stdint.h>

#include <iostream>


/*! \brief The FirstFit class implements the FirstFit logic on a block of memory. Each allocation
 * has additionally sizeof(size_t) bytes extra allocated in each allocation, pointing to
 * the beginning of the next allocation. Note, this means that in practice, this means that 
 * there is a hidden cost of sizeof(size_t) + 1  bytes for each allocation (a byte-flag denoting
 * whether the block is free or not is also included).
 */
class FirstFit{
private:
  std::size_t totalSize; //<! Size in bytes of the block of memory pointed to by buffer. 
  void* buffer; //<! Points to the beginning of the block of memory governed by FirstFit.
  std::uint8_t sizeof_size_t; //<! Represents sizeof(size_t)
  std::uint8_t memBlockSize; //<! 
  
  std::uint8_t MIN_SIZE_FREE_BLOCK = 1; //<! Denotes the minimum size in bytes that a block must have as not to be included in a bordering  allocation. 

  /*! \brief Returns pointer to the allocation that resides after the first-fit logistic data.
    \param idx The offset into buffer a allocation resides in.
    \return A pointer.
  */
  void* ret_pointer(std::size_t idx){
    return (((std::uint8_t*) this->buffer) + (idx + this->memBlockSize));
  }

  /*! \brief Returns offset to the next allocation after the one residing at index.
    \param idx The offset into buffer that a allocation resides.
    \return The offset into buffer where the next allocation begins.
  */
  std::size_t ret_next(std::size_t idx){
    return *((std::size_t*)(((std::uint8_t*) this->buffer) + idx));
  }

  /*! \brief Sets the offset for the allocation at index pointing to the next allocation.
    \param idx The offset into buffer a allocation resides.
    \param new_size The new offset.
  */
  void set_next(std::size_t idx, std::size_t new_size){
    *((std::size_t*)(((std::uint8_t*) this->buffer) + idx)) = new_size;
  }

  /*! \brief Returns flag telling whether allocation residing at index is free or reserved memory.
    \param idx The offset into buffer that a allocation resides.
    \return The status flag of the allocation. (non-zero value if allocation is free).
  */
  std::uint8_t ret_free(std::size_t idx){
    return *((std::uint8_t*)(((std::uint8_t*) this->buffer) + (idx + this->sizeof_size_t)));
  }

  /*! \brief Set the status flag of allocation residing at idx.
    \param idx The offset into buffer that a allocation resides.
    \param new_val The new status of the allocation at idx.
  */
  void set_free(std::size_t idx, std::uint8_t new_val){
    *((std::uint8_t*)(((std::uint8_t*) this->buffer) + (idx + this->sizeof_size_t))) = new_val;
  }
  
public:
  /*! \brief The class constructor of FirstFit.
   * The FirstFit class neither allocates the block of memory it governs, nor does it allocate
   * nodes to keep track of what allocations exists. It is provided a block of memory and
   * writes all allocations along with logistic information needed directly into the 
   * block of memory.
   \param buff_addr The starting address of the block of memory to be governed by the FirstFit logic.
   \param init_totalSize The size in bytes of the block of memory pointed to by buff_addr.
   \return A instance of the FirstFit class.
  */
  FirstFit(void* buff_addr, std::size_t init_totalSize){
    this->buffer = buff_addr;
    this->totalSize = init_totalSize;
    this->sizeof_size_t = ((std::uint8_t) sizeof(std::size_t)); 
    this->memBlockSize = this->sizeof_size_t + 1; // add 1 byte for "free"-flag
    set_next(0, this->totalSize);
    set_free(0, 1);
  }

  /*! \brief Allocates a block of memory.
   \param alloc_size The size in bytes that should be allocated.
   \return On success, a pointer, else a null pointer.
  */
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

  /*! \brief Frees a previously allocated block of memory.
   \param freeMe Must be a pointer previously returned by ff_allocate().
   \return On success, 0, else -1.
  */
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

  /*! \brief Adjusts the size of the block of memory block governed by FristFit.
   * Note that if the the new size provided is smaller than the current size, the
   * part of the memory that does not fit inside the new size (always located in the end 
   * of the block), will be cut off. 
   \param new_size The new size of the buffer.
   \return Cannot fail and always returns 0.
  */
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
