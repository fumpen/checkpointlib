CPP = g++
MAIN = main.cpp
TARGET = test

CFLAGS = -Wall  -W -Wpedantic -std=c++11
LINKING = -lpthread

default: clean 
	$(CPP) $(CFLAGS) main.cpp -o $(TARGET) $(LINKING)

clean:
ifneq ("$(wildcard *~)","")
	rm *~
endif
ifneq ("$(wildcard *.tmp)","")
	rm *.tmp
endif
ifneq ("$(wildcard $(TARGET))","")
	rm $(TARGET) 
endif
