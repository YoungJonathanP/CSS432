//
// Created by Jonathan Young on 5/27/2020.
// An FTP program that mimics TELNET features
//

#ifndef CSS432_FTPCLIENT_H
#define CSS432_FTPCLIENT_H

#include "Socket.h"
#include <iostream>
#include <sys/poll.h> // see Q1
#include <sys/types.h> // see Q6
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // see Q5
#include <pwd.h>
#include <cstdio>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/wait.h>

#include <sys/uio.h>

using namespace std;

const int DEFAULT_PORT = 21;

//string serverStream;

class FTPClient {

public:
    FTPClient(); // default constructor
    ~FTPClient();


    bool openServer(char *hostName, int port);

    // retrieve argument object -- requires target (file) argument
    bool get(char *fileName);

    // change to argument directory -- requires directory argument
    bool cd(const char *dir);

    bool ls(); // lists current directories content -- no argument required
    bool put(char *fileName); // place a file -- requires target (file)
    int receiveFile(char *filename); // from Q6
    void closeConnect(); // close server connection
    void quit(); // quit ftp client
    bool isActive(); // returns activity state -- user input only on true
    bool userIn(char *command); // sends user input to determine action
    void closeSock(int sockSD); // close current socket
    void serverReply(int socketSD); // interprets server send message
    // converts server reply message to int ftpCode, returns true if success
    void convertCode();

    // sends a message of intent to the server -- always calls convertCode()
    bool sendCommand(const string &cmd, int mSize);
    //bool createFile(char *file);

    // reduces inputs into tokens const pointer to next value
    char *tokenize(const char *input);


private:
    void passive(); // creates passive connection to server
    bool activityMode = false; // allows for return to main for loop
    bool openConnection = false; // variable for connection status
    int clientFD;
    int clientSD;
    int passiveSD;
    int port;
    int ftpCode;
    int replyCode;
    string serverName;
    string serverMsg;

    const int USER_BUF = 512; // value for char input arrays
    const int BUFLEN = 8192; // value for reply char reply arrays
    // for calculating port value
    const int PORT_MULT = 256;


};


#endif //CSS432_FTPCLIENT_H
