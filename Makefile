OPTIONS=`pkg-config --cflags --libs glib-2.0` -g -DINDEXED
OBJS=linked_list_new.o createtable.o main.o fes.o test_main.o utils.o
EXECS=basedb basedb_test
TEST_DB=students employee

all: basedb

run: basedb
	./basedb

run_test: test
	rm -rf ${TEST_DB}
	./basedb_test

basedb: createtable fes main linked_list_new utils
	gcc ${OPTIONS} linked_list_new.o createtable.o utils.o main.c fes.o -o basedb

test: createtable test_main linked_list_new utils
	gcc ${OPTIONS} linked_list_new.o createtable.o utils.o test_main.c -o basedb_test

createtable:
	gcc ${OPTIONS} -c createtable.c -o createtable.o

fes:
	gcc ${OPTIONS} -c fes.c -o fes.o

main:
	gcc ${OPTIONS} -c main.c -o main.o

test_main:
	gcc ${OPTIONS} -c test_main.c -o test_main.o

linked_list_new:
	gcc ${OPTIONS} -c linked_list_new.c -o linked_list_new.o

utils:
	gcc ${OPTIONS} -c utils.c -o utils.o

clean:
	rm -rf ${EXECS} ${OBJS} ${TEST_DB}
