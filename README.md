# checkpointlib

c++ library for checkpointing multiple threads in single address space.
requireing: std=c++11 & linux operating system.
header only

The library itself is in the directory lib/ (library documentation also in that folder)

Unit tests for each component of the library is in tests/ (note here that a lot of prints will happen if all tests are enabled in main.cpp, comment some of them out is advisable on run)

The  benchmarks reprecented in the thesis is in benchmarks/

Examples of using checkpointlib is found in examples/

The master thesis report submitted with the library is found in main.pdf