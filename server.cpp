#include "structures.h"

using namespace std;

void usage(char *file) {
	fprintf(stderr, "Usage: %s <SERVER_PORT>\n", file);
	exit(0);
}   

void decodeMessage (UDP *UDP, TCP &TCP) {
	int x;
	double y;

	strncpy(TCP.topic, UDP->topic, 50);
    TCP.topic[50] = 0;

    if(UDP->data_type == 0)
	{
    	x = ntohl(*(uint32_t*)(UDP->content + 1));

        if(UDP->content[0])
            x *= -1;

        sprintf(TCP.content, "%d", x);
        strcpy(TCP.data_type, "INT");
    }

    if(UDP->data_type == 1)
	{
    	y = ntohs(*(uint16_t*)(UDP->content));
        y /= 100;

        sprintf(TCP.content, "%.2f", y);
        strcpy(TCP.data_type, "SHORT_REAL");
    }

    if(UDP->data_type == 2)
	{
		y = ntohl(*(uint32_t*)(UDP->content + 1));
        y /= pow(10, UDP->content[5]);

        if(UDP->content[0])
            y *= -1;

        sprintf(TCP.content, "%lf", y);
        strcpy(TCP.data_type, "FLOAT");
    }

    if(UDP->data_type == 3)
	{
        strcpy(TCP.content, UDP->content);
        strcpy(TCP.data_type, "STRING");
    }
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, MAX_BUFFER);

	int tcp_socket, next_tcp_socket, number_of_port;
	int udp_socket, checked, k = 1;
	char buffer[MAX_BUFFER];
	struct sockaddr_in address, client_address;
	struct sockaddr_in udp_address;
	int i, out, SF;
	char *topicName;
	char ID_client[15];
	socklen_t cliLen = sizeof(struct sockaddr_in);
	vector<Subscriber> clients;
	TCP tcp_message;
	UDP *udp_message;
	Home home;
	fd_set read_file;
	fd_set temp_file;
	int max_file;

	if(argc < 2) usage(argv[0]);
	number_of_port = atoi(argv[1]);
	DIE(number_of_port == 0, "atoi");
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_socket < 0, "Socket UDP");
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket < 0, "Socket TCP");
	memset((char *) &address, 0, sizeof(address));
	memset((char *) &udp_address, 0, sizeof(udp_address));
	address.sin_family = udp_address.sin_family = AF_INET;
	address.sin_port = udp_address.sin_port = htons(number_of_port);
	address.sin_addr.s_addr = udp_address.sin_addr.s_addr = INADDR_ANY;

	out = bind(tcp_socket, (struct sockaddr *) &address, sizeof(struct sockaddr));
	DIE(out < 0, "bind");
	out = listen(tcp_socket, 5);
	DIE(out < 0, "listen");
	out = bind(udp_socket, (struct sockaddr *) &udp_address, sizeof(udp_address));
	DIE(out < 0, "bind");

	FD_ZERO(&read_file);
	FD_SET(tcp_socket, &read_file);
	FD_SET(udp_socket, &read_file);
	FD_SET(0, &read_file);
	max_file = max(tcp_socket, udp_socket);

	while (1)
	{
		temp_file = read_file; 
		
		out = select(max_file + 1, &temp_file, NULL, NULL, NULL);
		DIE(out < 0, "select");

		for(i = 0; i <= max_file; i++)
		{
			if(FD_ISSET(i, &temp_file))
			{
				if(i == udp_socket)
				{
					// Mesaj pentru client UDP
					socklen_t sizeAddr = sizeof(struct sockaddr_in);
					memset(buffer, 0, MAX_BUFFER);
					out = recvfrom(i, buffer, MAX_BUFFER, 0, (struct sockaddr*)&udp_address, &sizeAddr);
					DIE(out < 0, "recv");
					
					// Construiesc mesajul pentru clientul TCP
					tcp_message.port = ntohs(udp_address.sin_port);
                    strcpy(tcp_message.IP, inet_ntoa(udp_address.sin_addr));
                    udp_message = (UDP*)buffer;
                    decodeMessage(udp_message, tcp_message);

                	if(home.topic_exists(tcp_message.topic))
					{
                		Topic &topic = home.get_topic_name(tcp_message.topic);
                		for(Subscriber &c : clients)
						{
                			if(c.is_connected() && topic.is_subscribed(c.get_ID()))
							{
                				out = send(c.get_socket(), (char *) &tcp_message, MAX_BUFFER, 0);
                				DIE(out < 0, "send");
                			}

                			if(!c.is_connected() && topic.has_activated_SF(c.get_ID()))
							{
                				c.add_next_message(tcp_message);
                			}
                		}
                	}
				} else if(i == 0)
				{
					memset(buffer, 0, MAX_BUFFER);
					
					if(fgets(buffer, 9, stdin))
					{
						if(strncmp(buffer, "exit", 4) == 0)
						{
							// Deconecteaza toti clientii
							for(auto client : clients)
							{
								if(client.is_connected())
								{
									memset(buffer, 0, MAX_BUFFER);
									strncpy(buffer, "Already connected", 17);
									send(client.get_socket(), buffer, MAX_BUFFER, 0);
									close(client.get_socket());
								}
							}
							return 0;
						}
						else
						{
							printf("Only command available: exit\n");
						}
					}
				} else
				{
					// Un client incearca sa se conecteze la server
					if(i == tcp_socket)
					{
						next_tcp_socket = accept(tcp_socket, (struct sockaddr *) &client_address, &cliLen);
						DIE(next_tcp_socket < 0, "accept");

						// Opresc algoritmul lui Neagle
						out = setsockopt(next_tcp_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&k, sizeof(int));
						DIE(out < 0, "Neagle");

						FD_SET(next_tcp_socket, &read_file);

						if(next_tcp_socket > max_file)
							max_file = next_tcp_socket;

						memset(ID_client, 0, 15);

						out = recv(next_tcp_socket, ID_client, 15, 0);
						DIE(out < 0, "recv");

						checked = 1;

						for(auto client : clients)
						{
							if(strcmp(client.get_ID().c_str(), ID_client) == 0 && client.is_connected())
								checked = 0;
							else if(strcmp(client.get_ID().c_str(), ID_client) == 0 &&!client.is_connected())
								checked = 2;
						}

						// Clientul a fost conectat inainte
						if(checked == 2)
						{
							printf("New client %s connected from %s:%d.\n", ID_client, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
							for(auto &client : clients)
							{
								if(strcmp(client.get_ID().c_str(), ID_client) == 0)
								{
									client.connect(next_tcp_socket);
									for(auto msg : client.get_next_message())
									{
										out = send(client.get_socket(), (char *) &msg, MAX_BUFFER, 0);
		                				DIE(out < 0, "send");
									}
									client.clear_next_message();
								}
							}
						}

						// Clientul se contecteaza pentru prima data
						if(checked == 1)
						{
							printf("New client %s connected from %s:%d.\n", ID_client, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
							Subscriber new_client(ID_client, 1, next_tcp_socket);
							clients.push_back(new_client);
						}

						// Clientul este deja online
						if(checked == 0)
						{
							printf("Client %s already connected.\n", ID_client);
							memset(buffer, 0, MAX_BUFFER);
							strncpy(buffer, "Already connected", 17);
							send(next_tcp_socket, buffer, MAX_BUFFER, 0);
						}
					} else
					{
						// Clientul a primit o comanda
						memset(buffer, 0, MAX_BUFFER);
						out = recv(i, buffer, MAX_BUFFER, 0);
						DIE(out < 0, "recv");

						// Clientul se deconecteaza
						if(out == 0)
						{
							for(int j = 0; j < clients.size(); j++)
							{
								if(clients[j].get_socket() == i)
								{ 
									cout << "Client " << clients[j].get_ID() << " disconnected." << endl;
									clients[j].disconnect();
									close(i);
								}
							}

							FD_CLR(i, &read_file);
						}  else
						{
							topicName = strtok(buffer, " ");
							topicName = strtok(NULL, " ");

							for(auto client : clients)
							{
								if(i == client.get_socket()) strcpy(ID_client, client.get_ID().c_str());
							}

							if(strncmp (buffer , "subscribe", 9)==0)
							{
								SF = atoi(strtok(NULL, " "));

								// Daca topicul nu exista, il creez
								home.create_topic(topicName);

								Topic &t = home.get_topic_name(topicName);
								for(auto &client : clients)
								{
									if(strcmp(client.get_ID().c_str(), ID_client) == 0)
									{
										client.set_SF(SF);
										t.subscribe(client);
									}
								}

								// Trimit ACK inapoi la client
								memset(buffer, 0, MAX_BUFFER);
								strncpy(buffer, "Subscribed to topic.", 21);
								out = send(i, buffer, strlen(buffer), 0);
								DIE(out < 0, "send");
							}

							if(strncmp (buffer , "unsubscribe", 11)==0)
							{

								// Incearca se dea unsubscribe de la un topic inexistent
								if(!home.topic_exists(topicName))
								{
									memset(buffer, 0, MAX_BUFFER);
									strncpy(buffer, "Topic doesn't exist.", 21);
									out = send(i, buffer, strlen(buffer) + 1, 0);
									DIE(out < 0, "send");
									continue;
								}

								// Trimit ACK
								Topic &t = home.get_topic_name(topicName);
								t.unsubscribe(ID_client);
								memset(buffer, 0, MAX_BUFFER);
								strncpy(buffer, "Unsubscribed from topic.", 25);
								out = send(i, buffer, strlen(buffer), 0);
								DIE(out < 0, "send");
							}
						}
					}
				}
			}
		}
	}


	close(tcp_socket);
	close(udp_socket);

	return 0;
}
