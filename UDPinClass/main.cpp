// UDPServer code

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

const int MSGSIZE = 1500;

int main() {
    char message[MSGSIZE];
    // socket descriptor
    int sd;
    int port;
    struct sockaddr_in myAddr;

    // socket properties
    sd = socket(AF_INET, SOCK_DGRAM, 0);

    // port assignment
    port = 12345;

    // create local address and zero it out
    bzero((char*)&myAddr, sizeof(myAddr));
    myAddr.sin_family = AF_INET; // this sets that we are using the internet
    // htonl changes network bit ordering
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = port;

    bind (sd, (sockaddr*) &myAddr, sizeof(myAddr));

    while (true){
        int bytesRead = recv(sd, message, MSGSIZE, 0);
        cout << "Bytes Read: " << bytesRead << endl;
        cout << message[13] << endl;
    }

    return 0;
}