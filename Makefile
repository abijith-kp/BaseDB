OPTIONS=`pkg-config --cflags --libs glib-2.0` -g
OBJS=createtable.o main.o fes.o test_main.o
EXECS=basedb basedb_test
TEST_DB=students

all: basedb

run: basedb
	./basedb

run_test: test
	rm -rf ${TEST_DB}
	./basedb_test

basedb: createtable fes main
	gcc ${OPTIONS} createtable.o main.c fes.o -o basedb

test: createtable test_main
	gcc ${OPTIONS} createtable.o test_main.c -o basedb_test

createtable:
	gcc ${OPTIONS} -c createtable.c -o createtable.o

fes:
	gcc ${OPTIONS} -c fes.c -o fes.o

main:
	gcc ${OPTIONS} -c main.c -o main.o

test_main:
	gcc ${OPTIONS} -c test_main.c -o test_main.o

clean:
	rm -rf ${EXECS} ${OBJS} ${TEST_DB}
