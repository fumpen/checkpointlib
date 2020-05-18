

default: clean
	g++ gen_primes.cpp -lpthread -Werror 

clean:
ifneq ("$(wildcard *~)","")
	rm *~
endif
ifneq ("$(wildcard tmp*)","")
	rm tmp*
endif
ifneq ("$(wildcard a.out)","")
	rm a.out
endif
