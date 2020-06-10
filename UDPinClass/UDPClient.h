//
// Created by Jonathan Young on 4/27/2020.
//

#ifndef UDPINCLASS_UDPCLIENT_H
#define UDPINCLASS_UDPCLIENT_H
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
using namespace std;


const int MSGSIZE = 1500;

class UDPClient {

};

int main() {
    char message[MSGSIZE];
    // socket descriptor
    int sd;
    struct sockaddr_in destAddr;

    // socket properties
    sd = socket(AF_INET, SOCK_DGRAM, 0);

    struct hostnet* host = gethostbyname(argv[1]);
    // port assignment
    int port = 12345;

    // create local address and zero it out
    bzero((char*)&destAddr, sizeof(destAddr));
    destAddr.sin_family = AF_INET; // this sets that we are using the internet
    // htonl changes network bit ordering
    destAddr.sin_addr.s_addr = inet_addr(inet_ntoa( *(struct in_addr*)
            *host->h_addr_list));
    destAddr.sin_port = htons(port);

    for (int i = 0; i < MSGSIZE; i++){
        message[i] = 'u';
    }

    int bytesSent = sendto(sd, message, MSGSIZE, 0, (sockaddr *) &destAddr,
            sizeof(destAddr));

    bind (sd, (sockaddr*) &destAddr, sizeof(destAddr));


    return 0;
}




#endif //UDPINCLASS_UDPCLIENT_H
