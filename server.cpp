#include "server.hpp"

int main(){
    std::system("clear");
    std::cout << header << std::endl;
    std::srand(std::time(nullptr));

    int server_socket;    
    server_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if(server_socket == 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Falha ao configurar o socket (SO_REUSEADDR)");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("Falha ao configurar o socket (SO_REUSEPORT)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
        perror("Falha no bind");
        exit(EXIT_FAILURE);
    }
    
    if(listen(server_socket, MAX_CLIENTS) < 0){
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
        clients_list.push_back(client_socket);
        current_clients++;
        mutex_clt.unlock();

        std::thread client_thread(rotine_client, client_socket);
        client_thread.detach();  
    }

    close(server_socket);

    return 0;
}

std::string assign_color(int client_id) {
    if (!style) {
        return ""; 
    }
    return colors[client_id]; 
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

bool private_chat(std::string buffer, int client_socket) {
    int name_end = buffer.find(' ', 3);
    std::string name_receptor = buffer.substr(3, name_end - 3);

    mutex_clt.lock();
    if(clients_info.find(name_receptor) == clients_info.end()){
        std::string error_msg = "Usuário " + name_receptor + " não encontrado.\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        mutex_clt.unlock();
        return false;
    }
    int receptor_socket = clients_info[name_receptor];
    privates_chats[client_socket] = true;
    privates_chats[receptor_socket] = true;
    mutex_clt.unlock();

    std::string name_client = clients_networking[client_socket];
    std::string name_receptor_client = clients_networking[receptor_socket];
    
    std::string li = "-----------------------------------------------------------------------\n";
    std::string welcome_msg = li + "Iniciando chat privado com " + name_receptor + ". Digite \\exit para sair.\n" + li;
    
    send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);
    send(receptor_socket, welcome_msg.c_str(), welcome_msg.size(), 0);

    char buffer_private[BUFFER_SIZE];
    std::thread private_thread([client_socket, receptor_socket, name_client, name_receptor_client] {
        char buffer_private[BUFFER_SIZE];
        while (true) {
            memset(buffer_private, 0, sizeof(buffer_private));
            int bytes_read = recv(client_socket, buffer_private, sizeof(buffer_private), 0);

            if (bytes_read <= 0) {
                send(client_socket, "Erro ao ler a mensagem. Desconectando...\n", 40, 0);
                break;
            }

            std::string buffer_str(buffer_private, bytes_read);

            if (buffer_str == "\\exit\n") {
                std::string exit_msg = name_client + " saiu do chat privado.\n";
                send(client_socket, "Você saiu do chat privado.\n", 29, 0);
                send(receptor_socket, exit_msg.c_str(), exit_msg.size(), 0);
                break;
            }

            std::string output_msg = "[Privado - " + name_client + "]: " + buffer_str;
            send(receptor_socket, output_msg.c_str(), output_msg.size(), 0);
        }
        
        privates_chats[client_socket] = false;
        privates_chats[receptor_socket] = false;
    });

    std::thread receptor_thread([client_socket, receptor_socket, name_client, name_receptor_client] {
        char buffer_private[BUFFER_SIZE];
        while (true) {
            memset(buffer_private, 0, sizeof(buffer_private));
            int bytes_read = recv(receptor_socket, buffer_private, sizeof(buffer_private), 0);

            if (bytes_read <= 0) {
                send(receptor_socket, "Erro ao ler a mensagem. Desconectando...\n", 40, 0);
                break;
            }

            std::string buffer_str(buffer_private, bytes_read);

            if (buffer_str == "\\exit\n") {
                std::string exit_msg = name_receptor_client + " saiu do chat privado.\n";
                send(receptor_socket, "Você saiu do chat privado.\n", 29, 0);
                send(client_socket,  exit_msg.c_str(), exit_msg.size(), 0);
                break;
            }

            std::string output_msg = "[Privado - " + name_receptor_client + "]: " + buffer_str;
            send(client_socket, output_msg.c_str(), output_msg.size(), 0);
        }
        
        privates_chats[client_socket] = false;
        privates_chats[receptor_socket] = false;
    });

    private_thread.detach();
    receptor_thread.detach();

    return true; 
}


void send_anonymous(std::string buffer, int client_socket){
    std::string message;

    if(style)
        message = "\033[38;5;248m[Anonymous]: " + buffer.substr(3) + "\033[0m";
    else
        message = "[Anonymous]: " + buffer.substr(3);

    std::cout << message << std::endl;

    mutex_clt.lock();
    for(auto client : clients_list){
        if (style && client == client_socket)
            message = "\033[A\33[2K" + message;
            
        send(client, message.c_str(), message.size(), 0);
    }
    mutex_clt.unlock();
}

bool process_command(std::string buffer, int client_socket){

    if(buffer.substr(0, 2) == "\\w"){
        send_whisper(buffer, client_socket);
        return true;
    }

    if(buffer.substr(0, 2) == "\\p"){
        return private_chat(buffer, client_socket);
    }

    if(buffer.substr(0, 2) == "\\a"){
        send_anonymous(buffer, client_socket);
        return true;
    }

    return false;
}

void rotine_client(int client_socket){
    std::string color = assign_color(current_clients-1);
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string name_client(buffer, bytes_read);

    mutex_clt.lock();
    clients_info[name_client] = client_socket;
    clients_networking[client_socket] = name_client;
    privates_chats[client_socket] = false;
    mutex_clt.unlock();

    if(style)
        name_client = color + name_client + "]\033[0m";
    else
        name_client = "[" + name_client + "]";

    std::cout << "-> Cliente " << name_client << " conectado." << std::endl;

    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

        if(bytes_read > 0){
            std::string buffer_str(buffer, bytes_read);
            if(!process_command(buffer_str, client_socket)){ 
                std::string output_client = name_client + ": " + buffer_str;
                std::cout << output_client << std::endl;

                mutex_clt.lock();
                for(auto client : clients_list){
                    if(client != client_socket && !privates_chats[client])
                        send(client, output_client.c_str(), output_client.size(), 0);
                    else if (style && client == client_socket){
                        buffer_str = "\033[A\33[2K" + name_client + " (YOU): " + buffer_str;;
                        send(client, buffer_str.c_str(), buffer_str.size(), 0);
                    }
                }
                mutex_clt.unlock();
            }
        }else if(bytes_read == 0){
            std::cout << "-> Cliente " << name_client << " desconectado." << std::endl;
            remove_client(client_socket, name_client);
            break;
        }
    }
}

void remove_client(int client_socket, std::string name_client) {
    close(client_socket);

    mutex_clt.lock();
    for(size_t i = 0; i < clients_list.size(); ++i){
        if(clients_list[i] == client_socket){
            clients_list[i] = -1;  
            break;
        }
    }

    clients_info.erase(name_client);
    clients_networking.erase(client_socket);
    current_clients--;
    mutex_clt.lock();
}

