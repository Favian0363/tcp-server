#include <iostream> 
#include <sys/socket.h>
#include <netinet/in.h> // converting port numbers
#include <unistd.h> // close()
#include <arpa/inet.h> // converting binary ip to string
#include <thread>
#include <vector> 
#include <algorithm>
#include <condition_variable>
#include <queue>
#include <mutex>

std::condition_variable cv;
std::mutex mtx;

struct Client{
    int client_fd;
    sockaddr_in client_addr;
};

static std::queue<Client> work_queue;

void handle_client(int client_fd, sockaddr_in client_addr){
    if (client_fd < 0){
        perror("Accept Failed"); // print os error
        return;
    }
    std::cout << "Connection from: " << inet_ntoa(client_addr.sin_addr) << ':' << ntohs(client_addr.sin_port) << " | File descriptor: " << client_fd << '\n';
    send(client_fd, "Connected\n", sizeof("Connected\n"), 0);
    char buffer[1024];
    ssize_t read{recv(client_fd, buffer, sizeof(buffer), 0)}; // read input (blocking)
    std::cout << "Bytes read: " << read << '\n';
    if (read <= 0){
        close(client_fd);
        return;
    }
    std::cout.write(buffer, read); // prints exact number of bytes to avoid out of bounds from recv() not auto appending a null terminator
    send(client_fd, "Received\n", sizeof("Received\n"), 0);
    close(client_fd);
}

void worker_loop(){
    while (true){
        std::unique_lock<std::mutex> lock(mtx);
        while (work_queue.empty()){
            cv.wait(lock); // release lock and sleep
        }
        Client cl {work_queue.front()};
        work_queue.pop();
        lock.unlock();
        handle_client(cl.client_fd, cl.client_addr);
    }
}

int main(){
    int socket_fd{socket(AF_INET, SOCK_STREAM, 0)}; // create socket
    sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET; // Ipv4
    socket_addr.sin_port = htons(9090); // port
    socket_addr.sin_addr.s_addr = INADDR_ANY; // listen for everything

    // bind socket to port and ip
    if (bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) < 0){
        perror("bind");
        close(socket_fd);
        return 1;
    }; 
    // listen for client (max queue of 64)
    if (listen(socket_fd, 64) < 0){
        perror("listen");
        close(socket_fd);
        return 1;
    }; 

    std::vector<std::thread> worker_vec;

    worker_vec.reserve(8);
    for (int i{0}; i<8; ++i){
        worker_vec.emplace_back(worker_loop);
    }
    
    while (true){
        sockaddr_in client_addr;
        socklen_t client_addr_len {sizeof(client_addr)};
        int client_fd {accept(socket_fd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_len)}; // accept client (blocking)
        std::unique_lock<std::mutex> lock(mtx);
        work_queue.push(Client{client_fd, client_addr});
        lock.unlock();
        cv.notify_one();
    }
}

// thread pool
// have a vector of threads 
// when a client appears, assign a thread from the vector to handle that client 
