We have provided some wrapper functions so that it's easier for you to create RPC clients and servers. You can find the function declarations in a1_lib.h. To summarize, here is how you can use them:

- To create a server, you need the functions create_server() and accept_connection()
- For a client program to connect to a server, you can use connect_to_server()
- To send a message from a client to a server (or vice versa), use the function send_message()
- To receive a message, use the function recv_message().

For a concrete implementation of a simple client/server, you can refer to the example_*.c files. 

To run the Simple RPC Service, do the following:
    - In a terminal, run 'make rpc' to compile the program, it will generate the frontend and backend executable
    - Start the backend by running './backend 0.0.0.0 10000' or with the format "./backend <host_ip> <host_port>"
    - You will then see the message "Server listening on <host_ip>:<host_port>" 
    - Start the frontend in another terminal by running './frontend 0.0.0.0 10000' or with the format "./frontend <host_ip> <host_port>
    - Feel free to run multiple frotends by opening new terminal windows and enter the same command.
    - You will be prompted to type the command after ">>" in the frontend terminal and hit enter. 
    - The result/output will be shown quickly in the same frontend windows
    - Supported commands including:
    - "add 10 2"
    - "multiply 4 5"
    - "divide 99 10"
    - "factorial 6"  -> returns the factorial of 6
    - "sleep 5" -> sleeps for 5 seconds
    - "exit" -> terminates the frontend only
    - "quit" -> terminates the frontend and send a shutdown signal to the backend
    - Run 'make clean' to remove the executables.
