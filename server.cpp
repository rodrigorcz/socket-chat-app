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
#include <unordered_map>

#define PORT 8080

std::vector<int> clients;
std::unordered_map<int, std::pair<std::string,std::string>> clients_info;
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
    std::system("clear");

    int addrlen = sizeof(server_address);
    std::cout << "--------------------------------------------------";
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
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    std::string name_client(buffer, bytes_read);
    std::string color = random_color();

    mutex_clt.lock();
    clients_info[client_socket] = {name_client, color};
    mutex_clt.unlock();

    std::string color_name = color + name_client + "]\033[0m";
    std::cout << "------- Cliente " << color_name << " conectado ------- " << std::endl;

    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

        if(bytes_read > 0){
            std::string output_client = color_name + ": " + buffer;
            std::cout << output_client << std::endl;

            mutex_clt.lock();
            for (auto client : clients) {
                if (client != client_socket) {
                    send(client, output_client.c_str(), output_client.size(), 0);
                }
            }
            mutex_clt.unlock();

        }else if(bytes_read == 0){
            std::cout << "Cliente " << color_name << " desconectado." << std::endl;
            close(client_socket);

            mutex_clt.lock();
            for(size_t i = 0; i < clients.size(); ++i){
                if(clients[i] == client_socket){
                    clients[i] = -1;  
                    break;
                }
            }
            
            clients_info.erase(client_socket);
            mutex_clt.lock();

            break;
        }
    }
}



