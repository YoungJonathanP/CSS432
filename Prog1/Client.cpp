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


// build -> g++ Client.cpp -o Client
// Run -> ./Client argv[1] argv[2] argv[3] argv[4] argv[5] argv[6]
// where:   argv[1] = serverName
//          argv[2] = IP port number used by server (last 5 of student ID)
//          argv[3] = Repetition of sending a set of data buffers
//          argv[4] = Number of data buffers
//          argv[5] = Size of each data buffer (in bytes)
//          argv[6] = type of transfer scenario: 1, 2, or 3
int main(int argc, char *argv[]){
    // character pointer of server name -- server name example = UW1-320-06
    char *serverName; // argv[1]
    // port server is waiting on (socket is defined by: IP address and
    // Port== unique port in world to connect to)
    char port; // port is equal to last 5 of student ID = "78512" argv[2]
    // SID value of 78512 exceeds acceptable port values (65535) use 51278
    // number of times to send a set of data buffers argv[3]
    int repetition;
    // number of data buffers argv[4]
    int nbufs;
    // the size of each data buffer (in bytes) argv[5]
    int bufsize;
    // data type for the type of transfer scenario: expected 1, 2, or 3 argv[6]
    int type;
    // structs to hold values for address
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    // index into file descriptor table, where socket is kept once client
    // connects to server
    int clientSD = -1;

    /*
     * Argument Validation
     * Makes sure that name of server wanted to connect to is passed through
     * (6 arguments passed through plus argv[0])
     * One argument passed is name of program, other is name of server
     */
    if (argc != 7){
        cerr << "Usage: " << argv[0] << " does not provide correct number of "
                                        "arguments." << endl;
        return -1;
    }

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to
     * serverName / Port
     * This addrinfo structure has internet address which can be used to
     * create a socket too
     */
    serverName = argv[1];
    port = stoi(argv[2]);
    repetition = stoi(argv[3]);
    nbufs = stoi(argv[4]);
    bufsize = stoi(argv[5]);
    type = stoi(argv[6]);

    // Verifies that the port is within valid access parameters
    // (49152 < port < 65535)
    if (port < 49152 || port > 65535){
        cerr << "Usage: " << argv[0] << " is not accessing a valid"
                                        " port value." << endl;
        return -1;
    }

    // Verifies that repetition is performed with a valid parameter of
    // positive integers
    if (repetition < 0){
        cerr << "Usage: " << argv[0] << " did not provide the correct value "
                                        "for repetitions" << endl;
        return -1;
    }

    // Verifies that number of number of data buffers * size of data buffers
    // is equal to 1500
    if ((nbufs * bufsize) != BUFFSIZE){
        cerr << "Usage: " << argv[0] << " did not provide proper buffer "
                                        "values" << endl;
    }

    // Verifies that transfer type is within valid parameters
    if (type < 1 || type > 3){
        cerr << "Usage: " << argv[0] << " did not provide the correct value "
                                        "for transfer type." << endl;
        return -1;
    }

    // character pointer to databuf that will be read and written into
    char databuf[nbufs][bufsize];


    // setup for hint structure, which is also an address info structure
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = 0;// Optional options --0 means no optional arguments used
    hints.ai_protocol = 0;              // Allow any protocol
    // rc is results of different internet address where socket can be
    // getaddrinfo calls serverName and serverPort which uniquely defines
    // socket --hints structure is passed to as an argument to give information
    // about sockets
    // results is returning parameter as a pointer to list of different address
    // info structures that are filled in
    // resolves servername, internet address given port, and hint structure
    int rc = getaddrinfo(serverName, argv[2], &hints, &result);
    if (rc != 0){
        cerr<< "ERROR:" << gai_strerror(rc) << endl;
    }

    /*
     * Iterate through addresses and connect
     * This cycles through results to see if any of them are connectable
     */
    // rp is pointer to addrinfo struct
    // then interate through a linked set of address infos, looking for one to
    // connect to
    for (rp = result; rp != NULL; rp = rp->ai_next){
        // if clientSD == -1 pushes rp to next address
        // create socket from first info that is passed back
        // socket call has - domain = ai_family, - socket type = ai_socktype
        // (ussualy SOCK_STREAM),
        // -protocol = ai_protocol
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        // if socket is created successfully, will get an index into file
        // descriptor table
        if (clientSD == -1){
            continue;
        }
        /*
         * A socket has been successfully created
         */
        // clientSD is not = -1 allows for connect call to be made
        // connect to socket (clientSD) with information about address that
        // was passed back (ai_addr) when connect succeeds, server accepts
        // connection and creates socket that points back to client, loops
        // back around and accepts again (in this case the server just accepts
        // the client)
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0){
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        } else {  // success
            break;
        }
    }

    // after dropping out of loop, rp will either be NULL, meaning we didn't
    // find the server socket or rp is not NULL and we print out the
    // socket found
    if (rp == NULL){
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "Client Socket: " << clientSD << endl;
    }
    // free the result. Pointer was passed to getaddrinfo, filled with some
    // information and needs to be freed
    freeaddrinfo(result);

    /*
     * Write and read data over network
     */
    // now we have a socket, and we can read and write to that socket
    //
//    databuf = new char[bufsize];
//    // fill up databuf with z characters
//    for (int i = 0; i < bufsize; i++){
//        databuf[i] = 'z';
//    }

    // write to server databuf info up to bufsize
//    int bytesWritten = write(clientSD, databuf, bufsize);
//    cout << "Bytes Written: " << bytesWritten << endl;

    // This will measure the time the tasks take -- using the Chrono time lib
    // -- TODO

    // calls cycles for repetition and uses type to determine the action
    for (int i = 0; i < repetition; i++){
        // type 1 determines Multiple writes -- invokes the write() system
        // call for each data buffer, thus resulting in calling as many
        // write()s as the number of data buffers (nbufs)
        if (type == 1){
            for (int j = 0; j < nbufs; j++){
                write(clientSD, databuf[j], bufsize);
            }
        }
        // type 2 determines writev -- allocates an array of iovec data
        // structures, each having its *iov_base field point to a different
        // data buffer as well as storing the buffer size in its iov_len
        // field; thereafter calls writev() to send all data buffers at once.
        if (type == 2) {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++) {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev(clientSD, vector, nbufs);
        }
        // type 3 determines single write -- allocates an nbufs-sized array
        // of data buffers, and thereafter calls write() to send this array,
        // (i.e. all data buffers) at once.
        if (type == 3){
            write(clientSD, databuf, (nbufs * bufsize));
        }
    }

    // End the timer for the tasks
    // TODO

    // server writes back information to client
    // read from socket (clientSD) into databuf (should be 1500 bytes read)
//    int bytesRead = read(clientSD, databuf, bufsize);
//    cout << "Bytes Read: " << bytesRead << endl;
//    cout << databuf[13] << endl; // prints out 13th value in databuf --
    // server assigned 'R' to 13th character

    int nreads;
    // Server sends back how many read() system calls it performed
    read(clientSD, &nreads, nbufs); // 3rd argument may be sizeof(reads)
    //cout << "There were " << readTimes << " read calls made" << endl;

    // Print information about the test
    // TODO
    int time = 0;
    int throughput = 0;
    cout << "Test #" << type << ": time = " << time << " usec, #reads = " <<
    nreads << ", throughput " << throughput << " Gbps" << endl;

    // close client socket when done
    close(clientSD);
    return 0;
}