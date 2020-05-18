#ifndef FILEMANAGERTESTS_HPP
#define FILEMANAGERTESTS_HPP

#include "../lib/fileManager.hpp"
#include "testHelp.hpp"
#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <vector>

#include <stdio.h> // for FILE*, fseek, fopen(),...

int setup_file(std::string lab, std::size_t count, std::size_t size){
  FILE* fo = fopen(lab.c_str(), "wb");
  if(fo == 0){
    std::cout << "fo == 0\n";
    return -1;
  }
  if(ftruncate(fileno(fo), size)){
    fclose(fo);
    std::cout << "ftruncate(fileno(fo), size_a)\n";
    return -1;
  }
  if(fseek(fo, 0, SEEK_SET) != 0){
    fclose(fo);
    std::cout << "fseek(fo, 0, SEEK_SET)\n";
    return -1;
  }
  std::size_t init_counter = count;
  if(fwrite(&init_counter, sizeof(std::size_t), 1, fo) != 1){
    fclose(fo);
    std::cout << "fwrite(&init_counter, sizeof(std::size_t), 1, fo) != 1\n";
    return -1;
  }
  fclose(fo);
  return 0;
}

int read_file_into_buffer(std::string lab, void* buff, std::size_t size){
  FILE* fo = fopen(lab.c_str(), "rb");
  if(fo == 0){
    printf("fo == 0\n");
    return -1;
  }

  if(fseek(fo, sizeof(std::size_t), SEEK_SET) != 0){
    fclose(fo);
    printf("fseek(fo, 0, SEEK_SET)\n");
    return -1;
  }

  if(fread(buff, size, 1, fo) != 1){
    fclose(fo);
    printf("fread(&cpt_counter, sizeof(std::size_t), 1, fo) != 1\n");
    return -1;
  }
  
  fclose(fo);

  return 0;
}

int check_file_count(std::string lab, std::size_t compare_me){
  FILE* fo = fopen(lab.c_str(), "rb");
  if(fo == 0){
    printf("fo == 0\n");
    return -1;
  }

  if(fseek(fo, 0, SEEK_SET) != 0){
    fclose(fo);
    printf("fseek(fo, 0, SEEK_SET)\n");
    return -1;
  }
  std::size_t cpt_counter = 0;
  if(fread(&cpt_counter, sizeof(std::size_t), 1, fo) != 1){
    fclose(fo);
    printf("fread(&cpt_counter, sizeof(std::size_t), 1, fo) != 1\n");
    return -1;
  }
  fclose(fo);
  if(cpt_counter == compare_me){
    return 0;
  }else{
    return -1;
  }
}

int check_file_size(std::string lab, std::size_t compare_me){
  FILE* fo = fopen(lab.c_str(), "rb");
  if(fo == 0){
    printf("fo == 0\n");
    return -1;
  }

  if(fseek(fo, 0, SEEK_SET) != 0){
    fclose(fo);
    printf("fseek(fo, 0, SEEK_SET)\n");
    return -1;
  }

  long int l_start = ftell(fo);
  if(l_start == -1){
    fclose(fo);
    printf("l_start == -1\n");
    return -1;
  }

  if(fseek(fo, 0, SEEK_END) != 0){
    fclose(fo);
    printf("fseek(fo, 0, SEEK_SET)\n");
    return -1;
  }
  
  long int l_stop = ftell(fo);
  if(l_stop == -1){
    fclose(fo);
    printf("l_stop == -1\n");
    return -1;
  }
  
  std::size_t fileSize = l_stop - l_start;
  
  fclose(fo);
  if(fileSize == compare_me){
    return 0;
  }else{
    return -1;
  }
}

std::string cleanup_fmTest(std::string msg){
  return cleanup_test(msg);
}

std::string test0_create_FileManager_obj(){
  FileManager fm(LABEL1, LABEL2);
  return cleanup_fmTest(RET_SUCESS);
}

std::string test1_initialize_FileManager_simple(){
  //initialize filemanager with no existing files
  std::size_t size_reserved = 200;
  std::size_t size_tot = 200 + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  if(check_file_size(LABEL1, size_tot)) return cleanup_fmTest("check_file_size(LABEL1, size_tot)");
  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_size(LABEL2, size_tot)) return cleanup_fmTest("check_file_size(LABEL2, size_tot)");
  if(check_file_count(LABEL2, 0)) return cleanup_fmTest("check_file_count(LABEL2, 0)");
  return cleanup_fmTest(RET_SUCESS);
}

std::string test2_initialize_FileManager_advanced(){
  //initialize filemanager with existing files (label2 will have a larger count than label1)
  std::size_t size_reserved = 200;
  std::size_t size_tot = 200 + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  // initiate files for tests...
  if(setup_file(LABEL1, 44, 20)) return cleanup_fmTest("setup_file(LABEL1, 44, 20)");
  if(setup_file(LABEL2, 50, 20)) return cleanup_fmTest("setup_file(LABEL2, 50, 20)");
  
  // setup finished, start test 
  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  if(check_file_size(LABEL1, size_tot)) return cleanup_fmTest("check_file_size(LABEL1, size_tot)");
  if(check_file_count(LABEL1, 0)) return cleanup_fmTest("check_file_count(LABEL1, 0)");

  if(check_file_size(LABEL2, size_tot)) return cleanup_fmTest("check_file_size(LABEL2, size_tot)");
  if(check_file_count(LABEL2, 1)) return cleanup_fmTest("check_file_count(LABEL2, 1)");
  
  return cleanup_fmTest(RET_SUCESS);
}


std::string test3_resize(){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  std::size_t new_size = size_reserved + 50;
  std::size_t new_size_tot = new_size + sizeof(std::size_t);
  if(fm.expand_file(new_size)) return cleanup_fmTest("expand_file(new_size)");

  if(check_file_size(LABEL1, new_size_tot))
    return cleanup_fmTest("check_file_size(LABEL1, size_tot)");
  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_size(LABEL2, new_size_tot))
    return cleanup_fmTest("check_file_size(LABEL2, size_tot)");
  if(check_file_count(LABEL2, 0)) return cleanup_fmTest("check_file_count(LABEL2, 0)");
  
  return cleanup_fmTest(RET_SUCESS);
}

std::string test4_dont_modify_large_file_size(){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  std::size_t true_size_a = 400;
  std::size_t true_size_b = 600;
  if(setup_file(LABEL1, 66, true_size_a))
    return cleanup_fmTest("setup_file(LABEL2, 50, true_size_a)");
  if(setup_file(LABEL2, 50, true_size_b))
    return cleanup_fmTest("setup_file(LABEL2, 50, true_size_b)");
  
  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  if(check_file_size(LABEL1, true_size_a))
    return cleanup_fmTest("check_file_size(LABEL1, true_size_a)");
  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_size(LABEL2, true_size_b))
    return cleanup_fmTest("check_file_size(LABEL2, true_size_b)");
  if(check_file_count(LABEL2, 0)) return cleanup_fmTest("check_file_count(LABEL2, 0)");
  
  return cleanup_fmTest(RET_SUCESS);
}


std::string test5_save_file_simple(){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  std::size_t buff_size = sizeof(int) * 5;
  int* buff_in = ((int*)malloc(buff_size));
  int* buff_out = (int*)malloc(buff_size);
  vec_malloc.push_back(buff_in);
  vec_malloc.push_back(buff_out);
  for(int i=0; i<5; i++) buff_in[i] = i * 2;
  
  int fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  ssize_t w_ret;
  w_ret = write(fd, buff_in, buff_size);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != buff_size) return cleanup_fmTest("w_ret != buff_size");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  
  
  if(read_file_into_buffer(LABEL2, buff_out, buff_size))
    return cleanup_fmTest("read_file_into_buffer(LABEL1, buff_out, buff_size)");

  if(memcmp(buff_in, buff_out, buff_size))
    return cleanup_fmTest("memcmp(buff_in, buff_out, buff_size)");

  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_count(LABEL2, 2)) return cleanup_fmTest("check_file_count(LABEL2, 2)");
  
  return cleanup_fmTest(RET_SUCESS);
}

std::string test6_load_file_simple(){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  std::size_t buff_size = sizeof(int) * 5;
  int* buff_in = ((int*)malloc(buff_size));
  int* buff_out = (int*)malloc(buff_size);
  vec_malloc.push_back(buff_in);
  vec_malloc.push_back(buff_out);
  for(int i=0; i<5; i++) buff_in[i] = i * 2;
  
  int fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  ssize_t w_ret;
  w_ret = write(fd, buff_in, buff_size);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != buff_size) return cleanup_fmTest("w_ret != buff_size");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  
  if(fm.load_checkpoint(buff_out, buff_size))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, buff_size)");
  
  if(memcmp(buff_in, buff_out, buff_size))
    return cleanup_fmTest("memcmp(buff_in, buff_out, buff_size)");

  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_count(LABEL2, 2)) return cleanup_fmTest("check_file_count(LABEL2, 2)");
  
  return cleanup_fmTest(RET_SUCESS);
}


std::string test7_save_file_many_times(){
  std::size_t num_variables = 50;
  std::size_t size_reserved = sizeof(long int) * num_variables;
  
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  long int* buff_in_1 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_in_1);
  long int* buff_in_2 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_in_2);
  long int* buff_in_3 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_in_3);
  long int* buff_in_4 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_in_4);

  long int* buff_out_1 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_1);
  long int* buff_out_2 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_2);
  long int* buff_out_3 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_3);
  long int* buff_out_4 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_4);

  
  for(unsigned long int i=0; i<num_variables; i++){
    buff_in_1[i] = i;
    buff_in_2[i] = i * 2;
    buff_in_3[i] = i + (i/2);
    buff_in_4[i] = i * i;
  }

  ssize_t w_ret;
  int fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  w_ret = write(fd, buff_in_1, size_reserved);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != size_reserved) return cleanup_fmTest("w_ret != size_reserved");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  if(fm.load_checkpoint(buff_out_1, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in_1, buff_out_1, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");

  fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  w_ret = write(fd, buff_in_2, size_reserved);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != size_reserved) return cleanup_fmTest("w_ret != size_reserved");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  if(fm.load_checkpoint(buff_out_2, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in_2, buff_out_2, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");

  fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  w_ret = write(fd, buff_in_3, size_reserved);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != size_reserved) return cleanup_fmTest("w_ret != size_reserved");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  if(fm.load_checkpoint(buff_out_3, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in_3, buff_out_3, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");

  fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  w_ret = write(fd, buff_in_4, size_reserved);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != size_reserved) return cleanup_fmTest("w_ret != size_reserved");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  if(fm.load_checkpoint(buff_out_4, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in_4, buff_out_4, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");
  
  if(check_file_count(LABEL1, 5)) return cleanup_fmTest("check_file_count(LABEL1, 1)");
  if(check_file_count(LABEL2, 4)) return cleanup_fmTest("check_file_count(LABEL2, 2)");
  
  return cleanup_fmTest(RET_SUCESS);
}


std::string test8_load_save_w_new_fileManager(){
  std::size_t num_variables = 50;
  std::size_t size_reserved = sizeof(long int) * num_variables;
  std::size_t tot_size = size_reserved + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  if(fm.initFileManager(size_reserved) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  long int* buff_in = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_in);

  long int* buff_out_1 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_1);
  long int* buff_out_2 = ((long int*)malloc(size_reserved));
  vec_malloc.push_back(buff_out_2);
  
  for(unsigned long int i=0; i<num_variables; i++){
    buff_in[i] = i * i;
  }

  ssize_t w_ret;
  int fd = fm.save_checkpoint_fd();
  if(fd == -1){
    close(fd);
    return cleanup_fmTest("fd == -1");
  }
  w_ret = write(fd, buff_in, size_reserved);
  if(w_ret == -1) return cleanup_fmTest("w_ret == -1");
  if(((std::size_t)w_ret) != size_reserved) return cleanup_fmTest("w_ret != size_reserved");
  if(fm.save_checkpoint_finalize_save(fd))
    return cleanup_fmTest("fm.save_checkpoint_finalize_save(fd)");
  if(fm.load_checkpoint(buff_out_1, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in, buff_out_1, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");
  
  if(check_file_size(LABEL1, tot_size))
    return cleanup_fmTest("check_file_size(LABEL1, size_reserved)");
  if(check_file_count(LABEL1, 1)) return cleanup_fmTest("check_file_count(LABEL1, 1)");

  if(check_file_size(LABEL2, tot_size))
    return cleanup_fmTest("check_file_size(LABEL2, true_size_b)");
  if(check_file_count(LABEL2, 2)) return cleanup_fmTest("check_file_count(LABEL2, 2)");


  FileManager fm2(LABEL1, LABEL2);

  if(fm2.initFileManager(size_reserved + 50) == -1)
    return cleanup_fmTest("fm.initFileManager(size_reserved)");

  if(fm.load_checkpoint(buff_out_2, size_reserved))
    return cleanup_fmTest("fm.load_checkpoint(buff_out, size_reserved)");
  if(memcmp(buff_in, buff_out_2, size_reserved))
    return cleanup_fmTest("memcmp(buff_in, buff_out, size_reserved)");


  if(check_file_size(LABEL1, tot_size + 50))
    return cleanup_fmTest("check_file_size(LABEL1, true_size_a)");
  if(check_file_count(LABEL1, 0)) return cleanup_fmTest("check_file_count(LABEL1, 0)");

  if(check_file_size(LABEL2, tot_size + 50))
    return cleanup_fmTest("check_file_size(LABEL2, true_size_b)");
  if(check_file_count(LABEL2, 1)) return cleanup_fmTest("check_file_count(LABEL2, 1)");

  return cleanup_fmTest(RET_SUCESS);
}

void runAllFileManagerTests(){
  std::vector<std::string> test_results;
  std::cout << "\n--- Initiating  FileManager tests: --- \n";

  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test0_create_FileManager_obj());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test1_initialize_FileManager_simple());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test2_initialize_FileManager_advanced());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test3_resize());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test4_dont_modify_large_file_size());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test5_save_file_simple());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test6_load_file_simple());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test7_save_file_many_times());
  _delete_test_files(LABEL1, LABEL2);
  test_results.push_back(test8_load_save_w_new_fileManager());
  _delete_test_files(LABEL1, LABEL2);


  std::cout << "\n--- FileManager test results: --- \n";
  for(std::size_t i=0; i<test_results.size(); i++){
    if(RET_SUCESS.compare(test_results[i])){
      std::cout << "test " << i << " finished with: '" << "ERROR -> " + test_results[i] << "' \n";
    }else{
      std::cout << "test " << i << " finished with: '" << test_results[i] << "' \n";
    }
  }
  std::cout << "\n--- finished with FileManager tests --- \n";
  
}

#endif //FILEMANAGERTESTS_HPP

