#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main() {
    int client_socket = 0;
    char buffer[1024];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
        perror("Endereço inválido/não suportado");
        exit(EXIT_FAILURE);
    }

    if(connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Falha na conexão");
        exit(EXIT_FAILURE);
    }

    while(1){
        std::cout << ">";
        std::string msg;

        getline(std::cin, msg);
        memset(&buffer, 0, sizeof(buffer));
        strcpy(buffer, msg.c_str());

        if(msg == "exit"){
            send(client_socket, (char*)&buffer, strlen(buffer), 0);
            break;
        }

        send(client_socket, (char*)&buffer, strlen(buffer), 0);
        memset(&buffer, 0, sizeof(buffer));
        recv(client_socket, (char*)&buffer, sizeof(buffer), 0);

        if(!strcmp(buffer, "exit")){
            std::cout << "Desligando..." << std::endl;
            break;
        }

        std::cout << "Server: " << buffer << std::endl;
    }

    close(client_socket);

    return 0;
}