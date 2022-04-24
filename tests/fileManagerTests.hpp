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


class FileManagerTests : public ::testing::Test {
protected:
  
  //void SetUp() override {}

  void TearDown() override {
    cleanup_test(RET_SUCESS);
    _delete_test_files(LABEL1, LABEL2);
    
  }
};


TEST_F(FileManagerTests, test0_create_FileManager_obj) {
  FileManager fm(LABEL1, LABEL2);
}

TEST_F(FileManagerTests, test1_initialize_FileManager_simple){
  //initialize filemanager with no existing files
  std::size_t size_reserved = 200;
  std::size_t size_tot = 200 + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);
  
  ASSERT_FALSE(check_file_size(LABEL1, size_tot));
  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_size(LABEL2, size_tot));
  ASSERT_FALSE(check_file_count(LABEL2, 0));
}

TEST_F(FileManagerTests, test2_initialize_FileManager_advanced){
  //initialize filemanager with existing files (label2 will have a larger count than label1)
  std::size_t size_reserved = 200;
  std::size_t size_tot = 200 + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  // initiate files for tests...
  ASSERT_FALSE(setup_file(LABEL1, 44, 20));
  ASSERT_FALSE(setup_file(LABEL2, 50, 20));
  
  // setup finished, start test 
  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);
  
  ASSERT_FALSE(check_file_size(LABEL1, size_tot));
  ASSERT_FALSE(check_file_count(LABEL1, 0));

  ASSERT_FALSE(check_file_size(LABEL2, size_tot));
  ASSERT_FALSE(check_file_count(LABEL2, 1));

}


TEST_F(FileManagerTests, test3_resize){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);
  
  std::size_t new_size = size_reserved + 50;
  std::size_t new_size_tot = new_size + sizeof(std::size_t);
  ASSERT_FALSE(fm.expand_file(new_size));

  ASSERT_FALSE(check_file_size(LABEL1, new_size_tot));
  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_size(LABEL2, new_size_tot));
  ASSERT_FALSE(check_file_count(LABEL2, 0));
}

TEST_F(FileManagerTests, test4_dont_modify_large_file_size){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  std::size_t true_size_a = 400;
  std::size_t true_size_b = 600;
  ASSERT_FALSE(setup_file(LABEL1, 66, true_size_a));
  ASSERT_FALSE(setup_file(LABEL2, 50, true_size_b));
  
  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);

  ASSERT_FALSE(check_file_size(LABEL1, true_size_a));
  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_size(LABEL2, true_size_b));
  ASSERT_FALSE(check_file_count(LABEL2, 0));
}


TEST_F(FileManagerTests, test5_save_file_simple){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);

  std::size_t buff_size = sizeof(int) * 5;
  int* buff_in = ((int*)malloc(buff_size));
  int* buff_out = (int*)malloc(buff_size);
  vec_malloc.push_back(buff_in);
  vec_malloc.push_back(buff_out);
  for(int i=0; i<5; i++) buff_in[i] = i * 2;
  
  int fd = fm.save_checkpoint_fd();
  ASSERT_FALSE(fd == -1);
  
  ssize_t w_ret;
  w_ret = write(fd, buff_in, buff_size);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != buff_size);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  
  
  ASSERT_FALSE(read_file_into_buffer(LABEL2, buff_out, buff_size));

  ASSERT_FALSE(memcmp(buff_in, buff_out, buff_size));

  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_count(LABEL2, 2));
}

  
TEST_F(FileManagerTests, test6_load_file_simple){
  std::size_t size_reserved = 200;
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);
  
  std::size_t buff_size = sizeof(int) * 5;
  int* buff_in = ((int*)malloc(buff_size));
  int* buff_out = (int*)malloc(buff_size);
  vec_malloc.push_back(buff_in);
  vec_malloc.push_back(buff_out);
  for(int i=0; i<5; i++) buff_in[i] = i * 2;
  
  int fd = fm.save_checkpoint_fd();
  ASSERT_FALSE(fd == -1);
  
  ssize_t w_ret;
  w_ret = write(fd, buff_in, buff_size);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != buff_size);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  
  ASSERT_FALSE(fm.load_checkpoint(buff_out, buff_size));
  
  ASSERT_FALSE(memcmp(buff_in, buff_out, buff_size));

  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_count(LABEL2, 2));
}


TEST_F(FileManagerTests, test7_save_file_many_times){
  std::size_t num_variables = 50;
  std::size_t size_reserved = sizeof(long int) * num_variables;
  
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);

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
  ASSERT_FALSE(fd == -1);
  
  w_ret = write(fd, buff_in_1, size_reserved);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != size_reserved);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  ASSERT_FALSE(fm.load_checkpoint(buff_out_1, size_reserved));
  ASSERT_FALSE(memcmp(buff_in_1, buff_out_1, size_reserved));

  fd = fm.save_checkpoint_fd();
  ASSERT_FALSE(fd == -1);
  
  w_ret = write(fd, buff_in_2, size_reserved);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != size_reserved);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  ASSERT_FALSE(fm.load_checkpoint(buff_out_2, size_reserved));
  ASSERT_FALSE(memcmp(buff_in_2, buff_out_2, size_reserved));

  fd = fm.save_checkpoint_fd();
  ASSERT_FALSE(fd == -1);
  
  w_ret = write(fd, buff_in_3, size_reserved);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != size_reserved);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  ASSERT_FALSE(fm.load_checkpoint(buff_out_3, size_reserved));
  ASSERT_FALSE(memcmp(buff_in_3, buff_out_3, size_reserved));

  fd = fm.save_checkpoint_fd();
  ASSERT_FALSE(fd == -1);
  
  w_ret = write(fd, buff_in_4, size_reserved);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != size_reserved);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  ASSERT_FALSE(fm.load_checkpoint(buff_out_4, size_reserved));
  ASSERT_FALSE(memcmp(buff_in_4, buff_out_4, size_reserved));
  
  ASSERT_FALSE(check_file_count(LABEL1, 5));
  ASSERT_FALSE(check_file_count(LABEL2, 4));
}


TEST_F(FileManagerTests, test8_load_save_w_new_fileManager){
  std::size_t num_variables = 50;
  std::size_t size_reserved = sizeof(long int) * num_variables;
  std::size_t tot_size = size_reserved + sizeof(std::size_t);
  FileManager fm(LABEL1, LABEL2);

  ASSERT_FALSE(fm.initFileManager(size_reserved) == -1);

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
  ASSERT_FALSE(fd == -1);
  
  w_ret = write(fd, buff_in, size_reserved);
  ASSERT_FALSE(w_ret == -1);
  ASSERT_FALSE(((std::size_t)w_ret) != size_reserved);
  ASSERT_FALSE(fm.save_checkpoint_finalize_save(fd));
  ASSERT_FALSE(fm.load_checkpoint(buff_out_1, size_reserved));
  ASSERT_FALSE(memcmp(buff_in, buff_out_1, size_reserved));
  
  ASSERT_FALSE(check_file_size(LABEL1, tot_size));
  ASSERT_FALSE(check_file_count(LABEL1, 1));

  ASSERT_FALSE(check_file_size(LABEL2, tot_size));
  ASSERT_FALSE(check_file_count(LABEL2, 2));


  FileManager fm2(LABEL1, LABEL2);

  ASSERT_FALSE(fm2.initFileManager(size_reserved + 50) == -1);

  ASSERT_FALSE(fm.load_checkpoint(buff_out_2, size_reserved));
  ASSERT_FALSE(memcmp(buff_in, buff_out_2, size_reserved));


  ASSERT_FALSE(check_file_size(LABEL1, tot_size + 50));
  ASSERT_FALSE(check_file_count(LABEL1, 0));

  ASSERT_FALSE(check_file_size(LABEL2, tot_size + 50));
  ASSERT_FALSE(check_file_count(LABEL2, 1));

}

/*
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
*/
  
#endif //FILEMANAGERTESTS_HPP

