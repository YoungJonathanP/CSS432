//
// Created by Jonathan Young on 5/10/2020.
// Server code that emulates hw3_demo.cpp
//

#include "Socket.h"
#include <stdlib.h>

using namespace std;

void server( );
void usage( char progName[] );

Socket *sock;

int main( int argc, char* argv[] ) {
    if ( argc > 1 ) {
        sock = new Socket( atoi( argv[1] ) );
        if ( argc == 2 )
            server( );
    }
    else {
        usage( argv[0] );
        return -1;
    }
    return 0;
}

// server code -- must use the <shutdown> system call
void server( ) {

    // Get a server sd
    int serverSd = sock->getServerSocket( );

    // Exchange data
    char message[1500];
    read( serverSd, message, 1500 );
    write( serverSd, message, 1 );

    // serverside shutdown call
    shutdown(serverSD, SHUT_WR);

    // Close socket but not send FIN.
    close( serverSd );
}


void usage( char progName[] ) {
    cerr << "usage:" << endl;
    cerr << "server invocation: " << progName << " ipPort" << endl;
    //cerr << "client invocation: " << progName << " ipPort ipName" << endl;
}
