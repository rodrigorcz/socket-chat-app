#include "server.hpp"

int main(){
    // exibe o cabecalho inicial limpando a tela
    std::system("clear");
    std::cout << header << std::endl;
    std::srand(std::time(nullptr));

    // criacao do socket
    int server_socket;    
    server_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if(server_socket == 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    // configuracoes do socket para permitir reuso de endereco e porta
    int opt = 1;
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Falha ao configurar o socket (SO_REUSEADDR)");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("Falha ao configurar o socket (SO_REUSEPORT)");
        exit(EXIT_FAILURE);
    }

    // estrutura para armazenar informacoes do endereco do servidor
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_port = htons(PORT); // porta
    server_address.sin_addr.s_addr = INADDR_ANY;
    int addrlen = sizeof(server_address);

    // associa o socket ao endereco e porta
    if(bind(server_socket, (struct sockaddr *)&server_address, addrlen) < 0){
        perror("Falha no bind");
        exit(EXIT_FAILURE);
    }
    
    // escuta por conexoes de clientes
    if(listen(server_socket, MAX_CLIENTS) < 0){
        perror("Falha no listen");
        exit(EXIT_FAILURE);
    }

    // loop principal para aceitar conexoes de clientes
    while(true){
        // aceita uma nova conexao
        int client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen);
        if(client_socket < 0){
            perror("Falha no accept");
            continue;
        }

        // adiciona o socket à lista de clientes com uso de mutex
        mutex_clt.lock();
        clients_list.push_back(client_socket);
        current_clients++;
        mutex_clt.unlock();

        // cria uma nova thread para gerenciar a comunicacao com o cliente
        std::thread client_thread(rotine_client, client_socket);
        client_thread.detach();  // desanexa a thread para permitir execucao independente
    }

    // fecha o socket do servidor
    close(server_socket);

    return 0;
}

// funcao para gerenciar a rotina de cada cliente
void rotine_client(int client_socket){
    std::string color;
    if(style) color = colors[current_clients-1]; // define uma cor para o cliente
    
    // recebe o nome do cliente
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string name_client(buffer, bytes_read);

    // mapeia as informações referentes cliente (nome e socket)
    mutex_clt.lock();
    clients_info[name_client] = client_socket;
    clients_networking[client_socket] = name_client;
    privates_chats[client_socket] = false;
    mutex_clt.unlock();

    // formata o nome do cliente para exibicao
    std::string name_client_out;
    (!style)? name_client_out = "[" + name_client + "]" : 
              name_client_out = color + name_client + "]\033[0m";
   
    std::cout << "-> Cliente " << name_client_out << " conectado." << std::endl;

    // loop para receber mensagens
    while(running){
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0); // recebe a mensagem

        if(bytes_read > 0){ // verifica se a mensagem foi recebida
            std::string buffer_str(buffer, bytes_read);
            if(!process_command(buffer_str, client_socket)){  // verifica se a mensagem contem algum comando

                if(privates_chats[client_socket]) continue;
                
                // formatacao da mensagem que sera enviada
                std::string output_client = name_client_out + ": " + buffer_str;
                std::cout << output_client << std::endl;

                // envia a mensagem ao restante dos clientes conectados
                mutex_clt.lock();
                for(auto client : clients_list){
                    if(client != client_socket && !privates_chats[client])
                        send(client, output_client.c_str(), output_client.size(), 0);
                    else if (style && client == client_socket){
                        buffer_str = "\033[A\33[2K" + name_client_out + " (YOU): " + buffer_str;;
                        send(client, buffer_str.c_str(), buffer_str.size(), 0);
                    }
                }
                mutex_clt.unlock();
            }
        }else if(bytes_read == 0){ // verifica se o cliente desconectou
            std::cout << "-> Cliente " << name_client_out << " desconectado." << std::endl;
            remove_client(client_socket, name_client); 
            break;
        }
    }
}

// funcao que verifica se houve comando na mensagem enviada e ja executa o comando se houver
bool process_command(std::string buffer, int client_socket){

    // envia sussuro
    if(buffer.substr(0, 2) == "\\w"){
        send_whisper(buffer, client_socket);
        return true;
    }

    // entra em chat privado
    if(buffer.substr(0, 2) == "\\p"){
        return private_chat(buffer, client_socket);
    }

    // envia mensagem anonima
    if(buffer.substr(0, 2) == "\\a"){
        send_anonymous(buffer, client_socket);
        return true;
    }

    return false;
}

// funcao para o envio de sussuro
void send_whisper(std::string buffer, int client_socket){

    // processa a mensagem enviada
    int name_end = buffer.find(' ', 3);
    std::string name_receptor = buffer.substr(3, name_end - 3);
    std::string message;

    // formata a mensagem caso style esteja ativo
    if(!style) 
        message = "[" + clients_networking[client_socket] + "] ~ Sussuro: "+ buffer.substr(name_end + 1);
    else 
        message = "\033[38;5;170m[" + clients_networking[client_socket] + "] ~ Sussuro: \033[0m"+ buffer.substr(name_end + 1);

    mutex_clt.lock(); // bloqueia o mutex para acesso seguro aos dados compartilhados
    // verifica se o receptor existe
    if (clients_info.find(name_receptor) == clients_info.end()) {
        std::string error_msg = "Usuário " + name_receptor + " não encontrado.\n"; // mensagem de erro
        send(client_socket, error_msg.c_str(), error_msg.size(), 0); // envia mensagem de erro ao cliente
        mutex_clt.unlock(); 
        return; // retorna indicando falha
    }
    int client_receptor = clients_info[name_receptor]; // encontra o socket do cliente com base no nome indicado
    mutex_clt.unlock(); 

    // envia o sussuro para o cliente caso encontre-o
    send(client_receptor, message.c_str(), message.size(), 0);
}

// funcao para o envio de mensagem anonima
void send_anonymous(std::string buffer, int client_socket){
    // processa a mensagem e a formata
    std::string message;

    if(style)
        message = "\033[38;5;248m[Anonymous]: " + buffer.substr(3) + "\033[0m";
    else
        message = "[Anonymous]: " + buffer.substr(3);

    std::cout << message << std::endl;

    // envia a mensagem aos clientes
    for(auto client : clients_list){
        if (style && client == client_socket)
            message = "\033[A\33[2K" + message;
            
        send(client, message.c_str(), message.size(), 0);
    }
}

// funcao para iniciar um chat privado entre dois clientes
bool private_chat(std::string buffer, int client_socket) {
    int name_end = buffer.find(' ', 3);
    std::string name_receptor = buffer.substr(3, name_end - 3);

    mutex_clt.lock(); // bloqueia o mutex para acesso seguro aos dados compartilhados
    // verifica se o receptor existe
    if (clients_info.find(name_receptor) == clients_info.end()) {
        std::string error_msg = "Usuário " + name_receptor + " não encontrado.\n"; // mensagem de erro
        send(client_socket, error_msg.c_str(), error_msg.size(), 0); // envia mensagem de erro ao cliente
        mutex_clt.unlock(); 
        return true; // retorna true indicando encerramento da funcao
    }

    int receptor_socket = clients_info[name_receptor]; // encontra o socket do cliente com base no nome indicado
    privates_chats[client_socket] = true;
    privates_chats[receptor_socket] = true;
    mutex_clt.unlock();

    // encontra-se os nomes dos clientes envolvidos com base nos sockets
    std::string name_client = clients_networking[client_socket];
    std::string name_receptor_client = clients_networking[receptor_socket];
    
    // envia uma mensagem de entrada no chat privado aos envolvidos
    std::string li = "-----------------------------------------------------------------------\n";
    std::string welcome_msg = li + "          Iniciando chat privado com " + name_receptor + "!!\n" + li;
    
    send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0);
    send(receptor_socket, welcome_msg.c_str(), welcome_msg.size(), 0);

    // inicia duas threads para gerenciar a conversa privada
    std::thread private_thread(rotine_private_chat, client_socket, receptor_socket, name_client, name_receptor_client);
    std::thread receptor_thread(rotine_private_chat, receptor_socket, client_socket, name_receptor_client, name_client);

    // aguarda execucao das threads
    private_thread.join();
    receptor_thread.join();

    return true; 
}

// funcao para gerenciar a rotina do chat privado entre dois clientes
void rotine_private_chat(int client_socket, int receptor_socket, std::string name_client, std::string name_receptor_client) {
    char buffer_private[BUFFER_SIZE];
    bool active = true;

    // loop de troca de mensagens
    while(active){
        memset(buffer_private, 0, sizeof(buffer_private));
        int bytes_read = recv(client_socket, buffer_private, sizeof(buffer_private), 0);

        if(bytes_read <= 0){ // se nao houver bytes lidos encerra o chat
            active = false;
            break;
        }

        // formata a mensagem de saida e envia ao receptor
        std::string buffer_str(buffer_private, bytes_read);
        std::string output_msg;

        if(style){
            output_msg = "\033[38;5;051m[Privado - " + name_client + "]:\033[0m " + buffer_str;
            send(receptor_socket, output_msg.c_str(), output_msg.size(), 0);
            output_msg = "\033[A\33[2K" + output_msg;
            send(client_socket, output_msg.c_str(), output_msg.size(), 0);
        }else{
            output_msg = "[Privado - " + name_client + "]: " + buffer_str;
            send(receptor_socket, output_msg.c_str(), output_msg.size(), 0);
        }

    }

    // ao sair do chat privado atualiza as informações
    privates_chats[client_socket] = false;
    privates_chats[receptor_socket] = false;
}

// funcao para remover um cliente desconectado
void remove_client(int client_socket, std::string name_client) {

    // sinaliza a thread da rotina do cliente no servidor para parar
    running = false;

    // apaga as informacoes em todas as estruturas que armazenavam suas informacoes
    close(client_socket);

    clients_info.erase(name_client);
    clients_networking.erase(client_socket);
    current_clients--;
}
