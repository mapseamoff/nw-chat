all: server client

server: nw-server.cpp
	g++ -o nw-chat-server nw-server.cpp -lboost_system -lboost_thread -pthread
	
client: nw-client.cpp
	g++ -o nw-chat-client nw-client.cpp -lboost_system -lboost_thread -pthread
	
clean:
	rm -f nw-chat-client nw-chat-server
