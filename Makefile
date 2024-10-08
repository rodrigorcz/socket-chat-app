all: compile 

compile:
	g++ -lpthread -pthread -o client client.cpp
	g++ -lpthread -pthread -o server server.cpp

clear:
	rm client
	rm server