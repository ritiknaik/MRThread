default : run

build : test_spin test_search test_mutest test

run : build

test : mrthread.o utils.o test.o lock.o
			cc -o test mrthread.o utils.o test.o lock.o

test_spin : mrthread.o utils.o spintest.o lock.o
			cc -o test_spin mrthread.o utils.o lock.o spintest.o

test_search : mrthread.o utils.o search.o lock.o
			cc -o test_search mrthread.o utils.o search.o lock.o

test_mutest : mrthread.o utils.o mutest.o lock.o
			cc -o test_mutest mrthread.o utils.o mutest.o lock.o

mrthread.o : src/mrthread.c headers/mrthread.h
			cc -c src/mrthread.c

utils.o : src/utils.c headers/mrthread.h
			cc -c src/utils.c

lock.o : src/lock.c headers/lock.h 
			cc -c src/lock.c

test.o : tests/test.c headers/mrthread.h
			cc -c tests/test.c

spintest.o : tests/spintest.c headers/mrthread.h
			cc -c tests/spintest.c

mutest.o : tests/mutest.c headers/mrthread.h
			cc -c tests/mutest.c

search.o : tests/search.c headers/mrthread.h
			cc -c tests/search.c

clean :
		rm search.o mutest.o spintest.o test.o lock.o utils.o mrthread.o test_mutest test_search test_spin test