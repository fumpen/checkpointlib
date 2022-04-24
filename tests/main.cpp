#include "gtest/gtest.h"

#include "fileManagerTests.hpp"
#include "firstFitTests.hpp"
#include "memManagerTests.hpp"
#include "checkpointTests.hpp"

int main(int argc, char **argv){
  
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
