/*
    Author: Dakota Sadler
    
    Name: UDP Geometry Calculator
    Description: Client Server application that allows a client to request geometrical calculations from a server. 
    Utilizes UDP sockets for client server communications
    CS447
*/

#include <sys/types.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>
#include <bits/stdc++.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;

void error(const char *);
string* loadFile(string filename);

int main(int argc, char *argv[]){
    int sock, n, portno;
    unsigned int length;

    char sourcebuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
    struct sockaddr_in server, from;
    struct hostent *serv;
    char buffer[2048];

    int exit = 0;
    
    //Reading from client.conf file
    if (argc < 2) { error("No file Provided"); }
    string filename = string(argv[1]);
    string * conFileData;
    conFileData = loadFile(filename);
    string IP = conFileData[1].substr(11);
    portno = stoi(conFileData[2].substr(12));

    //IP Setup
    hostname = gethostname(sourcebuffer, sizeof(sourcebuffer));
    host_entry = gethostbyname(sourcebuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    string sourceIP = string(IPbuffer);

    //Socket Setup
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    server.sin_family = AF_INET;
    IP.erase(IP.size() - 1);    
    serv = gethostbyname(IP.c_str()); 
    if (serv==0) error("Unknown host");
    bcopy((char *)serv->h_addr, (char *) &server.sin_addr, serv->h_length);
    server.sin_port = htons(portno);
    length=sizeof(struct sockaddr_in);

    //Client communication with server (UDP)
    while(exit != 1){
        //User Input
        cout << "Please enter your message: ";
        bzero(buffer,2048);
        fgets(buffer,255,stdin);

        //Sending user input to server
        n = sendto(sock,buffer, strlen(buffer),0,(const struct sockaddr *)&server,length);
        if (n < 0) error("Sendto");
        cout << "  " << endl;

        //Response from Server
        n = recvfrom(sock,buffer,2048,0,(struct sockaddr *)&from, &length);
        server.sin_port = from.sin_port;
        if (n < 0) error("recvfrom");
        cout << buffer << endl;
        string message = string(buffer);
        
        //Exit
        if(message.rfind("200 BYE",0) == 0){ exit = 1; }
        bzero(buffer,2048);
        cout << " " << endl;
    }
    
    cout << "Client Program Now Ending, Goodbye!" << endl;
    close(sock);
    return 0;
    
}

//Loading the Configuration File
string* loadFile(string filename){
    vector<string> lines;
    string line;
    ifstream input_file(filename);
    static string lineData[3];

    if(!input_file.is_open()) {
        error("Could not open file");
    }
    while (getline(input_file, line)){
        lines.push_back(line);
    }
    int i = 0;
    for (auto it : lines) {
        lineData[i] = it;
        i++; 
    }
    input_file.close();
    return lineData;

}

//Error handling
void error(const char *msg){
    perror(msg);
    exit(0);
}