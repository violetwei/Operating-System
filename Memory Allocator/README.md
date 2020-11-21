Most of your programs rely on dynamic memory allocation to implement complex data structures.
By default, in Unix/Linux you use the malloc library for dynamic memory allocation. You could
install other libraries instead of malloc for dynamic memory allocation. In this assignment, you
will develop a library called Simple Memory Allocator (SMA) for dynamic memory management.
Because your library is working at the user-level, your library would grab memory in bulk from
the kernel and then distribute it to the application requests as they come in. The application
program is going to use the API provided by your Simple Memory Allocator to manage dynamic
memory.

### Testing & How to Run

In the a3_test1.c file, it contains "Test 1: Excess Memory Allocation" and "Test 6: Print SMA Statistics".
By running ```gcc a3_test1.c sma.c && ./a.out``` on the command line, you can see Test1 is PASSED and the SMA stats for Test6 are printed.

In the a3_test2.c file, it contains "Test 2: Program break expansion test" and "Test 6: Print SMA Statistics".
By running ```gcc a3_test2.c sma.c && ./a.out``` on the command line, you can see Test2 is PASSED and the SMA stats for Test6 are printed.

In the a3_test3.c file, it contains "Test 3: Check for Worst Fit algorithm".
By running ```gcc a3_test3.c sma.c && ./a.out``` on the command line, you can see Test3 is PASSED.

In the a3_test4.c file, it contains "Test 4: Check for Next Fit algorithm".
By running ```gcc a3_test4.c sma.c && ./a.out``` on the command line, you can see Test4 is PASSED.

In the a3_test5.c file, it contains "Test 5: Check for Reallocation with Next Fit".
By running ```gcc a3_test5.c sma.c && ./a.out``` on the command line, you can see Test5 is PASSED.

### Other running test option
```make test1 && ./test1```
```make test2 && ./test2```
```make test3 && ./test3```
```make test4 && ./test4```
```make test5 && ./test5```
