cw-2: server client
cw-3: server3 client
DIR = src/
server: $(DIR)server.cpp Rdt.o Reno.o
	g++ $(DIR)server.cpp Rdt.o Reno.o -pthread -o server
server3: $(DIR)server3-3.cpp Rdt.o Reno.o
	g++ $(DIR)server3-3.cpp Rdt.o Reno.o -pthread -o server

client: $(DIR)client.cpp Rdt.o Reno.o
	g++ $(DIR)client.cpp Rdt.o Reno.o -pthread -o client

Rdt.o: $(DIR)Rdt.cpp
	g++ -c $(DIR)Rdt.cpp
Reno.o: $(DIR)Reno.cpp
	g++ -c $(DIR)Reno.cpp
clean:
	rm server client  *.o *.jpg

