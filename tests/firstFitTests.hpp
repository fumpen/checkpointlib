#ifndef FIRSTFITTESTS_HPP
#define FIRSTFITTESTS_HPP

#include "../lib/first_fit.hpp"
#include "testHelp.hpp"
#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <vector>


std::string test0_create_allocation_obj(){
  std::size_t alloc_size = 500;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit imTheTest = FirstFit(b, alloc_size);
  void* t1 = imTheTest.ff_allocate(10);

  if(t1 == 0){
    return cleanup_test("t1 not null");
  }

  void* t2 = imTheTest.ff_allocate(10);
  if(t2 == 0) return cleanup_test("t2 == 0");

  if(t2 == t1)return cleanup_test("t1 == t2");
  
  return cleanup_test(RET_SUCESS);
}


std::string test1_allocation_entire_buffer(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 300;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  for(unsigned int i=0; i<number_of_allocations; i++){
    if(ff.ff_allocate(individual_allocations) == 0)
      return cleanup_test("ff.ff_allocate(individual_allocations) == 0");
  }

  if(ff.ff_allocate(individual_allocations) != 0)
    return cleanup_test("ff.ff_allocate(individual_allocations) != 0");

  return cleanup_test(RET_SUCESS);
}

std::string test2_free_single(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 300;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);

  FirstFit ff = FirstFit(b, alloc_size);
  void* test_free;
  for(unsigned int i=0; i<number_of_allocations; i++){
    test_free = ff.ff_allocate(individual_allocations);
    if(test_free == 0) return cleanup_test("test_free == 0"); 
  }
  
  if(ff.ff_free(test_free)) return cleanup_test("ff.ff_free(test_free)");
  
  if(ff.ff_allocate(individual_allocations) == 0)
    return cleanup_test("ff.ff_allocate(individual_allocations) == 0");

  return cleanup_test(RET_SUCESS);
}


std::string test3_free_all(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 200;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size);
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  void* test_free;
  for(unsigned int i=0; i<number_of_allocations; i++){
    test_free = ff.ff_allocate(individual_allocations);
    if(!test_free) return cleanup_test("!test_free");
    
    if(ff.ff_free(test_free)) return cleanup_test("ff.ff_free(test_free)");
  }

  return cleanup_test(RET_SUCESS);
}

std::string test4_fill_fragmented_mem(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 5;
  std::vector<void*> saved_allocations;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  void* test_free;
  for(unsigned int i=0; i<number_of_allocations; i++){
    saved_allocations.push_back(ff.ff_allocate(individual_allocations));
    if(saved_allocations[i] == 0) return cleanup_test("saved_allocations[i] == 0");
  }
  
  if(ff.ff_allocate(individual_allocations) != 0)
    return cleanup_test("ff.ff_allocate(individual_allocations) != 0");
  
  if(ff.ff_free(saved_allocations[2])) return cleanup_test("ff.ff_free(saved_allocations[2])");
  
  test_free = ff.ff_allocate(individual_allocations);
  if(test_free == 0) return cleanup_test("test_free == 0");
  
  if(test_free != saved_allocations[2]) return cleanup_test("test_free != saved_allocations[2]");
  
  return cleanup_test(RET_SUCESS);
}



std::string test5_fragmented_mem_not_created_for_blocks_below_min_size(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 5;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  std::vector<void*> saved_allocations;
  void* test_free;
  for(unsigned int i=0; i<number_of_allocations; i++){
    saved_allocations.push_back(ff.ff_allocate(individual_allocations));
    if(saved_allocations[i] == 0) return cleanup_test("saved_allocations[i] == 0");
       
  }
  
  ff.ff_free(saved_allocations[2]);

  test_free = ff.ff_allocate(individual_allocations - ff_alloc_size);
  if(test_free == 0) return cleanup_test("test_free == 0");
  
  if(test_free != saved_allocations[2]) return cleanup_test("test_free != saved_allocations[2]");

  if(ff.ff_allocate(ff_alloc_size))
    return cleanup_test("ff.ff_allocate(ff_alloc_size)");
  
  return cleanup_test(RET_SUCESS);
}


std::string test6_adjust_size_bigger(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 5;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  std::vector<void*> saved_allocations;
  for(unsigned int i=0; i<(number_of_allocations -1); i++){
    saved_allocations.push_back(ff.ff_allocate(individual_allocations));
    if(saved_allocations[i] == 0) return cleanup_test("saved_allocations[i] == 0");
    
  }
  
  saved_allocations.push_back(ff.ff_allocate(individual_allocations + 10));

  if(saved_allocations[number_of_allocations - 1] != 0)
    return cleanup_test("saved_allocations[number_of_allocations - 1] != 0");
  
  ff.ff_adjust_allocation_size(alloc_size + 10);
  saved_allocations[number_of_allocations - 1] = ff.ff_allocate(individual_allocations + 10);
  if(saved_allocations[number_of_allocations - 1] == 0)
    return cleanup_test("saved_allocations[number_of_allocations - 1] == 0");

  return cleanup_test(RET_SUCESS);
}


std::string test7_adjust_size_smaller(){
  unsigned int individual_allocations = 30;
  unsigned int number_of_allocations = 5;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b = malloc(alloc_size);
  vec_malloc.push_back(b);
  FirstFit ff = FirstFit(b, alloc_size);

  std::vector<void*> saved_allocations;
  for(unsigned int i=0; i<(number_of_allocations -1); i++){
    saved_allocations.push_back(ff.ff_allocate(individual_allocations));
    if(saved_allocations[i] == 0) return cleanup_test("saved_allocations[i] == 0");
    
  }

  ff.ff_adjust_allocation_size(alloc_size - 10);
  saved_allocations.push_back(ff.ff_allocate(individual_allocations));

  if(saved_allocations[number_of_allocations - 1] != 0)
    return cleanup_test("saved_allocations[number_of_allocations - 1] != 0");
  
  saved_allocations[number_of_allocations - 1] = ff.ff_allocate(individual_allocations - 10);
  if(saved_allocations[number_of_allocations - 1] == 0)
    return cleanup_test("saved_allocations[number_of_allocations - 1] == 0");

  return cleanup_test(RET_SUCESS);
}

std::string test8_is_mapping_identical(){
  unsigned int individual_allocations = sizeof(int);
  unsigned int number_of_allocations = 500;
  std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
  void* b1 = malloc(alloc_size);
  void* b2 = malloc(alloc_size);
  vec_malloc.push_back(b1);
  vec_malloc.push_back(b2);
  FirstFit ff1 = FirstFit(b1, alloc_size);
  FirstFit ff2 = FirstFit(b2, alloc_size);

  std::vector<void*> saved_allocations1;
  std::vector<void*> saved_allocations2;
  for(unsigned int i=0; i<(number_of_allocations -1); i++){
    saved_allocations1.push_back(ff1.ff_allocate(individual_allocations));
    if(saved_allocations1[i] == 0) return cleanup_test("saved_allocations1[i] == 0");
    *((int*)saved_allocations1[i]) = (int)i;
  }
  for(unsigned int i=0; i<(number_of_allocations -1); i++){
    saved_allocations2.push_back(ff2.ff_allocate(individual_allocations));
    if(saved_allocations2[i] == 0) return cleanup_test("saved_allocations2[i] == 0");
    *((int*)saved_allocations2[i]) = (int)i;
  }

  if(memcmp(b1, b2, alloc_size)) return cleanup_test("memcmp(b1, b2, alloc_size)");

  return cleanup_test(RET_SUCESS);
}

void runAllFirstFitTests(){
  std::vector<std::string> test_results;
  std::cout << "\n--- Initiating  FirstFit tests: --- \n";

  test_results.push_back(test0_create_allocation_obj());

  test_results.push_back(test1_allocation_entire_buffer());
  
  test_results.push_back(test2_free_single());
  
  test_results.push_back(test3_free_all());
  
  test_results.push_back(test4_fill_fragmented_mem());
  
  test_results.push_back(test5_fragmented_mem_not_created_for_blocks_below_min_size());

  test_results.push_back(test6_adjust_size_bigger());
  
  test_results.push_back(test7_adjust_size_smaller());
  
  test_results.push_back(test8_is_mapping_identical());

  std::cout << "\n--- FirstFit test results: --- \n";
  for(std::size_t i=0; i<test_results.size(); i++){
    if(RET_SUCESS.compare(test_results[i])){
      std::cout << "test " << i << " finished with: '" << "ERROR -> " + test_results[i] << "' \n";
    }else{
      std::cout << "test " << i << " finished with: '" << test_results[i] << "' \n";
    }
  }
  std::cout << "\n--- finished with FirstFit tests --- \n";
  
}

#endif //FIRSTFITTESTS_HPP

