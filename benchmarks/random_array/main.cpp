#include "speed_test.hpp"
#include <iostream>
#include <fstream>
#include <string>

int main(){

  unsigned int rep = 1000; // 9 hours, 20 min ~approx
  
  //std::size_t res_norm = test_array_scramble_normal(rep);
  //std::size_t res_cont = test_array_scramble_control(rep);
  std::size_t res_barr = test_array_scramble_barrier(rep);
  std::size_t res_thread = test_array_scramble_thread(rep);

  //std::cout << "res-chkpt:   " << res_norm << "\n";
  //std::cout << "res-control: " << res_cont << "\n";
  std::cout << "res-barrier: " << res_barr << "\n";
  std::cout << "res-thread: " << res_thread << "\n";
  return 0;
}
