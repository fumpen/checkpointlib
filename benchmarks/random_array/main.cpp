#include "speed_test.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <chrono>

int main(){

  unsigned int rep_per_round = 100;
  unsigned int rounds = 1;
  
  std::size_t res_norm = 0;
  std::size_t res_cont = 0;
  std::size_t res_barr = 0;
  std::size_t res_thread = 0;
  for(std::size_t i=0; i<rounds; i++){
    res_barr += test_array_scramble_barrier(rep_per_round);
    res_thread += test_array_scramble_thread(rep_per_round);
    res_norm += test_array_scramble_normal(rep_per_round);
    res_cont += test_array_scramble_control(rep_per_round);
    
  }
  
  auto time_stamp = std::chrono::system_clock::now();
  char tmp_filename[200];
  snprintf(tmp_filename, 200, "/home/frederik/tmp_results/randome_array_benchmark_%lu.txt",
	   std::chrono::duration_cast<std::chrono::seconds>(time_stamp.time_since_epoch()).count());
  std::ofstream myfile;
  myfile.open(tmp_filename);
  myfile << "***Benchmark results***\nWith " << rounds << " rounds and "
	 << rep_per_round << " iterations per round\n"
	 << "res time with chkpt:  " << res_norm << "\n"
	 << "res without control:  " << res_cont << "\n"
	 << "control with barrier: " << res_barr << "\n"
	 << "control with thread:  " << res_thread << "\n";
  myfile.close();

  return 0;
}
