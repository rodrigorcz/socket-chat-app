all: compile 

compile: 
	g++  -pthread -o client src/client.cpp
	g++  -pthread -o server src/server.cpp

clear:
	rm client
	rm server