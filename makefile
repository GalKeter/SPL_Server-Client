CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: StompWCIClient

EchoClient: bin/ConnectionHandler.o bin/echoClient.o
	g++ -o bin/EchoClient bin/ConnectionHandler.o bin/echoClient.o $(LDFLAGS)

StompWCIClient: bin/event.o bin/ConnectionHandler.o bin/Channel.o bin/StompProtocol.o bin/StompClient.o
	g++ -o bin/StompWCIClient bin/event.o bin/ConnectionHandler.o bin/Channel.o bin/StompProtocol.o bin/StompClient.o $(LDFLAGS)

bin/ConnectionHandler.o: src/ConnectionHandler.cpp 
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/echoClient.o: src/echoClient.cpp
	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Channel.o: src/Channel.cpp
	g++ $(CFLAGS) -o bin/Channel.o src/Channel.cpp


.PHONY: clean
clean:
	rm -f bin/*
	