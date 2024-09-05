#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

// server = socket + bind + listen + accept

int main(){
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

    int addrlen = sizeof(server_address);
    int new_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen);
    if(new_socket < 0) {
        perror("Falha no accept");
        exit(EXIT_FAILURE);
    }

    int bytes_read = 0;

    while(1){
        memset(&buffer, 0, sizeof(buffer));

        recv(new_socket, (char*)&buffer, sizeof(buffer), 0);

        if(!strcmp(buffer, "exit")){
            std::cout << "Fim da sessão..." << std::endl;
            break;
        }
        
        std::cout << "Cliente: " << buffer << std::endl;
        std::cout << ">";

        std::string msg;
        getline(std::cin, msg);

        memset(&buffer, 0, sizeof(buffer));

        strcpy(buffer, msg.c_str());
        if(msg == "exit"){
            send(new_socket, (char*)&buffer, strlen(buffer), 0);
            break;
        }

        send(new_socket, (char*)&buffer, strlen(buffer), 0);
    }

    close(new_socket);
    close(server_socket);

    return 0;
}



