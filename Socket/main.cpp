#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <w32api/psdk_inc/_ip_types.h>

using namespace std;

// set buffer size
const int BUFFSIZE = 1500;
// set max number of connections at specified port-- part of listener
const int NUM_CONNECTIONS = 5;

int main(int argc, char *argv[]) {
    // socket consists of a port and a server name
    int ServerPort;
    char *servername;
    char databuf[BUFFSIZE];
    // zero the databuf
    bzero(databuf, BUFFSIZE);

    // Build the Address
    int port = 12345;
    // data structure where address can be stored and binded to
    sockaddr_in acceptSocketAddress;
    // zero out the accept socket address
    bzero((char *) &acceptSocketAddress, sizeof(acceptSocketAddress));
    // sets family of socket to be usable with the internet
    acceptSocketAddress.sin_family = AF_INET;
    // allows ANY ip address to be accepted by the server socket
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // set socket port to port int variable (12345)
    acceptSocketAddress.sin_port = htons(port);

    // open the socket -- then bind to the address
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    // helps when you shutdown the server and assists with debugging. Also allows for quick reuse.
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));

    // bind serverSD socket to address.
    int rc = bind(serverSD, (sockaddr *) &acceptSocketAddress, sizeof(acceptSocketAddress));

    // assign serverSD socket to have up to 5 listeners
    listen(serverSD, NUM_CONNECTIONS);

    // creates accept- what is returned is socket of client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    // newSD is returned socket that points back to the client. This portion creates the accept
    // This is also considered a blocking call, this will stop the actions on this side until accept is handled
    int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);

    // allows for read and write back to client. Read from socket (newSD), reading to (databuf), read up to (BUFFSIZE).
    // What is returned from the read, is the number of bytes read
    int bytesRead = read(newSD, databuf, BUFFSIZE);
    // print out of how many bytes were read
    cout << "Bytes Read: " << bytesRead << endl;
    // write out first character of bytes
    cout << databuf[0] << endl;

    // set char 13 to 'R'
    databuf[13] = 'R';
    // write this back to client. Write to socket (newSD), write updated databuf (13 = 'R')
    int bytesWritten = write(newSD, databuf, BUFFSIZE);

    // close client socket
    close(newSD);
    // close accept socket
    close(serverSD);

    return 0;
}