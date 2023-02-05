/* 
    Author: Dakota Sadler
    Server.cpp
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
#include <filesystem>
#include <dirent.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>
#include <bits/stdc++.h>
#include <cmath>

using namespace std;

struct clientList{
    string clientIP = "None";
    string shape = "None";
    int greeting = 0;
    string serverResponse = "None";
    int connectionType;
};

//Globals
int sock, length, n;
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from; 
char bufferMessage[2048];
char* ipStringServer;
char *IPbuffer;
string serverIP;
clientList client;
float PI = 3.14159;
int clientNum = 0;
string seperate = "------------------------------";

void clientManager(char* clientAddress, int newSock, char buffer[2048]);
float circleArea(float radius);
float circleCircumference(float area);
float sphereVolume(float radius);
float sphereRadius(float area);
float cylinderArea(float radius, float height);
float cylinderHeight(float volume, float radius);
string helpCommand(string message);
string* loadFile(string filename);
void error(const char *msg);

int main(int argc, char *argv[]){
    char hostbuffer[2048];
    struct hostent *host_entry;
    int hostname, pid, portno;

    //Reading from Config file
    if (argc < 2) {
        error("No file Provided");
    }
    string filename = string(argv[1]);
    string * conFileData;
    conFileData = loadFile(filename);
    portno = stoi(conFileData[1].substr(10));
    cout << portno << endl;

    //Socket Setup
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(portno);
    int portNum = server.sin_port;
    if (bind(sock,(struct sockaddr *)&server,length)<0) 
        error("binding");
    fromlen = sizeof(struct sockaddr_in);
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    serverIP = string(IPbuffer);

    //Server Start-up
    cout << " " << endl;
    cout << "Welcome to the geometric calculator, the application is now online and is awaiting client commands..." << endl;
    cout << "Server IP: " << serverIP << endl;
    cout << "Server Port: " << portno << endl;

    //Client Server Interaction
    while (1) {

        //Listening for a new client
        clientNum = clientNum + 1;
        bzero(bufferMessage,2048);
        n = recvfrom(sock,bufferMessage,2048,0,(struct sockaddr *)&from,&fromlen);

        //If a client is found
        if(n > 0){
            pid = fork();
            if(pid < 0) {
                perror("UDP Server: ERROR while forking new process.\n");
                exit(1);
            }
            if(pid == 0) {
                int newSock;
                newSock = socket(AF_INET, SOCK_DGRAM, 0);
                portNum = portNum + clientNum;
                server.sin_port=htons(portNum);
                if (bind(newSock,(struct sockaddr *)&server,length)<0) 
                    error("binding");
                int k = 0;
                while(1){
                    if(k != 0){
                        n = recvfrom(newSock,bufferMessage,2048,0,(struct sockaddr *)&from,&fromlen);
                    }
                    char* clientIP = inet_ntoa(from.sin_addr);
                    cout << " " << endl;
                    cout << seperate << endl;
                    cout << " " << endl;

                    clientManager(clientIP, newSock, bufferMessage);
                    bzero(bufferMessage,2048);
                    k++;
                }
            }
        }
    }
    return 0;
}

///// Client Management /////

void clientManager(char* clientAddress, int newSock, char buffer[2048]){
    string message = string(buffer); 
    string tempReply;
    istringstream ss(message);
    string word;
    string commandArguements[20];
    int commandCount = -1;

    client.clientIP = clientAddress;

    cout << "Client " << clientAddress << " Entered Command: " << message << endl;

    //Cuts the commands into parameters
    int i = 0;
    while(ss >> word){
        commandArguements[i] = word;
        i++;
    }
    commandCount = i;

    //Greeting commands, returns 503 error if HELO is not said
    if(commandArguements[0].compare("HELO") == 0 && commandArguements[1].compare(serverIP) == 0){
        tempReply = "200 HELO " + client.clientIP + "(UDP)";
        client.greeting = 1;
    }
    
    else if(client.greeting == 1){
        if(commandArguements[0].compare("HELP") == 0){
            tempReply = helpCommand(tempReply);
        }
        else if(commandArguements[0].compare("BYE") == 0 && commandArguements[1].compare(serverIP) == 0){
            tempReply = "200 BYE " + client.clientIP + "(UDP)";
        }

        //Shape Set commands
        else if(commandArguements[0].compare("CIRCLE") == 0){ //Circle Set Command
            client.shape = "Circle";
            tempReply = "210 CIRCLE Ready!";
        }else if(commandArguements[0].compare("SPHERE") == 0){ //Sphere Set Command
            client.shape = "Sphere";
            tempReply = "220 SPHERE Ready!";
        }else if(commandArguements[0].compare("CYLINDER") == 0){ //Cylinder Set Command
            client.shape = "Cylinder";
            tempReply = "230 CYLINDER Ready!";
        }
        
        //CIRCLE AREA COMMAND
        else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Circle") == 0 && commandCount == 2){
            try{
                tempReply = "250 " + to_string(circleArea(stod(commandArguements[1])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Circle") != 0 && commandCount == 2){
            tempReply = "503 Wrong Command Order: CIRCLE before AREA <r> ";
        }else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Circle") == 0 && commandCount < 2){
            tempReply = "501 Syntax Error: Too Few Arguements: AREA <r> ";
        }
        
        //CIRCLE CIRCUMFERENCE COMMAND
        else if(commandArguements[0].compare("CIRC") == 0 && client.shape.compare("Circle") == 0){
            try{
                tempReply = "250 " + to_string(circleCircumference(stod(commandArguements[1])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("CIRC") == 0 && client.shape.compare("Circle") != 0 && commandCount == 2){
            tempReply = "503 Wrong Command Order: CIRCLE before CIRC <A> ";
        }else if(commandArguements[0].compare("CIRC") == 0 && client.shape.compare("Circle") == 0 && commandCount < 2){
            tempReply = "501 Syntax Error: Too Few Arguements: CIRC <A> ";
        }
        
        //SPHERE VOLUME COMMAND
        else if(commandArguements[0].compare("VOL") == 0 && client.shape.compare("Sphere") == 0){
            try{
                tempReply = "250 " + to_string(sphereVolume(stod(commandArguements[1])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("VOL") == 0 && client.shape.compare("Sphere") != 0 && commandCount == 2){
            tempReply = "503 Wrong Command Order: SPHERE before VOL <r> ";
        }else if(commandArguements[0].compare("VOL") == 0 && client.shape.compare("Sphere") == 0 && commandCount < 2){
            tempReply = "501 Syntax Error: Too Few Arguements: VOL <r> ";
        }
        
        //SPHERE RADIUS COMMAND
        else if(commandArguements[0].compare("RAD") == 0 && client.shape.compare("Sphere") == 0){
            try{
                tempReply = "250 " + to_string(sphereRadius(stod(commandArguements[1])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("RAD") == 0 && client.shape.compare("Sphere") != 0 && commandCount == 2){
            tempReply = "503 Wrong Command Order: SPHERE before RAD <A> ";
        }else if(commandArguements[0].compare("RAD") == 0 && client.shape.compare("Sphere") == 0 && commandCount == 1){
            tempReply = "501 Syntax Error: Too Few Arguements: RAD <A> ";
        }
        
        //CYLINDER AREA COMMAND
        else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Cylinder") == 0 && commandCount > 2){
            try{
                tempReply = "250 " + to_string(cylinderArea(stod(commandArguements[1]), stod(commandArguements[2])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Cylinder") != 0 && commandCount == 3){ //MAY BE AREA OF CONFLICT
            tempReply = "503 Wrong Command Order: CYLINDER before AREA <r> <h> ";
        }else if(commandArguements[0].compare("AREA") == 0 && client.shape.compare("Cylinder") == 0 && commandCount < 3){
            tempReply = "501 Syntax Error: Too Few Arguements: AREA <r> <h> ";
        }
        
        //CYLINDER HEIGHT COMMAND
        else if(commandArguements[0].compare("HGT") == 0 && client.shape.compare("Cylinder") == 0 && commandCount > 2){
            try{
                tempReply = "250 " + to_string(cylinderHeight(stod(commandArguements[1]), stod(commandArguements[2])));
            }catch(...){ tempReply = "501 Syntax Error in Arguements"; }
        }else if(commandArguements[0].compare("HGT") == 0 && client.shape.compare("Cylinder") != 0 && commandCount == 3){ //MAY BE AREA OF CONFLICT
            tempReply = "503 Wrong Command Order: CYLINDER before HGT <V> <r>";
        }else if(commandArguements[0].compare("HGT") == 0 && client.shape.compare("Cylinder") == 0 && commandCount < 3){
            tempReply = "501 Syntax Error: Too Few Arguements: HGT <V> <r> ";
        }
        
        //UNKNOWN COMMAND COMMAND 
        else{
            tempReply = "500 – Syntax Error, Command unrecognized";
        }
    }
    if(client.greeting != 1){
        tempReply = "503 Wrong Command Order: Greeting Needed to Continue \n "
            "HELO <Hostname/HostIP>";
    }

    cout << "Server Reply: " << tempReply << endl;
    bzero(bufferMessage,2048);
    strcpy(bufferMessage, tempReply.c_str());
    n = sendto(newSock,bufferMessage,2048,0,(struct sockaddr *)&from,fromlen);
    if (n  < 0) error("sendto");
    bzero(bufferMessage,2048);
}

///// Circle Management /////
float circleArea(float radius){
    float area;
    area = PI * radius * radius;
    return area;
}
float circleCircumference(float area){
    float circumference;
    circumference = 2 * PI * sqrt(area/PI);
    return circumference;
}

///// Sphere Management /////
float sphereVolume(float radius){
    float volume;
    volume = 1.33333 * PI * radius * radius * radius;
    return volume;
}
float sphereRadius(float area){
    float radius;
    radius = 0.5 * sqrt(area/PI);
    return radius;
}

///// Cylinder Management /////
float cylinderArea(float radius, float height){
    float area;
    area = 2 * PI * radius * height + 2 * PI * radius * radius;
    return area;
}
float cylinderHeight(float volume, float radius){
    float height;
    float temp = PI * radius * radius;
    height = volume/temp;
    return height;
}

///// Help Management /////
string helpCommand(string message){
    message = "200 Help Menu: \n"
            "      > Shape Commands: \n"
            "          CIRCLE - Sets current shape to Circle \n"
            "          SPHERE - Sets current shape to Sphere \n"
            "          CYLINDER - Sets current shape to CYLINDER \n \n"

            "      > CIRCLE Commands: \n"
            "          AREA <r> - Given Radius <r>, Calculates Area of the Circle \n"
            "          CIRC <A> - Given Area <A>, Calculates Circumference of the Circle \n \n"

            "      > SPHERE Commands: \n"
            "          VOL <r> - Given Radius <r>, Calculates Volume of the Sphere \n"
            "          RAD <A> - Given Area <A>, Calculates Radius of the Sphere \n \n"

            "      > CYLINDER Commands: \n"
            "          AREA <r> <h> - Given Radius <r> and Height <h>, Calculates Area of the Cylinder \n"
            "          HGT <V> <r> - Given Volume <V> and Radius <r>, Calculates Height of the Cylinder \n \n"

            "      > MISC. Commands: \n"
            "          HELP - Returns a list of useful commands to the user \n"
            "          HELO <hostname> - Greets the host server \n"
            "          BYE <hostname> - ends interactions with the client \n \n";

    if(client.shape.compare("Circle") == 0){
        message = message + "------------------------------------------------------------ \n"
            "      > Selected Shape: Circle \n"
            "      > Currently Usable Commands: \n"
            "          AREA <r> - Given Radius <r>, Calculates Area of the Circle \n"
            "          CIRC <A> - Given Area <A>, Calculates Circumference of the Circle \n"
            "          HELP - Returns a list of useful commands to the user \n"
            "          HELO <hostname> - Greets the host server \n"
            "          BYE <hostname> - ends interactions with the client \n";
    }
    if(client.shape.compare("Sphere") == 0){
        message = message + "------------------------------------------------------------ \n"
            "      > Selected Shape: Sphere \n"
            "      > Currently Usable Commands: \n"
            "          VOL <r> - Given Radius <r>, Calculates Volume of the Sphere \n"
            "          RAD <A> - Given Area <A>, Calculates Radius of the Sphere \n \n"
            "          HELP - Returns a list of useful commands to the user \n"
            "          HELO <hostname> - Greets the host server \n"
            "          BYE <hostname> - ends interactions with the client \n";
    }
    if(client.shape.compare("Cylinder") == 0){
        message = message + "------------------------------------------------------------ \n"
            "      > Selected Shape: Cylinder \n"
            "      > Currently Usable Commands: \n"
            "          AREA <r> <h> - Given Radius <r> and Height <h>, Calculates Area of the Cylinder \n"
            "          HGT <V> <r> - Given Volume <V> and Radius <r>, Calculates Height of the Cylinder \n \n"
            "          HELP - Returns a list of useful commands to the user \n"
            "          HELO <hostname> - Greets the host server \n"
            "          BYE <hostname> - ends interactions with the client \n";
    }


    return message;
}

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

void error(const char *msg){
    perror(msg);
    exit(0);
}


