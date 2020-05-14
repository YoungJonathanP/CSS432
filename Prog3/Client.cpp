//
// Created by Jonathan Young on 5/10/2020.
// Client code that emulates hw3_demo.cpp
//

#include "Socket.h"
#include <stdlib.h>

using namespace std;

void client( char ipName[] );
void usage( char progName[] );

Socket *sock;

int main( int argc, char* argv[] ) {
    if ( argc > 1 ) {
        sock = new Socket( atoi( argv[1] ) );
        if (argc == 3){
            client(argv[2]);
        }
    } else {
        usage( argv[0] );
        return -1;
    }
    return 0;
}


void client( char ipName[] ) {

    // Get a client sd
    int clientSd = sock->getClientSocket( ipName );

    // Exchange data
    char message[1500];
    write( clientSd, message, 1500 );
    read( clientSd, message, 1 );

    // Close socket to send FIN.
    close( clientSd );
}

void usage( char progName[] ) {
    cerr << "usage:" << endl;
    cerr << "client invocation: " << progName << " ipPort ipName" << endl;
}
