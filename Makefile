all: compile 

compile:
		g++ -pthread -o client client.cpp
		g++ -pthread -o server server.cpp

run:
		./server
		./client

clear:
		rm client
		rm server