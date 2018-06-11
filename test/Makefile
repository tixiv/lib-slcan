
INCLUDES= -I../

TARGET=test

all: clean run_tests

clean:
	rm -f test.exe

run_tests: test
	./test
	

test: test_main.cpp ../slcan.c
	g++ -Wall -Wextra $(INCLUDES) -o test $^ -lUnitTest++
