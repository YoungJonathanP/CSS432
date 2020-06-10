/*
 Created by Jonathan Young on 5/22/2020.
 Domain Name Service CSS432 Dimpsey
This assignment is to design and code a program that enables a server to
check the integrity of a client connection by looking for IP address spoofing.
Through this assignment, you are going to learn how to use functions for
accessing the Domain Name System (DNS), such as getpeername, gethostbyaddr,
and for converting addresses, such as inet_ntoa, ntohs, and inet_addr.

*/
#include "Socket.h"
#include <stdlib.h>

using namespace std;

void server();

Socket *sock;

// argument acceptance and validation
int main(int argc, char *argv[]) {
    // port assignment should not be on registered or reserved
    // ports: 49152-65535
    if (argc > 1 && argv[1] > 49251 && argv[1] < 65536) {
        // Instantiate a TCP socket
        sock = new Socket(atoi(argv[1]));
        if (argc == 2)
            server();
    } else {
        usage(argv[0]);
        return -1;
    }
    return 0;
}

void server(){
//    sockaddr_in acceptSocketAddress;
//    bzero((char *) &acceptSocketAddress, sizeof(acceptSocketAddress));

//    acceptSocketAddress.sin_family = AF_INET;
//    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
//    acceptSocketAddress.sin_port = htons(port);

    //int serverSD = socket(AF_INET, SOCK_STREAM, 0);

    // Set the resuseaddr option-- reduces reliability but better for testing
//    const int on = 1;
//    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));

    // bind socket
//    int rc = bind(serverSD, (sockaddr *) &acceptSocketAddress, sizeof
//    (acceptSocketAddress));

//    listen(serverSD, 5);

    // initiate infinite loop to accept and process connections
    while(true){
        // 1. Accept a new connection from a client through accept()
        int serverSd = sock->getServerSocket();

        sockaddr_in clientAddr;
        socklen_t newSockAddrSize = sizeof(clientAddr);

        // 2. Spawn a child process through fork()
        pid_t pid = fork();
        if (pid == 0){ // pid is child
            //child continues checking the integrity of this connection

            // 3. Retrieve the client's IP address and port of this connection
            // through getpeername()
            // -- returns an int value 0 on success else -1
            getpeername(serverSd, (sockaddr *) &clientAddr, newSockAddrSize);

            // 4. Retrieve the client's hostent data structure through
            // gethostbyaddr()
            // -- returns a pointer to the entry for a host (hostent)
            // maintained by a DNS server
            struct hostent *hostPtr = gethostbyaddr(const void *addr,
                    sizeof(unsigned int), AF_INET) // first argument needs ot
                            // be -- unsigned int addr of a client IP address
                            // string that has been converted into an
                            // unsigned integer through inet_addr()


            // 5. Retrieve the client's official name, aliases, and
            // registered IP addresses from the hostent
            const char *officialName = hostPtr->h_name;
            char **aliasList = hostPtr->h_aliases;
            string *addList;
            in_addr *currAddress;
            // loop to store address list in string list
            for (int i = 0; currAddress = (in_addr *) hostPtr->h_addr_list[i]
                    != nullptr; i++){
                addList[i] = inet_ntoa(*currAddress);
            }

            // 6. Decide whether this client is a honest or a spoofing client


            //  matching its IP address retrieved from getpeername() and the
            //  list of addresses retrieved via gethostbyaddr()

            // if the client's IP address of this connection matches one of
            // the listed addresses in hostent, you can trust this client


            // 7. Terminate this child process


        } else if( pid > 0){ // pid is parent
            //parent closes this connection and goes back to the top of the loop
            close(serverSd);
        } else { // fork failed
            // exit connection if failed?
            cout << "fork() connection failed" << endl;
            return 1;
        }
    }
}