/*
 * Created by Jonathan Young on 4/12/2020.
 * CSS432 Program 1: Socket Programming - Server:
 * The purpose of this code has three purposes
 * 1- Utilize various socket-related system calls
 * 2- Create a multi-threaded server
 * 3- Evaluate the throughput of different mechanisms when using TCP/IP
 *    to do point-to-point communication over a network.
 *
 * --This program you will use the Client-Server Model where a client process
 * establishes a connection to a server, sends data or requests, and
 * closes the connection. The server will accept the connection and create a
 * thread to service the request and then wait for another connection on the
 * main thread. Servicing the request consists of:
 * (1) reading the number of iterations the client will perform
 * (2) reading the data sent by the client
 * (3) sending the number of reads which the server performed.
 */

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
#include <pthread.h>

using namespace std;

// set buffer size to 1500 bytes
const int BUFSIZE = 1500;
// set max number of connections at specified port-- part of listener
const int NUM_CONNECTIONS = 5;

/*
 * Server only takes one argument- Port: Server IP port
 */
int main(int argc, char *argv[]) {
    // socket consists of a port and a server name
    //int ServerPort;
    //char *servername;
    char databuf[BUFSIZE];
    int convertCount;
    // zero the databuf
    bzero(databuf, BUFSIZE);

    int receivedIterations = 0;


    // TODO -- Assign pthread data to allow for multiple connections

    // Build the IP Address, this establishes where listening will be done--
    // May need to be changed to prevent collisions
    int port; // port request should match client port -- test  = 51278
    // Argument verification
    if (argc != 2) {
        cerr << "Please enter valid port for " << argv[0] << endl;
    }
    port = stoi(argv[1]);
//    // Verifies that the port is within valid access parameters
//    // (49152 < port < 65535)
//    if (port < 49152 || port > 65535){
//        cerr << "Usage: " << argv[0] << " is not accessing a valid"
//                                        " port value." << endl;
//        return -1;
//    }

    // data structure where address can be stored and binded to--
    // sockaddr_in is a struct
    sockaddr_in acceptSocketAddress;
    // zero out the accept socket address
    bzero((char *) &acceptSocketAddress, sizeof(acceptSocketAddress));
    // sets family of socket to be usable with the internet --
    // AF_INET is the internet
    acceptSocketAddress.sin_family = AF_INET;
    // allows ANY ip address to be accepted by the server socket
    // -- INADDR_ANY server should listen for any ip address trying to connect
    // htonl changes format of address to something network can understand
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // set socket port to port int variable (12345)
    // htons converts unsigned short integer hostshort from host byte order
    // to network byte order
    acceptSocketAddress.sin_port = htons(port);

    // Create Socket -- AF_INET says to use the internet --
    // SOCK_STREAM says to use TCP IP
    // what is returned is index to file descriptor table (expected 3)
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    // helps when you shutdown the server and assists with debugging.
    // Also allows for quick reuse. This allows the port to be free sooner
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;

    // Bind, Listen, Accept
    // bind serverSD socket to address.
    // serverSD first argument to bind, which is an index to file
    // descriptor table
    // acceptSocketAddress is address where (we) listen
    int rc = bind(serverSD, (sockaddr *) &acceptSocketAddress,
                  sizeof(acceptSocketAddress));

    // Listen
    // assign serverSD socket to have up to 5 listeners(or connections)
    listen(serverSD, NUM_CONNECTIONS);

    // Accept
    // creates accept- what is returned is socket of client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    // newSD is returned socket that points back to the client.
    // This portion creates the accept
    // This is also considered a blocking call, this will stop the
    // actions on this side until accept is handled
    int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
    cout << "Accepted Socket #: " << newSD << endl;

    // allows for read and write back to client. Read from socket (newSD),
    // reading to (databuf), read up to (BUFFSIZE).
    // What is returned from the read, is the number of bytes read
//    int bytesRead = read(newSD, databuf, BUFSIZE);
    // print out of how many bytes were read
//    cout << "Bytes Read: " << bytesRead << endl;
    // write out first character of bytes
//    cout << databuf[0] << endl;

    // ensures that read retrieves all BUFSIZE
//    int count = 0;
//    for (int nread = 0; (nread += read(newSD, databuf, BUFSIZE - nread)) <
//    BUFSIZE; count++);

    int retStatus = read(serverSD, &convertCount, sizeof(convertCount));

    int nRead;
    for (int i = 0; i <= retStatus; i++) {
        nRead = 0;
        while (nRead < BUFSIZE) {
            int bytesRead = read(newSD, databuf, BUFSIZE - nRead);
            nRead += bytesRead;
        }
    }

    // sends information about how many reads were made
    convertCount =  htonl(nRead);
    write(newSD, &convertCount, sizeof(convertCount));

    // set char 13 to 'R'
//    databuf[13] = 'R';
    // write this back to client. Write to socket (newSD), write updated
    // databuf (13 = 'R')
//    int bytesWritten = write(newSD, databuf, BUFSIZE);
    // print out how many bytes were written
//    cout << "Bytes Written: " << bytesWritten << endl;



    // close client socket
    close(newSD);
    // close accept socket
    close(serverSD);

    return 0;
}