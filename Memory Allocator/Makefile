# .DEFAULT_GOAL=all
CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -lm

TEST1=test1
TEST2=test2
TEST3=test3
TEST4=test4
TEST5=test5   

test1: a3_test1.c sma.c
	$(CC) -o $(TEST1) $(CFLAGS) sma.c a3_test1.c

test2: a3_test2.c sma.c
	$(CC) -o $(TEST2) $(CFLAGS) sma.c a3_test2.c

test3: a3_test3.c sma.c
	$(CC) -o $(TEST3) $(CFLAGS) sma.c a3_test3.c

test4: a3_test4.c sma.c
	$(CC) -o $(TEST4) $(CFLAGS) sma.c a3_test4.c

test5: a3_test5.c sma.c
	$(CC) -o $(TEST5) $(CFLAGS) sma.c a3_test5.c

	
clean:
	rm -rf  $(TEST1) $(TEST2) $(TEST3) $(TEST4) $(TEST5)