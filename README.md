# Chat System

This project is a chat system in C++ that uses sockets for communication between the server and clients.

![Screenshot From 2025-01-15 14-33-20](https://github.com/user-attachments/assets/11f4a0ed-f70d-4091-ad15-d2cd1e8a7762)


## Compilation and Execution Instructions

### Compilation
To compile the project, run the command:

```
$ make all
```

Then, you need to start the server program and run separate programs for each client:

```
$ ./server
```

Next, start the program for each required client:

```
$ ./client 
$ ./client 
$ ./client 
...
```

## Project Structure
- `src/server.cpp`: Server implementation.
- `src/server.hpp`: Headers and definitions used by the server.
- `src/client.cpp`: Client implementation.
- `src/client.hpp`: Headers and definitions used by the client.

## Additional Notes
- This project uses ANSI escape codes to customize terminal output with colors and styles. There is a variable `style = 1` in `server.hpp`. To disable styles, set this variable to `0`.

- This project employs multithreading to manage connections with multiple clients, supporting up to 8 clients by default. To increase the number of clients, you need to modify the `MAX_CLIENTS` constant in `server.hpp` and adapt the rest of the code accordingly.

- Private chat functionality is implemented with a command mechanism for messages. Clients can use the following four commands:

```
\a mesage :  Sends a global anonymous message
\p user : Initiates a private chat with the user
\w user mensagem : Sends a whisper to the user 
\exit : Exits the chat
```

