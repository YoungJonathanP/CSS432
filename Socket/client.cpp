//
// Created by Jonathan Young on 4/12/2020.
// Sockets-Client - client program to demonstrate sockets usage
// CSS 432
//
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

using namespace std;

// size of buffer we read and write into
const int BUFFSIZE = 1500;

int main(int argc, char *argv[]){
    // character pointer of server name -- server name example = UW1-320-06
    char *serverName;
    // port server is waiting on (socket is defined by: IP address and Port== unique port in world to connect to)
    char serverPort[6] = "12345";
    // character pointer to databuf that will be read and written into
    char *databuf;
    //
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    // index into file descriptor table, where socket is kept once client connects to server
    int clientSD = -1;

    /*
     * Argument Validation
     * Makes sure that name of server wanted to connect to is passed through (2 arguments required)
     * One argument passed is name of program, other is name of server
     */
    if (argc != 2){
        cerr << "Usage: " << argv[0] << "serverName" << endl;
        return -1;
    }

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
     * This addrinfo structure has internet address which can be sued to create a socket too
     */
    serverName = argv[1];

    // setup for hint structure, which is also an address info structure
    //
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = 0;                 // Optional options -- 0 means no optional arguments used
    hints.ai_protocol = 0;              // Allow any protocol
    // rc is results of different internet address where socket can be
    // getaddrinfo calls serverName and serverPort which uniquely defines socket --
    // hints structure is passed to as an argument to give information about sockets
    // results is returning parameter as a pointer to list of different address info structures that are filled in
    // resolves internet address given port, servername, and hint structure
    int rc = getaddrinfo(serverName, serverPort, &hints, &result);
    if (rc != 0){
        cerr<< "ERROR:" << gai_strerror(rc) << endl;
    }

    /*
     * Iterate through addresses and connect
     * This cycles through results to see if any of them are connectable
     */
    // rp is pointer to addrinfo struct
    // then interate through a linked set of address infos, looking for one to connect to
    for (rp = result; rp != NULL; rp = rp->ai_next){ // if clientSD == -1 pushes rp to next address
        // create socket from first info that is passed back
        // socket call has - domain = ai_family, - socket type = ai_socktype (ussualy SOCK_STREAM),
        // -protocol = ai_protocol
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        // if socket is created successfully, will get an index into file descriptor table
        if (clientSD == -1){
            continue;
        }
        /*
         * A socket has been successfully created
         */
        // clientSD is not = -1 allows for connect call to be made
        // connect to socket (clientSD) with information about address that was passed back (ai_addr)
        // when connect succeeds, server accepts connection and creates socket that points back to client,
        // loops back around and accepts again (in this case the server just accepts the client)
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0){
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        } else {  // success
            break;
        }
    }

    // after dropping out of loop, rp will either be NULL, meaning we didn't find the server socket
    // or rp is not NULL and we print out the socket found
    if (rp == NULL){
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "Client Socket: " << clientSD << endl;
    }
    // free the result. Pointer was passed to getaddrinfo, filled with some information and needs to be freed
    freeaddrinfo(result);

    /*
     * Write and read data over network
     */
    // now we have a socket, and we can read and write to that socket
    //
    databuf = new char[BUFFSIZE];
    // fill up databuf with z characters
    for (int i = 0; i < BUFFSIZE; i++){
        databuf[i] = 'z';
    }

    // write to server databuf info up to BUFFSIZE
    int bytesWritten = write(clientSD, databuf, BUFFSIZE);
    cout << "Bytes Written: " << bytesWritten << endl;

    // server writes back information to client
    // read from socket (clientSD) into databuf (should be 1500 bytes read)
    int bytesRead = read(clientSD, databuf, BUFFSIZE);
    cout << "Bytes Read: " << bytesRead << endl;
    cout << databuf[13] << endl; // prints out 13th value in databuf -- server assigned 'R' to 13th character

    // close client socket when done
    close(clientSD);
    return 0;
}