#ifndef FIRSTFITTESTS_HPP
#define FIRSTFITTESTS_HPP

#include "gtest/gtest.h"

#include "../lib/first_fit.hpp"
#include "testHelp.hpp"
#include <stddef.h>
#include <stdint.h>
#include <iostream>
#include <vector>


TEST(firstFit_tests, create_alloc_object) {
	std::size_t alloc_size = 500;
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);
	FirstFit imTheTest = FirstFit(b, alloc_size);
	void* t1 = imTheTest.ff_allocate(10);

	ASSERT_FALSE(t1 == 0);

    void* t2 = imTheTest.ff_allocate(10);
	ASSERT_FALSE(t2 == 0);
	ASSERT_FALSE(t2 == t1);

}


TEST(firstFit_tests, allocate_the_entire_buffer){
	unsigned int individual_allocations = 30;
	unsigned int number_of_allocations = 300;
	std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);
	FirstFit ff = FirstFit(b, alloc_size);

	for(unsigned int i=0; i<number_of_allocations; i++)
		ASSERT_FALSE(ff.ff_allocate(individual_allocations) == 0);

	ASSERT_FALSE(ff.ff_allocate(individual_allocations) != 0);
}


TEST(firstFit_tests, free_single_allocation){
	unsigned int individual_allocations = 30;
	unsigned int number_of_allocations = 300;
	std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);

	FirstFit ff = FirstFit(b, alloc_size);
	void* test_free;
	for(unsigned int i=0; i<number_of_allocations; i++){
		test_free = ff.ff_allocate(individual_allocations);
		ASSERT_FALSE(test_free == 0);
	}

	ASSERT_FALSE(ff.ff_free(test_free));

	ASSERT_FALSE(ff.ff_allocate(individual_allocations) == 0);
}

TEST(firstFit_tests, free_all_allocations){
	unsigned int individual_allocations = 30;
	unsigned int number_of_allocations = 200;
	std::size_t alloc_size = (individual_allocations + ff_alloc_size);
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);
	FirstFit ff = FirstFit(b, alloc_size);

	void* test_free;
	for(unsigned int i=0; i<number_of_allocations; i++){
		test_free = ff.ff_allocate(individual_allocations);
		ASSERT_FALSE(!test_free);

		ASSERT_FALSE(ff.ff_free(test_free));
	}
}

TEST(firstFit_tests, fill_fragmented_mem){
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
		ASSERT_FALSE(saved_allocations[i] == 0);
	}

	ASSERT_FALSE(ff.ff_allocate(individual_allocations) != 0);

	ASSERT_FALSE(ff.ff_free(saved_allocations[2]));

	test_free = ff.ff_allocate(individual_allocations);
	ASSERT_FALSE(test_free == 0);

	ASSERT_FALSE(test_free != saved_allocations[2]);
}

TEST(firstFit_tests, fragmented_mem_not_created_for_blocks_below_min_size){
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
		ASSERT_FALSE(saved_allocations[i] == 0);
	}

	ff.ff_free(saved_allocations[2]);

	test_free = ff.ff_allocate(individual_allocations - ff_alloc_size);
	ASSERT_FALSE(test_free == 0);

	ASSERT_FALSE(test_free != saved_allocations[2]);

	ASSERT_FALSE(ff.ff_allocate(ff_alloc_size));
}

TEST(firstFit_tests, adjust_size_bigger){
	unsigned int individual_allocations = 30;
	unsigned int number_of_allocations = 5;
	std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);
	FirstFit ff = FirstFit(b, alloc_size);

	std::vector<void*> saved_allocations;
	for(unsigned int i=0; i<(number_of_allocations -1); i++){
		saved_allocations.push_back(ff.ff_allocate(individual_allocations));
		ASSERT_FALSE(saved_allocations[i] == 0);

	}

	saved_allocations.push_back(ff.ff_allocate(individual_allocations + 10));

	ASSERT_FALSE(saved_allocations[number_of_allocations - 1] != 0);

	ff.ff_adjust_allocation_size(alloc_size + 10);
	saved_allocations[number_of_allocations - 1] = ff.ff_allocate(individual_allocations + 10);
	ASSERT_FALSE(saved_allocations[number_of_allocations - 1] == 0);
}


TEST(firstFit_tests, adjust_size_smaller){
	unsigned int individual_allocations = 30;
	unsigned int number_of_allocations = 5;
	std::size_t alloc_size = (individual_allocations + ff_alloc_size) * number_of_allocations;
	void* b = malloc(alloc_size);
	vec_malloc.push_back(b);
	FirstFit ff = FirstFit(b, alloc_size);

	std::vector<void*> saved_allocations;
	for(unsigned int i=0; i<(number_of_allocations -1); i++){
		saved_allocations.push_back(ff.ff_allocate(individual_allocations));
		ASSERT_FALSE(saved_allocations[i] == 0);
	}

	ff.ff_adjust_allocation_size(alloc_size - 10);
	saved_allocations.push_back(ff.ff_allocate(individual_allocations));

	ASSERT_FALSE(saved_allocations[number_of_allocations - 1] != 0);

	saved_allocations[number_of_allocations - 1] = ff.ff_allocate(individual_allocations - 10);
	ASSERT_FALSE(saved_allocations[number_of_allocations - 1] == 0);
}


TEST(firstFit_tests, is_mapping_identical){
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
		ASSERT_FALSE(saved_allocations1[i] == 0);
		*((int*)saved_allocations1[i]) = (int)i;
	}
	for(unsigned int i=0; i<(number_of_allocations -1); i++){
		saved_allocations2.push_back(ff2.ff_allocate(individual_allocations));
		ASSERT_FALSE(saved_allocations2[i] == 0);
		*((int*)saved_allocations2[i]) = (int)i;
	}

	ASSERT_FALSE(memcmp(b1, b2, alloc_size));
}

#endif //FIRSTFITTESTS_HPP

