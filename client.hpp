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
-----------------------------------------------------------------------
Comandos

\a mensagem : Envia uma mensagem global anonima
\p usuario : Entra em um chat privado com usuario
\w usuario mensagem : Envia um sussuro para um usuario
-----------------------------------------------------------------------

)";
void send_messages(int client_socket);
void receive_messages(int client_socket);