#ifndef STRUCTURES_H
#define STRUCTURES_H 1

#include "helpers.h"

#include <bits/stdc++.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define MAX_BUFFER 1551

struct UDP{
    char topic[50];
    uint8_t data_type;
    char content[1501];
} __attribute__((packed));

struct TCP{
    char IP[16];
    uint16_t port;
    char topic[51];
    char data_type[11];
    char content[1501];
} __attribute__((packed));

struct Subscriber{
    string ID;
    int socket;
    bool connection;
    bool Store_And_Forward;
    vector<TCP> next_message;

    Subscriber (string ID_client, int SF, int socket);

    vector<TCP> &get_next_message ();
    string get_ID ();
    void set_SF (int SF);
    int get_socket ();
    bool is_connected ();
    void connect (int socket);
    void disconnect ();
    bool has_SF ();
    void add_next_message (TCP tcp_message);
    void clear_next_message ();
};

struct Topic{
    string topic;
    vector<Subscriber> subscribers;

    Topic (string topic_name);

    string get_topic_name ();
    void set_topic_name (string topic_name);
    vector<Subscriber> &getSubscribers ();
    void subscribe (Subscriber subscriber);
    void unsubscribe (string ID_client);
    bool is_subscribed (string ID_client);
    bool has_activated_SF (string ID_client);
};

struct Home{
    vector<Topic> topics;

    Home();
    
    vector<Topic> get_topics ();
    Topic &get_topic_name (string topic_name);
    bool topic_exists (string topic_name);
    void create_topic (string topic_name);
};

#endif