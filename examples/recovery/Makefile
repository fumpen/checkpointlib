

default: test-application
	g++ main.cpp


test-application: clean
	g++ application.cpp -lpthread -o testApp

clean:
ifneq ("$(wildcard *~)","")
	rm *~
endif
ifneq ("$(wildcard tmp*)","")
	rm tmp*
endif
ifneq ("$(wildcard testApp)","")
	rm testApp
endif
ifneq ("$(wildcard a.out)","")
	rm a.out
endif
