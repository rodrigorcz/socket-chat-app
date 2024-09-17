#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime> 
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

std::vector<int> clients;
std::mutex mutex_clt;

void rotine_client(int client_socket);

std::string random_color() {
    return "\033[38;5;" + std::to_string(16 + rand() % 220) + "m["; // ANSI Code
}

int main(){
    std::srand(std::time(nullptr));

    char buffer[1024];
    int server_socket;
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP IP
    if(server_socket == 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    // Configurar SO_RCVBUF pode ser util
    int opt = 1;
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("Falha ao configurar o socket (setsockopt)");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Falha no bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server_socket, 6) < 0) {
        perror("Falha no listen");
        exit(EXIT_FAILURE);
    }
    int ret = std::system("clear");

    int addrlen = sizeof(server_address);
    std::cout << "--------------------------------------------------" << std::endl;
    while(true){
        int client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen);
        if(client_socket < 0){
            perror("Falha no accept");
            continue;
        }

        mutex_clt.lock();
        clients.push_back(client_socket);
        mutex_clt.unlock();

        std::thread client_thread(rotine_client, client_socket);
        client_thread.detach();  
    }

    close(server_socket);

    return 0;
}


void rotine_client(int client_socket){
    char buffer[1024];
    recv(client_socket, buffer, sizeof(buffer), 0);

    char name_client[1024];
    strcpy(name_client, buffer);

    std::string color = random_color();
    std::cout << ">----- Cliente " << color << name_client << "]\033[0m conectado ----< " << std::endl;
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

        if(bytes_read > 0){
            std::cout << color << name_client << "]\033[0m: " << buffer << std::endl;

            mutex_clt.lock();
            for (auto client : clients) {
                if (client != client_socket) {
                    send(client, buffer, bytes_read, 0);
                }
            }
            mutex_clt.unlock();

        }else if(bytes_read == 0){
            std::cout << "Cliente desconectado." << std::endl;
            close(client_socket);

            mutex_clt.lock();
            for(size_t i = 0; i < clients.size(); ++i){
                if(clients[i] == client_socket){
                    clients[i] = -1;  
                    break;
                }
            }
            mutex_clt.lock();

            break;
        }
    }
}



