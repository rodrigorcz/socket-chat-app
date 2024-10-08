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
#include <map>
#include <atomic>

// definicao de constantes
#define PORT 8080         // porta em que o servidor escutara
#define BUFFER_SIZE 1024  // tamanho do buffer para comunicacao
#define MAX_CLIENTS 8     // numero maximo de clientes conectados

// variavel para habilitar ANSI codes no terminal (cores e estilos): 1 = ativado; 0 = desativado
int style = 1; 

int current_clients = 0;         // contador de clientes conectados
std::mutex mutex_clt;            // mutex para proteção de acesso a recursos compartilhados
std::atomic<bool> running(true); // variavel atomica 'running' que controla a execucao da thread

std::vector<int> clients_list; // Lista de clientes conectados

// mapeamentos de informações de clientes 
std::map<int, bool> privates_chats;            //(key: client_socket, value: bool (esta no chat privado))
std::map<std::string, int> clients_info;       //(key: client_name  , value: client_socket)
std::map<int, std::string> clients_networking; //(key: client_socket, value: client_name) 

std::string header = R"(
-----------------------------------------------------------------------
 ██████╗██╗  ██╗ █████╗ ████████╗               █████╗ ██████╗ ██████╗ 
██╔════╝██║  ██║██╔══██╗╚══██╔══╝              ██╔══██╗██╔══██╗██╔══██╗
██║     ███████║███████║   ██║       █████╗    ███████║██████╔╝██████╔╝
██║     ██╔══██║██╔══██║   ██║       ╚════╝    ██╔══██║██╔═══╝ ██╔═══╝ 
╚██████╗██║  ██║██║  ██║   ██║                 ██║  ██║██║     ██║     
 ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝                 ╚═╝  ╚═╝╚═╝     ╚═╝     
-----------------------------------------------------------------------)";

// vetor de cores pre estabelecidas
std::vector<std::string> colors = {
    "\033[38;5;196m[",  // Vermelho
    "\033[38;5;202m[",  // Laranja
    "\033[38;5;226m[",  // Amarelo
    "\033[38;5;082m[",  // Verde
    "\033[38;5;051m[",  // Azul
    "\033[38;5;201m[",  // Magenta
    "\033[38;5;045m[",  // Ciano
    "\033[38;5;244m["   // Cinza
};

int g; // lixo

// funcoes utilizadas
void rotine_client(int client_socket);
bool process_command(std::string buffer, int client_socket);
void send_whisper(std::string buffer, int client_socket);
void send_anonymous(std::string buffer, int client_socket);
bool private_chat(std::string buffer, int client_socket);
void rotine_private_chat(int client_socket, int receptor_socket, std::string name_client);
void remove_client(int client_socket, std::string name_client);