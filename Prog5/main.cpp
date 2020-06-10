/*
 * Created by Jonathan Young 5-27-2020
 * University of Washington Bothell CSS432 Prof. Robert Dimpsey
 * Design and code a limited ftp client program that is based on RFC 959
 *
 * The program will be limited to the commands below.
 *
 * The program should be invoked with ftp [hostname]. It connects a session to
 * the remote hostname server using the default port 21 and transfers files
 * in passive mode.
 *
 *  If no argument is given, the client program does not establish any
 *  connection until it receives an open command (See below).
 *
 *  ftp client interface -- the corresponding RFC959 commands -- behavior
 *  open ip port        -- NA --      Establish a TCP connection to ip on port
 *  Name: account  -- USER account -- Send a user identification to the server.
 *  password: user_password -- PASS user_password//SYST -- Send the user password to the server.
 *  cd subdir --CWD subdir-- change the server's current working directory to subdir
 *  ls -- PASV // LIST -- Ask the server to send back its current directory contents through the data connection
 *  get file -- PASV // RETR file -- Get a file from the current remote directory
 *  put file -- PASV // STOR file -- Store a file into the current remote directory
 *  close -- QUIT -- Close the connection but not quit ftp
 *  quit -- 	QUIT (if not closed)    -- Close the connection and quit ftp
 *
 *  PASV requests the server to send back its IP and port on which it listens
 *  to a data TCP connection from the client
 *
 */

#include <iostream>
#include "FTPClient.h"
using namespace std;

// port selected if no other port is specified
//const int DEFAULT_PORT = 21;
const int CMD_MAX = 1000;

/*
 * The expected inputs range from 1-3
 * ./ftp will open the 'ftp>' prompt waiting for a valid open operation
 * ./ftp with 2 arguments 'hostName' will connect to the host on default port
 * ./ftp with 3 arguments 'hostName' and 'port' will set both and connect
 */

int main(int argc, char *argv[]) {
    char *hostName = nullptr;
    int port = DEFAULT_PORT;
    FTPClient client; // create client object
    if (argc > 1){ // expected arguments range 1 - 3
        hostName = argv[1]; // assign host value to second argument
        if (argc == 3){
            port = atoi(argv[2]); // reassign port value (assume valid)
        }
        client.openServer(hostName, port);
    } //else {
        //cerr << "Usage: " << argv[0] << "clientName" << endl; // standard err
    //}

    do {
        char command[CMD_MAX];
        printf("ftp> ");
        cin.getline(command, CMD_MAX);
        //char *userCmd = strtok(command, " ");
        // may receive one or two inputs
        client.userIn(command); // does command need to be cleared?
    } while (client.isActive()); // show ftp prompt while active is true

    return 0;
}