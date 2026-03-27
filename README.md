### Simple TCP Servers

How to use (terminal):
- "ss -ltn" -> verify socket creation
- "nc localhost 9090" -> connect to socket 

#### Blocking, Single Client
- create socket -> bind socket
- set socket to listen 
- accept connection -> handle client (confirm connection and receive bytes)

#### Non-Blocking, Multi Client
- thread pool (8) for independent client handling
    - using mutex, unique locks, condition variables 

