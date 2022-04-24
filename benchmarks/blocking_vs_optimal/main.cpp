#include "speed_test.hpp"
#include<iostream>
#include <fstream>
#include <string>

std::string results_fname = "speedtest_results.tmp";

void run_test(std::size_t test_repetitions, std::size_t allocation_size,
	      unsigned int threadTotal, unsigned long int iterations = 5){
  std::ofstream myfile;
  myfile.open (results_fname.c_str(), std::ios::app);
  myfile << "\n------------------------------------------------\n"
	 << "Testing with number of test repetitions: " << test_repetitions
	 << ", memory allocation size: " << allocation_size
	 << ", number of threads: " << threadTotal
	 << ", number of iterations in each test: " << iterations << "\n";
  myfile.close();

  std::size_t summ_normal_beginning = 0;
  std::size_t summ_normal_end = 0;
  std::size_t summ_blocking = 0;
  std::size_t summ_control = 0;
  for(std::size_t i=0; i<test_repetitions; i++){
    summ_normal_beginning += speed_write_beginning_checkpoint_normal(allocation_size, threadTotal, iterations);
    summ_normal_end += speed_write_end_checkpoint_normal(allocation_size, threadTotal, iterations);
    summ_blocking += speed_write_checkpoint_blocking(allocation_size, threadTotal, iterations);
    summ_control += speed_control(allocation_size, threadTotal, iterations);
  }
  summ_normal_beginning = summ_normal_beginning / test_repetitions;
  summ_normal_end = summ_normal_end / test_repetitions;
  summ_blocking = summ_blocking / test_repetitions;
  summ_control = summ_control / test_repetitions;

  myfile.open (results_fname.c_str(), std::ios::app);
  myfile << "normal write-beginning avg: " << summ_normal_beginning << "\n"
	 << "normal write-end avg:       " << summ_normal_end << "\n"
	 << "blocking avg:               " << summ_blocking << "\n"
	 << "control avg:                " << summ_control << "\n"
	 << "\n";
  myfile.close();
}

int main(){
  
  std::ofstream myfile;
  
  myfile.open (results_fname.c_str(), std::ios::trunc);
  myfile << "Stating new speedtest benchmarks.\n";
  myfile.close();

  std::size_t test_repetitions = 1000;
  std::size_t allocation_size = 100000;
  unsigned int threadTotal = 4;
  unsigned long int iterations = 1000000;

  myfile.open (results_fname.c_str(), std::ios::app);
  myfile << "\n\n**Now testing increasing allocation size**\n\n";
  myfile.close();
  
  while(allocation_size < 1000001){
    run_test(test_repetitions, allocation_size, threadTotal, iterations);
    allocation_size += 100000;
  }
  


  return 0;
}
