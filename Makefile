all: compile 

compile:
	g++ -pthread -o client client.cpp
	g++ -pthread -o server server.cpp

clear:
	rm client
	rm server