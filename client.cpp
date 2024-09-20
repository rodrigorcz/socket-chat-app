#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP "127.0.0.1"

std::string name_client;
std::string header = R"(
-----------------------------------------------------------------------
 ██████╗██╗  ██╗ █████╗ ████████╗               █████╗ ██████╗ ██████╗ 
██╔════╝██║  ██║██╔══██╗╚══██╔══╝              ██╔══██╗██╔══██╗██╔══██╗
██║     ███████║███████║   ██║       █████╗    ███████║██████╔╝██████╔╝
██║     ██╔══██║██╔══██║   ██║       ╚════╝    ██╔══██║██╔═══╝ ██╔═══╝ 
╚██████╗██║  ██║██║  ██║   ██║                 ██║  ██║██║     ██║     
 ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝                 ╚═╝  ╚═╝╚═╝     ╚═╝     
-----------------------------------------------------------------------)";

void send_messages(int client_socket);
void receive_messages(int client_socket);

int main(){
    int client_socket = 0;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0){
        perror("Endereço inválido/não suportado");
        exit(EXIT_FAILURE);
    }

    if(connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Falha na conexão");
        exit(EXIT_FAILURE);
    }
    
    std::system("clear");
    std::cout << "Insira seu nome: " << std::endl;
    std::getline(std::cin, name_client);
    std::system("clear");
    
    send(client_socket, name_client.c_str(), name_client.size(), 0);

    std::cout << header << std::endl;

    std::thread send_thread(send_messages, client_socket);
    std::thread receive_thread(receive_messages, client_socket);

    send_thread.join();
    receive_thread.join();

    close(client_socket);

    return 0;
}

void send_messages(int client_socket){
    char buffer[BUFFER_SIZE];
    while(true){
        std::cin.getline(buffer, BUFFER_SIZE);
        if(!strcmp(buffer, "exit")){            
            close(client_socket);
            exit(0);
        };

        std::string msg =  "\033[A\33[2K" + name_client;
        send(client_socket, buffer, strlen(buffer), 0);
    }
}

void receive_messages(int client_socket){
    char buffer[BUFFER_SIZE];
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if(bytes_received > 0){
            std::cout << buffer << std::endl;
        }else if(bytes_received == 0){
            std::cout << "Cliente desconectado." << std::endl;
            close(client_socket);
            exit(0);
        }
    }
}