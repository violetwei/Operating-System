Most of your programs rely on dynamic memory allocation to implement complex data structures.
By default, in Unix/Linux you use the malloc library for dynamic memory allocation. You could
install other libraries instead of malloc for dynamic memory allocation. In this assignment, you
will develop a library called Simple Memory Allocator (SMA) for dynamic memory management.
Because your library is working at the user-level, your library would grab memory in bulk from
the kernel and then distribute it to the application requests as they come in. The application
program is going to use the API provided by your Simple Memory Allocator to manage dynamic
memory.
