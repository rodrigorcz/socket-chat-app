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

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 8

int style = 0;

int current_clients = 0;
std::mutex mutex_clt;

std::vector<int> clients_list;

// hash (key: client_socket, value: bool (esta no chat privado))
std::map<int, bool> privates_chats;

// hash (key: client_name, value: client_socket)
std::map<std::string, int> clients_info;

// hash (key: client_socket, value: client_name) 
std::map<int, std::string> clients_networking; 

std::string header = R"(
-----------------------------------------------------------------------
 ██████╗██╗  ██╗ █████╗ ████████╗               █████╗ ██████╗ ██████╗ 
██╔════╝██║  ██║██╔══██╗╚══██╔══╝              ██╔══██╗██╔══██╗██╔══██╗
██║     ███████║███████║   ██║       █████╗    ███████║██████╔╝██████╔╝
██║     ██╔══██║██╔══██║   ██║       ╚════╝    ██╔══██║██╔═══╝ ██╔═══╝ 
╚██████╗██║  ██║██║  ██║   ██║                 ██║  ██║██║     ██║     
 ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝                 ╚═╝  ╚═╝╚═╝     ╚═╝     
-----------------------------------------------------------------------)";

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

void rotine_client(int client_socket);
bool process_command(std::string buffer, int client_socket);
void send_whisper(std::string buffer, int client_socket);
void send_anonymous(std::string buffer, int client_socket);
bool private_chat(std::string buffer, int client_socket);
void rotine_private_chat(int client_socket, int receptor_socket, std::string name_client, std::string name_receptor_client);
void remove_client(int client_socket, std::string name_client);