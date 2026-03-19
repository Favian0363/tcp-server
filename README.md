### Simple TCP Servers

How to use (terminal):
- "ss -ltn" -> verify socket creation
- "nc localhost 9090" -> connect to socket 

#### Blocking, Single Client
- only handling one client's file descriptor

#### Non-Blocking, Multi Client
- using std::thread for independent client handling

