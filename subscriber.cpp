#include "structures.h"

using namespace std;

void usage(char *file) {
	fprintf(stderr, "Usage: %s <CLIENT_ID> <SERVER_IP> <SERVER_PORT>\n", file);
	exit(0);
}

int checkSubscribe(char buffer[]) {
	int topic_len = 0, free = 0;

	for(int i = 0; i < strlen(buffer); i++)
	{
		if(free == 1) 
			topic_len++; 
		if(buffer[i] == ' ') 
			free++;
	}

	if(topic_len > 51) 
		return -2;
	if(free != 2) 
		return -1;
	return 0;
}

int checkUnsubscribe(char buffer[]) {
	int free = 0;
	for(int i = 0; i < strlen(buffer); i++)
	{
		if(buffer[i] == ' ') 
			free++;
	}

	if(free != 1) 
		return -1;
	return 0;
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, MAX_BUFFER);
	int new_socket, n, out, flag = 1;
	char ID_client[15];
	struct sockaddr_in address;
	char buffer[MAX_BUFFER];
	TCP *received;

	if(argc < 4) usage(argv[0]);

	strcpy(ID_client, argv[1]);

	fd_set read_file;
	fd_set tmp_file;

	FD_ZERO(&tmp_file);
	FD_ZERO(&read_file);

	new_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(new_socket < 0, "socket");

	FD_SET(new_socket, &read_file);
	FD_SET(0, &read_file);

	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[3]));

	out = inet_aton(argv[2], &address.sin_addr);
	DIE(out == 0, "inet_aton");
	out = connect(new_socket, (struct sockaddr*) &address, sizeof(address));
	DIE(out < 0, "connect");
	out = send(new_socket, argv[1], 15, 0);
	DIE(out < 0, "send");
	out = setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	DIE(out < 0, "Neagle");

	while (1)
	{
		tmp_file = read_file; 
		
		out = select(new_socket + 1, &tmp_file, NULL, NULL, NULL);
		DIE(out < 0, "select");

		if(FD_ISSET(new_socket, &tmp_file))
		{
			memset(buffer, MAX_BUFFER, 0);

			out = recv(new_socket, buffer, MAX_BUFFER, 0);
			DIE(out < 0, "recv");

			if(strncmp(buffer, "Already connected", 17) == 0) 
				exit(0);
			
			if(strncmp(buffer, "Subscribed to topic.", 20) == 0)
			{
				cout << "Subscribed to topic." << endl;
			} else if(strncmp(buffer, "Unsubscribed from topic.", 24) == 0)
			{
				cout << "Unsubscribed from topic." << endl;
			} else if(strncmp(buffer, "Topic doesn't exist.", 20) == 0)
			{
				cout << "Topic doesn't exist" << endl;
			}
			else
			{
	            received = (TCP*)buffer;
	            printf("%s:%hu - %s - %s - %s\n", received->IP, received->port, received->topic, received->data_type, received->content);
			}
		}

		if(FD_ISSET(0, &tmp_file))
		{
			memset(buffer, 0, MAX_BUFFER);
			fgets(buffer, MAX_BUFFER - 1, stdin);

			if(strncmp(buffer, "exit", 4) == 0) 
				break;

			if(strncmp(buffer, "subscribe", 9) == 0)
			{
				out = checkSubscribe(buffer);
				if(out == -2) 
					cout << "Maximum topic length is 50" << endl;
				if(out == -1) 
					cout << "subscribe <topic> <sf>" << endl;
				if(out == 0)
				{
					out = send(new_socket, buffer, MAX_BUFFER, 0);
					DIE(out < 0, "send");
				}
			} else if(strncmp(buffer, "unsubscribe", 11) == 0)
			{
				out = checkUnsubscribe(buffer);
				if(out) cout << "unsubscrive <topic>" << endl;
				else
				{
					out = send(new_socket, buffer, MAX_BUFFER, 0);
					DIE(out < 0, "send");
				}
			} else
			{
				cout << "ERROR: Command not found.\n";
			}
		}
	}

	close(new_socket);

	return 0;
}
