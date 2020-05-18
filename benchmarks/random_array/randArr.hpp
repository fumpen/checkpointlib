#ifndef RANDARR_HPP
#define RANDARR_HPP


#include <stdlib.h>
#include <time.h>

struct arrTestDat{
  unsigned long int segment;
  unsigned long int num_segments;
  int* prime;
  int** mem;
};
  

int random_range(int lower, int upper){
  return (rand() % (upper - lower + 1)) + lower;
}

void init_arr(int* a, unsigned long int seg){
  int tmp;
  for(unsigned long int y=0; y<seg; y++){
    tmp = random_range(-10000, 10000);
    a[y] = tmp;
  }
}

void add_in_place(int* a, int* b, unsigned long int seg){
  for(unsigned long int y=0; y<seg; y++){
    a[y] = a[y] + b[y]; 
  }  
}

void init_all(int** buff, unsigned long int seg, unsigned long int num){
  int tmp;
  for(unsigned long int i = 0; i<num; i++){
    tmp = rand();
    for(unsigned long int y=0; y<seg; y++){
      buff[i][y] = tmp;
    }
  }
}

void add_one_to_all(int* addMe, int** buff, unsigned long int seg, unsigned long int num){
  for(unsigned long int i=0; i<num; i++){
    add_in_place(buff[i], addMe, seg);
    std::cout << "i: " << i << "\n";
  }
}

void add_multiples_to_all(int* addMe, int** buff, unsigned long int seg, unsigned long int num){
  for(unsigned long int i=0; i<num; i++){
    for(unsigned long int y=0; y<i; y++){
      add_in_place(buff[i], addMe, seg);
    }
  }
}

void refresh_prime(int* prime, int** buff, unsigned long int seg, unsigned long int num){
  int a;
  int b;
  for(unsigned long int i=0; i<seg; i++){
    a = random_range(0, num - 1);
    b = random_range(0, seg - 1);
    prime[i] = buff[a][b];
  }
}

void printArr(int** a, unsigned long int seg, unsigned long int num){
  std::cout << " --- \n";
  for(unsigned long int x=0; x<num; x++){
    for(unsigned long int y=0; y<seg; y++){
      std::cout << a[x][y] << ", ";
      if(!(y % 10)) std::cout << "\n";
    }
    std::cout << "-\n- ";
  }
  std::cout << "\n *** \n";
}


#endif //RANDARR_HPP
