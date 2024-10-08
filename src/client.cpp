#include "client.hpp"

int main(){

    // criacao de um socket do tipo TCP
    int client_socket = 0;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){
        perror("Falha ao criar o socket");
        exit(EXIT_FAILURE);
    }

    // estrutura para armazenar informacoes do endereco do servidor
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;   // IPv4
    serv_addr.sin_port = htons(PORT); // porta
    
    // converte o endereço IP do servidor
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0){
        perror("Endereço inválido/não suportado");
        exit(EXIT_FAILURE);
    }

    // tenta estabelecer a conexão com o servidor
    if(connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Falha na conexão com servidor");
        exit(EXIT_FAILURE);
    }
    
    // limpa a tela do terminal e solicita o envio de um nome
    g = std::system("clear");
    std::cout << "Insira seu nome: " << std::endl;
    std::cin >> name_client;
    g = std::system("clear");
    
    // envia o nome do cliente para o servidor
    send(client_socket, name_client.c_str(), name_client.size(), 0);

    std::cout << header << std::endl; // imprime op header

    // cria uma thread para enviar mensagens e outra para receber, respectivamente
    std::thread send_thread(send_messages, client_socket);
    std::thread receive_thread(receive_messages, client_socket);

    // aguarda a finalização das threads
    send_thread.join();
    receive_thread.join();

    close(client_socket);

    return 0;
}

// funcao para o envio de mensagem entre cliente -> servidor
void send_messages(int client_socket){
    char buffer[BUFFER_SIZE];

    // loop para envio de mensagem
    while(true){
        std::cin.getline(buffer, BUFFER_SIZE);
        if(!strcmp(buffer, "\\exit")){ // fecha o chat caso o cliente digite \exit
            std::cout << "Fechando o Chat...." << std::endl;  
            close(client_socket);
            exit(EXIT_SUCCESS);
        };

        send(client_socket, buffer, strlen(buffer), 0); // envia a mensagem ao servidor
    }
}

// funcao para o recebimento de mensagens entre servidor -> cliente
void receive_messages(int client_socket){
    char buffer[BUFFER_SIZE];

    // loop para recebimento de mensagens
    while(true){
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); // recebe a mensagem do servidor
        if(bytes_received > 0){
            std::cout << buffer << std::endl;
        }else if(bytes_received == 0){ // verifica se o servidor foi desconectado
            std::cout << "Servidor desconectado." << std::endl;
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
    }
}
