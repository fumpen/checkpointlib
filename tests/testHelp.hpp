#ifndef TESTHELP_HPP
#define TESTHELP_HPP

#include <stdio.h> //remove()
#include <vector>
#include <unistd.h>


static std::vector<void*> vec_malloc;

static std::string RET_SUCESS = "success";

static std::uint8_t ff_alloc_size = sizeof(std::size_t) + 1;
static std::size_t _page_size = sysconf(_SC_PAGESIZE);

static std::string LABEL1 = "test_checkpoint_active";
static std::string LABEL2 = "test_checkpoint_backup";

void _delete_test_files(std::string fName1, std::string fName2){
  if(remove(fName1.c_str())){
    printf("err while deleting file\n");
  }
  if(remove(fName2.c_str())){
    printf("err while deleting file\n");
  }
}

std::string cleanup_test(std::string ret_msg){
  for(std::size_t i=0; i<vec_malloc.size(); i++){
    if(vec_malloc[i] != 0){
      free(vec_malloc[i]);
    }
  }
  vec_malloc.clear();
  return ret_msg;
}

std::size_t round_up_page(std::size_t size_req){
  std::size_t multiple = ((std::size_t)
			  (size_req /
			   _page_size));
  if(size_req % _page_size) multiple += 1;
  
  return multiple * _page_size;
}

#endif //TESTHELP_HPP
