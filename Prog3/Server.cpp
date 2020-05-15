//
// Created by Jonathan Young on 5/10/2020.
// Server code that emulates hw3
//

#include "Socket.h"
#include <stdlib.h>

using namespace std;

void server();

void usage(char progName[]);

Socket *sock;

// argument acceptance and validation
int main(int argc, char *argv[]) {
    if (argc > 1) {
        sock = new Socket(atoi(argv[1]));
        if (argc == 2)
            server();
    } else {
        usage(argv[0]);
        return -1;
    }
    return 0;
}

// server code -- must use the <shutdown> system call
void server() {

    // Get a server sd
    int serverSd = sock->getServerSocket();

    // Exchange data
    char message[1500];
    read(serverSd, message, 10); // expect a 10B transmission from client
    write(serverSd, message, 10); // send a 10B transmission to client

    // serverside shutdown call - send FIN call to client
    shutdown(serverSD, SHUT_WR);

    read(serverSd, message, 1450); // expect a 1450B transmission from client


    // Close socket but not send FIN.
    //close(serverSd); // hw3 server model does not sent close
}


void usage(char progName[]) {
    cerr << "usage:" << endl;
    cerr << "server invocation: " << progName << " ipPort" << endl;
}
