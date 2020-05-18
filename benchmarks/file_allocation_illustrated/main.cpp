#include <stdlib.h>
#include <time.h>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long double avg_time(std::size_t* measurements, std::size_t list_length){
  std::size_t summ = 0;
  for(std::size_t i=0; i<list_length; i++){
    summ += measurements[i];
  }
  return ((long double)summ) / ((long double)list_length);
	
}


long double file_test_stream(std::string fname, std::size_t f_size, std::size_t repititions){
  
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  std::ofstream outfile;
  char placeholder = ' ';

  for(std::size_t i=0; i<repititions; i++){
    remove(fname.c_str());
    t1 = std::chrono::high_resolution_clock::now();
    outfile.open(fname.c_str(), std::ios::out | std::ios::trunc | std::ios::binary );
    outfile.seekp(f_size -1, std::ios_base::beg);
    outfile.write(&placeholder, 1);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
    outfile.close();
  }
  
  return avg_time(tmp_results, repititions);
}

long double file_test_FILE(std::string fname, std::size_t f_size, std::size_t repititions){
  
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  FILE *fp;
  char placeholder = ' ';
  

  for(std::size_t i=0; i<repititions; i++){
    remove(fname.c_str());
    t1 = std::chrono::high_resolution_clock::now();
    fp = fopen(fname.c_str(), "w+b");
    fseek(fp, f_size -1, SEEK_SET);
    fwrite(&placeholder, 1, 1, fp);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
    fclose(fp);
  }
  
  return avg_time(tmp_results, repititions);
}

long double file_test_syscalls(std::string fname, std::size_t buff_size, std::size_t repititions){
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  int fd;

  for(std::size_t i=0; i<repititions; i++){
    remove(fname.c_str());
    t1 = std::chrono::high_resolution_clock::now();
    fd = open(fname.c_str(), O_RDWR  | O_CREAT, S_IRWXU);
    ftruncate(fd, buff_size);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
    close(fd);
  }

  return avg_time(tmp_results, repititions);
}


int main (){
  std::size_t num_test_iterations = 100000;
  std::size_t number_tests_to_run = 5;
  std::size_t test_size;
  
  std::string test_filename = "testfile.test";

  long double test_time_stream[number_tests_to_run];
  long double test_time_FILE[number_tests_to_run];
  long double test_time_syscalls[number_tests_to_run];
  
  std::size_t all_test_size[number_tests_to_run];
  all_test_size[0] = 100000;
  all_test_size[1] = 1000000;
  all_test_size[2] = 10000000;
  all_test_size[3] = 100000000;
  all_test_size[4] = 1000000000;
  
  for(unsigned int q=0; q<5; q++){
    test_size = all_test_size[q];
    
    test_time_stream[q] = file_test_stream(test_filename, test_size, num_test_iterations);
    
    test_time_FILE[q] = file_test_FILE(test_filename, test_size, num_test_iterations);
      
    test_time_syscalls[q] = file_test_syscalls(test_filename, test_size, num_test_iterations);

    
    printf("Avg time in microseconds for writing buffer of size (in bytes): %lu \n stream avg time: %lf \n FILE* avg time: %lf \n syscall avg time: %lf \n\n",
	   test_size,
	   (double)test_time_stream[q],
	   (double)test_time_FILE[q],
	   (double)test_time_syscalls[q]);

  }
  
  return 0;
}
