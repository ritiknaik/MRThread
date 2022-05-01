default : run

build : test_search test

run : build

test : mrthread.o queue.o test.o mangle.o
			cc -o test mrthread.o queue.o test.o mangle.o

test_search : mrthread.o queue.o search.o mangle.o
			cc -o test_search mrthread.o queue.o search.o mangle.o

mrthread.o : src/mrthread.c headers/mrthread.h
			cc -c src/mrthread.c

queue.o : src/queue.c headers/mrthread.h
			cc -c src/queue.c

test.o : tests/test.c headers/mrthread.h
			cc -c tests/test.c

search.o : tests/search.c headers/mrthread.h
			cc -c tests/search.c

mangle.o : src/mangle.c
			cc -c src/mangle.c

clean :
		rm search.o test.o queue.o mrthread.o mangle.o test_search test