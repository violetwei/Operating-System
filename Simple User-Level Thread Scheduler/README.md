## How to compile 
open a command prompt / terminal
```
make sut
```

## How to run the sut.c program with the 5 tests

### Run test1.c
```
gcc test1.c sut.c -o test1 -lpthread && ./test1
```

### Run test2.c
```
gcc test2.c sut.c -o test2 -lpthread && ./test2
```

### Run test3.c
```
gcc test3.c sut.c -o test3 -lpthread && ./test3
```

### Run test4.c 
In order to test sut_write(), open a new terminal to open a backend and enter ```nc -l -p 3001```, where 3001 can replaced with any port number

Next, in another terminal window, run
```
gcc test4.c sut.c -o test4 -lpthread && ./test4
```

### Run test5.c
In order to test sut_read(), open a terminal windown and run ```make backend`` 

Then, enter ```./backend 0.0.0.0 3001``` to bring up a backend which keeps sending messages

Next, in nother terminal window, run
```
gcc test5.c sut.c -o test5 -lpthread && ./test5
```