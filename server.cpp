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
#define BUFFER_SIZE 1024

std::vector<int> clients;

// hash (key: client_name, value: client_socket)
std::unordered_map<std::string, int> clients_info;

// hash (key: client_socket, value: client_name) 
std::unordered_map<int, std::string> clients_networking; 

std::mutex mutex_clt;
std::string header = R"(
-----------------------------------------------------------------------
 ██████╗██╗  ██╗ █████╗ ████████╗               █████╗ ██████╗ ██████╗ 
██╔════╝██║  ██║██╔══██╗╚══██╔══╝              ██╔══██╗██╔══██╗██╔══██╗
██║     ███████║███████║   ██║       █████╗    ███████║██████╔╝██████╔╝
██║     ██╔══██║██╔══██║   ██║       ╚════╝    ██╔══██║██╔═══╝ ██╔═══╝ 
╚██████╗██║  ██║██║  ██║   ██║                 ██║  ██║██║     ██║     
 ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝                 ╚═╝  ╚═╝╚═╝     ╚═╝     
-----------------------------------------------------------------------)";

void rotine_client(int client_socket);
std::string random_color();
void send_whisper(std::string buffer, int client_socket);
void send_anonymous(std::string buffer, int client_socket);
void private_chat();
bool is_command(std::string buffer, int client_socket);

int main(){
    std::system("clear");
    std::cout << header << std::endl;
    std::srand(std::time(nullptr));

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

void send_whisper(std::string buffer, int client_socket){
    int name_end = buffer.find(' ', 3);
    std::string name_receptor = buffer.substr(3, name_end - 3);
    std::string message = "\033[38;5;170m[" + clients_networking[client_socket] + "] ~ Sussuro \033[0m:"+ buffer.substr(name_end + 1);

    int client_receptor = clients_info[name_receptor];

    if(client_receptor > 0){
        mutex_clt.lock();
        send(client_receptor, message.c_str(), message.size(), 0);
        mutex_clt.unlock();
    }
}

void private_chat(){

}


void send_anonymous(std::string buffer, int client_socket){
    std::string message = "\033[38;5;248m[Anonymous]: " + buffer.substr(3) + "\033[0m";

    mutex_clt.lock();
    for(auto client : clients){
        if(client != client_socket)
            send(client, message.c_str(), message.size(), 0);
    }
    mutex_clt.unlock();
}

bool is_command(std::string buffer, int client_socket){

    if(buffer.substr(0, 2) == "\\w"){
        send_whisper(buffer, client_socket);
        return true;
    }

    if(buffer.substr(0, 2) == "\\p"){
        private_chat();
        return true;
    }

    if(buffer.substr(0, 2) == "\\a"){
        send_anonymous(buffer, client_socket);
        return true;
    }

    return false;
}

void rotine_client(int client_socket){
    int is_private = 0;
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    std::string name_client(buffer, bytes_read);
    std::string color = random_color();

    mutex_clt.lock();
    clients_info[name_client] = client_socket;
    clients_networking[client_socket] = name_client;
    mutex_clt.unlock();

    std::string color_name = color + name_client + "]\033[0m";
    std::cout << "-> Cliente " << color_name << " conectado." << std::endl;

    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

        if(bytes_read > 0){
            std::string buffer_str(buffer, bytes_read);
            if(!is_command(buffer, client_socket)){ // send global message
                std::string output_client = color_name + ": " + buffer;
                std::cout << output_client << std::endl;

                mutex_clt.lock();
                for(auto client : clients){
                    if(client != client_socket)
                        send(client, output_client.c_str(), output_client.size(), 0);
                }
                mutex_clt.unlock();
            }
        }else if(bytes_read == 0){
            std::cout << "-> Cliente " << color_name << " desconectado." << std::endl;
            close(client_socket);

            mutex_clt.lock();
            for(size_t i = 0; i < clients.size(); ++i){
                if(clients[i] == client_socket){
                    clients[i] = -1;  
                    break;
                }
            }

            clients_info.erase(name_client);
            clients_networking.erase(client_socket);
            mutex_clt.lock();

            break;
        }
    }
}

std::string random_color() {
    return "\033[38;5;" + std::to_string(16 + rand() % 220) + "m["; // ANSI Code
}


