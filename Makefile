# Protocoale de comunicatii:
# Laborator 7: TCP
# Echo Server
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul
PORT_SERVER = 12345

# Adresa IP a serverului
IP_SERVER = 127.0.0.1 

all: server subscriber

# Compileaza server.cpp
server: server.cpp functions.cpp

# Compileaza subscriber.cpp
subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT_SERVER}

# Ruleaza clientul
run_subscriber:
	./subscriber $(ID_CLIENT) ${IP_SERVER} ${PORT_SERVER}

clean:
	rm -f server subscriber
