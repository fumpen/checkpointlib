#include "fileManagerTests.hpp"
#include "firstFitTests.hpp"
#include "memManagerTests.hpp"
#include "checkpointTests.hpp"

int main(){
  runAllFileManagerTests();
  runAllFirstFitTests();
  
  run_all_memManagerTests();

  runAllCheckpointManagerTests();
  return 0;
}
