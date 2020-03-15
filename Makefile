CC = g++
CFLAGS = -std=c++11 
 
all:server.o client.o
	$(CC) $(CFLAGS) server.o -o server -lpthread
	$(CC) $(CFLAGS) client.o -o client -lpthread
 
client.o: client.cpp command.hpp msg.hpp head.hpp socket.hpp
	$(CC) $(CFLAGS) -c	client.cpp
	
server.o: server.cpp command.hpp msg.hpp head.hpp socket.hpp
	$(CC) $(CFLAGS) -c	server.cpp
 
clean:
	rm -f *.o server client