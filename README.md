# Chat System

Este projeto é um sistema de chat em C++ que utiliza sockets para comunicação entre servidor e cliente.

## Membros do Grupo
- Marcus Vinicius da Silva - NUSP: 13833150
- Rodrigo Rodrigues de Castro - NUSP: 13695362
- Karine Cerqueira Nascimento - NUSP: 13718404
- André Jacob Suaide - NUSP: 13864673

## Informações do Sistema
- Linux Kernel 6.8.0
- Sistema Operacional: Ubuntu 24.04 LTS LTS
- Compilador utilizado: g++ 11.3.0

## Instruções de Compilação e Execução

### Compilação
Para compilar o projeto, execute o comando:

```
$ make all
```

Com isso, será necessário rodar um programa para o servidor e o restante para os clientes:

```
$ ./server
```

Em seguida roda o programa para cada cliente requisitado

```
$ ./client 
$ ./client 
$ ./client 
...
```

## Estrutura do Projeto
- `src/server.cpp`: Implementação do servidor.
- `src/server.hpp`: Cabeçalhos e definições usadas pelo servidor.
- `src/client.cpp`: Implementação do cliente.
- `src/client.hpp`: Cabeçalhos e definições usadas pelo cliente.

## Observações Adicionais
- Este projeto utiliza ANSI escape code para customizar as saidas no terminal, com cores e estilos. Exite uma variavel `style = 1` em `server.hpp` para remover o uso de estilo é so configurar essa varivael com `0`. 

- Este projeto utiliza multithreading para gerenciar conexões com múltiplos clientes, suportando no maximo 8 clientes. Para aumentar o numero de clientes deve-se mudar a constante `MAX_CLIENTS` em `server.hpp`, mas deve-se adaptar o restante do código.

- O chat privado é implementado com um mecanismo de comandos para mensagens, o cliente pode executar 4 comandos:

```
\a mensagem : Envia uma mensagem global anonima
\p usuario : Entra em um chat privado com usuario
\w usuario mensagem : Envia um sussuro para um usuario
\exit : Sair do chat
```
