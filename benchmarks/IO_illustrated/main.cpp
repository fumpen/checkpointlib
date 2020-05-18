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

int fill_buffer(std::size_t number_of_ints, void* buff){
  srand (time(NULL));
  int* buffer = (int*) buff;
  for(std::size_t i=0; i< number_of_ints; i++){
    buffer[i] = rand();
  }
  return 0;
}

long double write_test_stream(std::string fname, std::size_t buff_size,
			      void* buff, std::size_t repititions){
  remove(fname.c_str());
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  std::ofstream outfile;
  outfile.open(fname.c_str(), std::ios::out | std::ios::trunc | std::ios::binary );
  outfile.seekp(0, std::ios_base::beg);
  outfile.write((char *)buff, buff_size);

  for(std::size_t i=0; i<repititions; i++){
    outfile.seekp(0, std::ios_base::beg);
    t1 = std::chrono::high_resolution_clock::now();
    outfile.write((char *)buff, buff_size);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
  }
  
  
  outfile.close();
  return avg_time(tmp_results, repititions);
}

long double write_test_FILE(std::string fname, std::size_t buff_size,
			    void* buff, std::size_t repititions){
  remove(fname.c_str());
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  FILE *fp;
  fp = fopen(fname.c_str(), "w+b");
  fseek(fp, 0, SEEK_SET);
  fwrite(buff, buff_size, 1, fp);

  for(std::size_t i=0; i<repititions; i++){
    fseek(fp, 0, SEEK_SET);
    t1 = std::chrono::high_resolution_clock::now();
    fwrite(buff, buff_size, 1, fp);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
  }
  fclose(fp);
  return avg_time(tmp_results, repititions);
}

long double write_test_syscalls(std::string fname, std::size_t buff_size,
				void* buff, std::size_t repititions){
  remove(fname.c_str());
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  int fd = open(fname.c_str(), O_RDWR  | O_CREAT, S_IRWXU);
  ftruncate(fd, buff_size);
  lseek(fd, 0, SEEK_SET);
  write(fd, buff, buff_size);

  for(std::size_t i=0; i<repititions; i++){
    lseek(fd, 0, SEEK_SET);
    t1 = std::chrono::high_resolution_clock::now();
    write(fd, buff, buff_size);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
  }

  close(fd);
  return avg_time(tmp_results, repititions);
}

long double write_test_malloc(std::size_t buff_size, void* buff,
			      std::size_t repititions){
  std::size_t tmp_results[repititions];
  auto t1 = std::chrono::high_resolution_clock::now();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::size_t duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

  void* new_buff = malloc(buff_size);

  for(std::size_t i=0; i<repititions; i++){
    t1 = std::chrono::high_resolution_clock::now();
    memcpy(new_buff, buff, buff_size);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    tmp_results[i] = duration;
  }
  
  free(new_buff);
  return avg_time(tmp_results, repititions);
}

int main (){
  std::size_t num_test_iterations = 1000;
  std::size_t number_tests_to_run = 5;
  std::size_t number_of_test_integers;
  std::size_t test_size;
  void* buff;

  std::string test_filename = "testfile.test";

  long double test_time_stream[number_tests_to_run];
  long double test_time_FILE[number_tests_to_run];
  long double test_time_syscalls[number_tests_to_run];
  long double test_time_malloc[number_tests_to_run];

  std::size_t all_test_integers[number_tests_to_run];
  all_test_integers[0] = 10000;
  all_test_integers[1] = 100000;
  all_test_integers[2] = 1000000;
  all_test_integers[3] = 10000000;
  all_test_integers[4] = 100000000;
  
  for(unsigned int q=0; q<5; q++){
    number_of_test_integers = all_test_integers[q];
    test_size = number_of_test_integers * sizeof(int);
    buff = malloc(test_size);
    fill_buffer(number_of_test_integers, buff);

    test_time_stream[q] = write_test_stream(test_filename, test_size, buff, num_test_iterations);
    
    test_time_FILE[q] = write_test_FILE(test_filename, test_size, buff, num_test_iterations);
      
    test_time_syscalls[q] = write_test_syscalls(test_filename, test_size, buff, num_test_iterations);

    test_time_malloc[q] = write_test_malloc(test_size, buff, num_test_iterations);

    
    printf("Avg time in microseconds for writing buffer of size (in bytes): %lu \n stream avg time: %lf \n FILE* avg time: %lf \n syscall avg time: %lf \n malloc avg time: %lf\n\n",
	   test_size,
	   (double)test_time_stream[q],
	   (double)test_time_FILE[q],
	   (double)test_time_syscalls[q],
	   (double)test_time_malloc[q]);

    free(buff);
  }
  
  return 0;
}
