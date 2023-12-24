#include "structures.h"

Subscriber::Subscriber (string ID_client, int SF, int socket){
    ID = ID_client;
    this -> socket = socket;
    connection = true;
    if (SF == 1)
        Store_And_Forward = true;
    else
        Store_And_Forward = false;
}
    
string Subscriber::get_ID (){
    return ID;
}

void Subscriber::set_SF (int SF){
    if(SF == 0)
        Store_And_Forward = false;
    else 
        Store_And_Forward = true; 
}

bool Subscriber::is_connected (){
    return connection;
}

int Subscriber::get_socket (){
    return socket;
}

void Subscriber::connect (int socket){
    socket = socket;
    connection = true;
}

void Subscriber::disconnect (){
    connection = false;
}

vector<TCP> &Subscriber::get_next_message (){
    return next_message;
}

bool Subscriber::has_SF (){
    return Store_And_Forward;
}

void Subscriber::add_next_message (TCP tcp_message){
    next_message.push_back (tcp_message);
}

void Subscriber::clear_next_message (){
    next_message.clear ();
}

Topic::Topic (string topic_name){
    topic = topic_name;
}
    
string Topic::get_topic_name (){
    return topic;
}

void Topic::set_topic_name (string topic_name){
    topic = topic_name;
}

vector<Subscriber> &Topic::getSubscribers (){
    return subscribers;
}

bool Topic::is_subscribed (string ID_client){
    for (auto subscriber : subscribers)
    {
        if(subscriber.get_ID() == ID_client) 
            return true;
    }
    return false;
}

void Topic::subscribe (Subscriber subscriber){
    if (is_subscribed (subscriber.get_ID ().c_str ()))
        return;

    subscribers.push_back(subscriber);
}

void Topic::unsubscribe (string ID_client){
    for(int i = 0; i < subscribers.size (); i++) 
    {
        if(subscribers[i].get_ID () == ID_client)
            subscribers.erase (subscribers.begin () + i);
    }
}

bool Topic::has_activated_SF (string ID_client){
    for(auto subscriber : subscribers)
    {
        if(subscriber.get_ID () == ID_client)
            return subscriber.has_SF ();
    }
    pthread_exit (NULL);
} 

Home::Home (){

}
    
vector<Topic> Home::get_topics (){
    return topics;
}

Topic &Home::get_topic_name (string topic_name){
    for(auto &topic : topics)
    {
        if(topic.get_topic_name () == topic_name)
            return topic;
    }
    pthread_exit (NULL);
}

bool Home::topic_exists (string topic_name){
    for(auto topic : topics)
    {
        if(topic.get_topic_name () == topic_name)
            return true;
    }
    return false;
}

void Home::create_topic (string topic_name){
    if(!topic_exists (topic_name))
    {
        Topic newTopic (topic_name);
        topics.push_back (topic_name);
    }
}

