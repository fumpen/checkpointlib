#ifndef CHECKPOINTTESTS_HPP
#define CHECKPOINTTESTS_HPP

#include "../lib/checkpoint.hpp"
#include "testHelp.hpp"

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <thread>


class CheckpointOrganizerTests : public ::testing::Test {
protected:
  CheckpointOrganizer* checkOrgCleanupRef = nullptr;
  //void SetUp() override {}

  void TearDown() override {
    if(checkOrgCleanupRef == nullptr){
      _delete_test_files(LABEL1, LABEL2);
    }else{ 
      delete checkOrgCleanupRef; 
      _delete_test_files(LABEL1 , LABEL2);
    }
  }
};


TEST_F(CheckpointOrganizerTests, test0_create_checkpointManager){
  uint8_t threadTotal = 1;
  bool load_first = false;
  std::size_t alloc_size = 2000;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);

  ASSERT_FALSE(checkOrg.initCheckpointOrganizer(threadTotal, load_first, alloc_size));
}

void _test1_f(void* data, Checkpoint* chkpt){
  (void) data;
  (void) chkpt;
  printf("hey, function where nothing happens was called\n");
}

TEST_F(CheckpointOrganizerTests, test1_start_single_thread){
  uint8_t threadTotal = 1;
  bool load_first = false;
  std::size_t alloc_size = 2000;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  ASSERT_FALSE(checkOrg.initCheckpointOrganizer(threadTotal, load_first, alloc_size));

  void (*test_func)(void *, Checkpoint*) = _test1_f;

  int test_dat1 = 1;
  std::thread* t1 = checkOrg.startThread(test_func, &test_dat1);
  ASSERT_TRUE(t1->joinable());
  t1->join();

}


void  _test2_f(void* data, Checkpoint* chkpt){
  (void) chkpt;
  *(int*) data = 0;
  for(unsigned int i=0; i< 10000000; i++){}
}

TEST_F(CheckpointOrganizerTests, test2_start_many_threads){
  uint8_t threadTotal = 4;
  bool load_first = false;
  std::size_t alloc_size = 2000;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  ASSERT_FALSE(checkOrg.initCheckpointOrganizer(threadTotal, load_first, alloc_size));
  
  void (*test_func)(void *, Checkpoint*) = _test2_f;

  int test_dat1 = 1;
  checkOrg.startThread(test_func, &test_dat1);

  int test_dat2 = 2;
  checkOrg.startThread(test_func, &test_dat2);

  int test_dat3 = 3;
  checkOrg.startThread(test_func, &test_dat3);

  int test_dat4 = 4;
  checkOrg.startThread(test_func, &test_dat4);
  
  checkOrg.joinAll();
  ASSERT_FALSE(test_dat1);
  ASSERT_FALSE(test_dat2);
  ASSERT_FALSE(test_dat3);
  ASSERT_FALSE(test_dat4);
 
}

void  _test3_f(void* data, Checkpoint* chkpt){
  for(unsigned int i=0; i< 100000; i++){
    if((i % 10000) == 0){
      chkpt->check();
    }
    *(long int*) data = (long int)i;
  }
}

TEST_F(CheckpointOrganizerTests, test3_start_threads_with_checkpoint_save){
  uint8_t threadTotal = 4;
  bool load_first = false;
  std::size_t alloc_size = 2000;
  CheckpointOrganizer checkOrg(LABEL1, LABEL2);
  ASSERT_FALSE(checkOrg.initCheckpointOrganizer(threadTotal, load_first, alloc_size));
  
  void (*test_func)(void *, Checkpoint*) = _test3_f;

  long int* test_dat1 = (long int*) checkOrg.check_alloc(sizeof(long int));
  checkOrg.startThread(test_func, test_dat1);

  long int* test_dat2 = (long int*) checkOrg.check_alloc(sizeof(long int));
  checkOrg.startThread(test_func, test_dat2);

  long int* test_dat3 = (long int*) checkOrg.check_alloc(sizeof(long int));
  checkOrg.startThread(test_func, test_dat3);

  long int* test_dat4 = (long int*) checkOrg.check_alloc(sizeof(long int));
  checkOrg.startThread(test_func, test_dat4);

  
  checkOrg.joinAll();
  ASSERT_FALSE(*test_dat1 != 99999);
  ASSERT_FALSE(*test_dat2 != 99999);
  ASSERT_FALSE(*test_dat3 != 99999);
  ASSERT_FALSE(*test_dat4 != 99999);
  
}


void  _test4_f(void* data, Checkpoint* chkpt){
  unsigned int local_cpt_count = 0;
  unsigned int* i = ((unsigned int*) data);
  unsigned int* cpt_counter = (unsigned int*)(((uint8_t*) data) + sizeof(unsigned int));
  *i=0;
  while(*i<10000){
    *i+=1;
    if((*i % 1000) == 0){
      local_cpt_count += 1;
      chkpt->check();
      *cpt_counter = local_cpt_count;
    }
  }
}



TEST_F(CheckpointOrganizerTests, test4_start_threads_with_checkpoint_save_and_load){
  uint8_t threadTotal = 4;
  bool load_first = false;

  std::size_t each_allocatioin = (sizeof(unsigned int) * 2);
  std::size_t alloc_size = (each_allocatioin + ff_alloc_size) *
    threadTotal + 9;
  CheckpointOrganizer* checkOrg = new CheckpointOrganizer(LABEL1, LABEL2);
  checkOrgCleanupRef = checkOrg;
  ASSERT_FALSE(checkOrg->initCheckpointOrganizer(threadTotal, load_first, alloc_size));

  void (*test_func)(void *, Checkpoint*) = _test4_f;

  void* test_dat1 = checkOrg->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat1 == 0);
  void* test_dat2 = checkOrg->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat2 == 0);
  void* test_dat3 = checkOrg->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat3 == 0);
  void* test_dat4 = checkOrg->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat4 == 0);
  
  checkOrg->startThread(test_func, test_dat1);
  checkOrg->startThread(test_func, test_dat2);
  checkOrg->startThread(test_func, test_dat3);
  checkOrg->startThread(test_func, test_dat4);

  checkOrg->joinAll();
  unsigned int test5iterations = 10000;
  unsigned int test5checkpoint_calls = 10; 

  ASSERT_FALSE(*((unsigned int*) test_dat1) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat2) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat3) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat4) != test5iterations);
  
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat1) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat2) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat3) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat4) + sizeof(unsigned int))) != test5checkpoint_calls);

  delete checkOrg;


  // loading instead!
  CheckpointOrganizer* checkOrg2 = new CheckpointOrganizer(LABEL1, LABEL2);
  checkOrgCleanupRef = checkOrg2;
  ASSERT_FALSE(checkOrg2->initCheckpointOrganizer(threadTotal, true, alloc_size));
  
  
  test_dat1 = checkOrg2->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat1 == 0);
  test_dat2 = checkOrg2->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat2 == 0);
  test_dat3 = checkOrg2->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat3 == 0);
  test_dat4 = checkOrg2->check_alloc(each_allocatioin);
  ASSERT_FALSE(test_dat4 == 0);
  
  checkOrg2->startThread(test_func, test_dat1);
  checkOrg2->startThread(test_func, test_dat2);
  checkOrg2->startThread(test_func, test_dat3);
  checkOrg2->startThread(test_func, test_dat4);

  checkOrg2->joinAll();

  test5checkpoint_calls = 1; // NOTE that only one is expected, because were resuming at the end of the loop!!! 
  ASSERT_FALSE(*((unsigned int*) test_dat1) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat2) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat3) != test5iterations);
  ASSERT_FALSE(*((unsigned int*) test_dat4) != test5iterations);
  
  

  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat1) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat2) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat3) + sizeof(unsigned int))) != test5checkpoint_calls);
  ASSERT_FALSE(*((unsigned int*)(((uint8_t*) test_dat4) + sizeof(unsigned int))) != test5checkpoint_calls);
 
}


struct test5Dat {
 unsigned long int iterations;
 unsigned long int step;
 char* mem;
 };

void  _test5_count_up_f(void* data, Checkpoint* chkpt){
  test5Dat dat = *((test5Dat*) data);
  chkpt->check();
  for(unsigned long int i=0; i<dat.iterations; i += dat.step){
    if(i < dat.iterations){
      dat.mem[i] = 120;
      printf("count up\n");
    }
  }
}

void  _test5_count_down_f(void* data, Checkpoint* chkpt){
  test5Dat dat = *((test5Dat*) data);
  chkpt->check();
  for(long int i=dat.iterations; i>0; i -= dat.step){
    if(i > 0){
      dat.mem[i] = 120;
      printf("count down\n");
    }
  }
}

TEST_F(CheckpointOrganizerTests, test5_visual_check_memprot_works){
  uint8_t threadTotal = 4;
  bool load_first = false;

  std::size_t alloc_size = 1000000;
  std::size_t each_alloc = alloc_size - 50;
  CheckpointOrganizer* checkOrg = new CheckpointOrganizer(LABEL1, LABEL2);
  checkOrgCleanupRef = checkOrg;
  ASSERT_FALSE(checkOrg->initCheckpointOrganizer(threadTotal, load_first, alloc_size));

  void (*test_func)(void *, Checkpoint*) = _test5_count_up_f;

  test5Dat test_dat;
  void* test_mem = checkOrg->check_alloc(each_alloc);
  ASSERT_FALSE(test_mem == 0);

  test_dat.iterations = each_alloc;
  test_dat.step = 90000;
  test_dat.mem = (char*)test_mem;
  
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 1;
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 2;
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 3;
  checkOrg->startThread(test_func, &test_dat);
  
  checkOrg->joinAll();
  
  delete checkOrg;

  CheckpointOrganizer* checkOrg2 = new CheckpointOrganizer(LABEL1, LABEL2);
  checkOrgCleanupRef = checkOrg2;
  ASSERT_FALSE(checkOrg->initCheckpointOrganizer(threadTotal, load_first, alloc_size));

  test_func = _test5_count_down_f;

  test_mem = checkOrg2->check_alloc(each_alloc);
  ASSERT_FALSE(test_mem == 0);

  test_dat.iterations = each_alloc;
  test_dat.mem = (char*)test_mem;
  
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 1;
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 2;
  checkOrg->startThread(test_func, &test_dat);
  test_dat.iterations = each_alloc - 3;
  checkOrg->startThread(test_func, &test_dat);
  
  checkOrg->joinAll();
   
}
 

#endif //CHECKPOINTTESTS_HPP
