#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// definicao de constantes
#define PORT 8080         // porta em que o servidor escutara
#define BUFFER_SIZE 1024  // tamanho do buffer para comunicacao
#define IP "127.0.0.1"    // endereço IP do servidor

std::string name_client;
std::string header = R"(
-----------------------------------------------------------------------
 ██████╗██╗  ██╗ █████╗ ████████╗               █████╗ ██████╗ ██████╗ 
██╔════╝██║  ██║██╔══██╗╚══██╔══╝              ██╔══██╗██╔══██╗██╔══██╗
██║     ███████║███████║   ██║       █████╗    ███████║██████╔╝██████╔╝
██║     ██╔══██║██╔══██║   ██║       ╚════╝    ██╔══██║██╔═══╝ ██╔═══╝ 
╚██████╗██║  ██║██║  ██║   ██║                 ██║  ██║██║     ██║     
 ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝                 ╚═╝  ╚═╝╚═╝     ╚═╝     
-----------------------------------------------------------------------
Comandos

\a mensagem : Envia uma mensagem global anonima
\p usuario : Entra em um chat privado com usuario
\w usuario mensagem : Envia um sussuro para um usuario
-----------------------------------------------------------------------

)";

// funcoes utilizadas
void send_messages(int client_socket);
void receive_messages(int client_socket);