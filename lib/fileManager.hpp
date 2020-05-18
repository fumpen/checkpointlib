#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <cstring> //for strerror
//#include <errno.h> // for errorno...


/*
  Handles Interactions with the filesystem. 
  
  undefined behavior if:
  one or more save-file is deleted after FileManager initialization
  the files in question exceeds what size can be represented as a std::ssize or off_t
  if size increases this threshold, 
  
*/
class FileManager {
private:
  typedef long long int llint;
  
  std::string fileNameA;
  std::string fileNameB;
  std::string* newest_save;
  std::string* oldest_save;
  std::uint8_t sizeof_counter;
  std::size_t checkpoint_save_counter;
  
  void cpt_perror(std::string err_msg){
    std::string full_msg = "CheckpointLib-> " + err_msg;
    perror(full_msg.c_str());
  }

  void cpt_printf(std::string msg){
    std::string full_msg = "CheckpointLib-> " + msg + "\n";
    printf("%s", full_msg.c_str());
  }

  // returns -1 on fail, 0 on sucess
  int closeFile(int fd){
    int ret = close(fd);
    if(ret) this->cpt_perror("err closing file");
    return ret;
  }

  //return -1 on error
  llint ret_file_size(int fd){
    struct stat sb;
    int ret = fstat(fd, &sb);
    if(ret){
      cpt_perror("err reading fstat()");
      return -1;
    }
    return sb.st_size;
  }

  std::size_t max_size_allowed(){
    std::uint8_t* bytes = (std::uint8_t*)malloc(sizeof(std::size_t));
    std::uint8_t max_byte = 0xFF;
    for(std::size_t i=0; i< sizeof(ssize_t); i++){
      memcpy(&bytes[i], &max_byte, 1);
    }
    std::size_t res1 = *((std::size_t*)bytes);
    free(bytes);

    bytes = (std::uint8_t*)malloc(sizeof(std::size_t));
    for(std::size_t i=0; i< sizeof(off_t); i++){
      memcpy(&bytes[i], &max_byte, 1);
    }
    std::size_t res2 = *((std::size_t*)bytes);
    free(bytes);

    if(res1 < res2){
      return res1 / 2;
    }else{
      return res2 / 2;
    }
  }
  
public:
  // do not shrink file size, it cannot be known if it increases later
  FileManager(std::string init_fileNameA, std::string init_fileNameB){
    this->fileNameA = init_fileNameA;
    this->fileNameB = init_fileNameB;
    this->sizeof_counter = sizeof(std::size_t);
  }

  llint fm_read(int fd, void *buf, size_t count, size_t offset){
    lseek(fd, offset, SEEK_SET);
    return read(fd, buf, count);
  }

  //creates/expands files, sets count and new/old pointers. Returns 0 on sucess
  int initFileManager(std::size_t size){
    std::size_t size_w_counter = size + this->sizeof_counter;
    if(this->max_size_allowed() < size_w_counter){
      this->cpt_printf("requested size exeeds MAX supported size");
      return -1;
    }
    
    int fd1;
    if(access(this->fileNameA.c_str(), R_OK | W_OK)){
      fd1 = open(this->fileNameA.c_str(), O_RDWR | O_CREAT, S_IRWXU);
      if(fd1 == -1){
	this->cpt_perror("could not create file");
	return -1;
      }
    }else{
      fd1 = open(this->fileNameA.c_str(), O_RDWR);
      if(fd1 == -1){
	this->cpt_perror("could not open file");
	return -1;
      }
    }
    
    llint fsize = ret_file_size(fd1);
    if(fsize < 0) return -1;
    if(((std::size_t)fsize) < size_w_counter){ // expand file to match size
      if(ftruncate(fd1, size_w_counter)){
	this->cpt_perror("ftruncate() failed");
	this->closeFile(fd1);
	return -1;
      }
    }

    std::size_t counter_1 = 0;
    lseek(fd1, 0, SEEK_SET);
    llint check_counter = read(fd1, &counter_1, this->sizeof_counter);
    if((check_counter == -1) || (check_counter != this->sizeof_counter)){
      if(check_counter == -1){
	this->cpt_perror("read() failed");
      }else{
	this->cpt_printf("read() did not return expected amount of bytes");
      }
      this->closeFile(fd1);
      return -1;
    }
    
    int fd2;
    if(access(this->fileNameB.c_str(), R_OK | W_OK)){
      fd2 = open(this->fileNameB.c_str(), O_RDWR | O_CREAT, S_IRWXU);
      if(fd2 == -1){
	this->cpt_perror("could not create file");
	return -1;
      }
    }else{
      fd2 = open(this->fileNameB.c_str(), O_RDWR);
      if(fd2 == -1){
	this->cpt_perror("could not open file");
	return -1;
      }
    }
    
    fsize = ret_file_size(fd2);
    if(fsize < 0) return -1;
    if(((std::size_t)fsize) < size_w_counter){ // expand file to match size
      if(ftruncate(fd2, size_w_counter)){
	this->cpt_perror("ftruncate() failed");
	this->closeFile(fd1);
	this->closeFile(fd2);
	return -1;
      }
    }
    
    std::size_t counter_2 = 0;
    lseek(fd2, 0, SEEK_SET);
    check_counter = read(fd2, &counter_2, this->sizeof_counter);
    if((check_counter == -1) || (check_counter != this->sizeof_counter)){
      if(check_counter == -1){
	this->cpt_perror("read() failed");
      }else{
	this->cpt_printf("read() did not return expected amount of bytes");
      }
      this->closeFile(fd1);
      this->closeFile(fd2);
      return -1;
    }

    std::size_t new_count;
    int new_fd;
    int old_fd;
    //printf("fd1: %lu, fd2: %lu\n", counter_1, counter_2);
    if(counter_1 >= counter_2){
      new_fd = fd1;
      old_fd = fd2;
      this->newest_save = &fileNameA;
      this->oldest_save = &fileNameB;
    }else{
      new_fd = fd2;
      old_fd = fd1;
      this->newest_save = &fileNameB;
      this->oldest_save = &fileNameA;
    }
    this->checkpoint_save_counter = 1;
    new_count = 1;
    lseek(new_fd, 0, SEEK_SET);
    check_counter = write(new_fd, &new_count, this->sizeof_counter);
    if((check_counter == -1) || (check_counter != this->sizeof_counter)){
      if(check_counter == -1) this->cpt_perror("write() failed");
      if(check_counter != this->sizeof_counter)
	this->cpt_printf("write() did not return expected amount of bytes");
      this->closeFile(fd1);
      this->closeFile(fd2);
      return -1;
    }
    new_count = 0;
    lseek(old_fd, 0, SEEK_SET);
    check_counter = write(old_fd, &new_count, this->sizeof_counter);
    if((check_counter == -1) || (check_counter != this->sizeof_counter)){
      if(check_counter == -1) this->cpt_perror("write() failed");
      if(check_counter != this->sizeof_counter)
	this->cpt_printf("write() did not return expected amount of bytes");
      this->closeFile(fd1);
      this->closeFile(fd2);
      return -1;
    }

    this->closeFile(fd1);
    this->closeFile(fd2);
    return 0;
  }

  /* if current file-size is found to be smaller than new_size, expand file to 
     new_size. If file-size >= current size, it does nothing.
     return: 0 on sucess, -1 on failure.
   */  
  int expand_file(std::size_t new_size){
    std::size_t size_w_counter = new_size + this->sizeof_counter;
    if(this->max_size_allowed() < size_w_counter){
      this->cpt_printf("requested size exeeds MAX supported size");
      return -1;
    }
    int fd;
    if(access(this->fileNameA.c_str(), R_OK | W_OK)){
      this->cpt_perror("file not available, acess()");
      return -1;
    }else{
      fd = open(this->fileNameA.c_str(), O_RDONLY | O_WRONLY);
      if(fd == -1){
	this->cpt_perror("could not open file");
	return -1;
      }
    }
    
    llint fsize = ret_file_size(fd);
    if(fsize < 0) return -1;
    if(((std::size_t)fsize) < size_w_counter){ // expand file to match size
      if(ftruncate(fd, size_w_counter)){
	this->cpt_perror("ftruncate() failed");
	this->closeFile(fd);
	return -1;
      }
    }
    this->closeFile(fd);
    
  
    if(access(this->fileNameB.c_str(), R_OK | W_OK)){
      this->cpt_perror("file not available, acess()");
      return -1;
    }else{
      fd = open(this->fileNameB.c_str(), O_RDONLY | O_WRONLY);
      if(fd == -1){
	this->cpt_perror("could not open file");
	return -1;
      }
    }
    
    fsize = ret_file_size(fd);
    if(fsize < 0) return -1;
    if(((std::size_t)fsize) < size_w_counter){ // expand file to match size
      if(ftruncate(fd, size_w_counter)){
	this->cpt_perror("ftruncate() failed");
	this->closeFile(fd);
	return -1;
      }
    }
    this->closeFile(fd);
    return 0;
  }

  
  int load_checkpoint(void* buff, std::size_t size){
    if(access(this->newest_save->c_str(), R_OK)){
      this->cpt_printf("file not acessible");
      return -1;
    }
    int fd = open(this->newest_save->c_str(), O_RDONLY);
    if(fd == -1){
      this->cpt_perror("err opening file ()");
    }

    off_t seek_ret = lseek(fd, this->sizeof_counter, SEEK_SET); 
    if(seek_ret == -1){
      this->cpt_perror("err repositioning offset into file lseek()");
      this->closeFile(fd);
      return -1;
    }
    
    llint ret = 0;
    std::size_t count = 0;
    while(count < size){
      ret = read(fd, buff, (size - count));
      if(ret == -1){
	this->cpt_perror("err reading from file");
	this->closeFile(fd);
	return -1;
      }
      count += ret;
    }
    this->closeFile(fd);
    return 0;
  }
  
  int save_checkpoint_fd(){
    if(access(this->oldest_save->c_str(), W_OK)){
      this->cpt_printf("file not acessible");
      return -1;
    }
    
    // O_DSYNC = guarantees that when write() returns, data is committed to file
    int fd = open(this->oldest_save->c_str(), O_WRONLY | O_DSYNC); 
    if(fd == -1){
      this->cpt_perror("err opening file ()");
      this->closeFile(fd);
      return -1;
    }

    off_t seek_ret = lseek(fd, this->sizeof_counter, SEEK_SET); 
    if(seek_ret == -1){
      this->cpt_perror("err repositioning offset into file lseek()");
      this->closeFile(fd);
      return -1;
    }
    return fd;
    //
  }

  int save_checkpoint_finalize_save(int fd){
    off_t seek_ret = lseek(fd, 0, SEEK_SET); 
    if(seek_ret == -1){
      this->cpt_perror("err repositioning offset into file lseek()");
      this->closeFile(fd);
      return -1;
    }
    this->checkpoint_save_counter += 1;
    if(write(fd, &this->checkpoint_save_counter, this->sizeof_counter) != this->sizeof_counter){
      this->cpt_perror("err committing new save-count, write()");
      this->closeFile(fd);
      return -1;
    }
    this->closeFile(fd);
    std::string* tmp_ptr = this->oldest_save;
    this->oldest_save = this->newest_save;
    this->newest_save = tmp_ptr;
    return 0;
  }
  
};


#endif //FILEMANAGER_HPP
