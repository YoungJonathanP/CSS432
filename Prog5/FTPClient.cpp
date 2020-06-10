//
// Created by Jonathan Young on 5/27/2020.
// An FTP program that mimics TELNET features
//

#include "FTPClient.h"


FTPClient::FTPClient() {


}

// deconstructor for closing all sockets
FTPClient::~FTPClient() {
    close(clientFD);
    close(clientSD);
    close(clientFD);

}


// opens connection to specified server
bool FTPClient::openServer(char *hostName, int port) {
    if (openConnection) { // if currently connected send message
        cerr << "Connection currently open, close before opening new" << endl;
        return false;
    }
    char userInput[USER_BUF];
    if (hostName == nullptr) {
        cout << "(to) ";
        cin.getline(userInput, USER_BUF);
        if (!userInput) { // no user entry, return error
            cerr << "usage: open host-name [port]" << endl;
            return false;
        }
        hostName = userInput;
    }
    // calls on socket class assigning a sock from the port
    Socket *sock = new Socket(port);
    clientSD = sock->getClientSocket(hostName);
    if (clientSD > 0) { // true if binding of socket from host
        openConnection = true;
        activityMode = true;
        serverName = hostName;
        cout << "Connected to " << serverName << endl;
    }
    convertCode();
    ftpCode = replyCode; // assign ftpCode for next comparison
    if (ftpCode == 220) { // 220 Service ready for new user.
        struct passwd *pass;
        pass = getpwuid(getuid());
        char userInput[USER_BUF];
        string userString = pass->pw_name;;
        string userName = "USER ";
        string password = "PASS ";
        // output to show connection info
        cout << "Name (" << serverName << ":" << userString << "): ";
        // binds user input from console
        cin.getline(userInput, USER_BUF);
        char *userIn = strtok(userInput, " ");
        if (userIn != nullptr) {
            userString = userIn;
        }
        //Add the user input to the user name
        userName += userString;
        //Attempt to send the username and see if it's valid
        sendCommand(userName, userName.length());
        ftpCode = replyCode; // reassign ftpCode for password reception
        if (ftpCode == 331) { //331 User name okay, need password.
            char *userPass = getpass("Password: ");
            userIn = strtok(userPass, " ");
            if (userIn != nullptr) {
                password += userIn;
            }
            if (sendCommand(password, password.length())) {
                // if true password accepted by server
                serverReply(clientSD);
            }
        }
    }
    ftpCode = replyCode;
    if (ftpCode == 230) { // 230 User logged in, proceed.
        serverReply(clientSD);
        sendCommand("SYST", 4);
        // 215 NAME system type.
        // Where NAME is an official system name from the list in the
        // Assigned Numbers document.
        return true;
    }

    if (ftpCode == 530) { // not logged in
        cout << "Login failed." << endl;
    }

    cout << "No reply received from server";
    closeSock(clientSD);
    return false;
}

// retrieve argument object -- requires target (file) argument
bool FTPClient::get(char *fileName) {
    if (!openConnection) {
        cerr << "Not connected" << endl;
        return false;
    }
    string sendCmd = "RETR ";
    string remotePath;
    char *localPath;
    char userInput[USER_BUF];
    char *userIn;
    if (fileName == nullptr) {
        // retrieval for server file dir
        cout << "(remote-file) ";
        cin.getline(userInput, USER_BUF);
        userIn = strtok(userInput, " , ");
        remotePath = userIn;

        // retrieval for local file dir
        cout << "(local-file) ";
        bzero(userInput, sizeof(userInput));
        cin.getline(userInput, USER_BUF);
        userIn = tokenize(userInput);
        localPath = userIn;
    }
        // User input only has get and remote file, local == remote file name
    else {
        userIn = strtok(fileName, " , ");
        remotePath = userIn;
        userIn = strtok(userIn, " , ");
        localPath = userIn;
    }
    //set passive mode and activate passiveSD
    passive();

    // set type to binary
    string binType = "TYPE I";
    sendCommand(binType, binType.length());

    ftpCode = replyCode;
    // 200 Command okay.
    if (ftpCode != 200) {
        return false;
    }
    sendCmd += remotePath;
    sendCommand(sendCmd, sendCmd.length());

    // 150 File status okay; about to open data connection.
    ftpCode = replyCode;
    if (ftpCode == 150) {
        // receiveFile attempts to use special server permissions
        long fileSize = receiveFile(localPath);
        if (fileSize <= 0) {
            // cannot create file at this location
            cout << "local: " << localPath << ": Permission denied" << endl;
            closeSock(passiveSD);
            close(clientFD);
            return false;
        }

        pid_t childStatus, pid = fork();
        if (pid < 0) {
            printf("Fork failed!\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            //time to get the file size
            fileSize = receiveFile(localPath);

            //Close socket and file descriptors
            closeSock(passiveSD);
            close(clientFD);

            //See the message the server sends and set reply code
            convertCode();
            exit(EXIT_SUCCESS);
        } else {
            wait(&childStatus);
        }
    }
    closeSock(passiveSD);
    return false;
}

// change to argument directory -- requires directory argument
bool FTPClient::cd(const char *dir) {
    if (!openConnection) {
        cerr << "Not connected" << endl;
        return false;
    }
    string cwdCom = "CWD ";
    char userInput[USER_BUF];

    if (dir != nullptr) { // directory argument added to cwd string
        cwdCom += dir;
    } else { // no argument given, prompt for directory
        cout << "(remote-directory) ";
        cin.getline(userInput, USER_BUF);
        if (!userInput) { // no user entry, return error
            cerr << "usage: cd remote-directory" << endl;
            return false;
        }
        cwdCom += userInput;
    }
    sendCommand(cwdCom, cwdCom.length());
    ftpCode = replyCode;
    if (ftpCode == 250) { //250 Requested file action okay, completed.
        return true;
    }

    cerr << "usage: cd remote-directory" << endl;
    return false;
}

// command to list contents of specified server
bool FTPClient::ls() {
    if (!openConnection) {
        cerr << "Not connected" << endl;
        return false;
    }
    passive(); // sends PASV and passivly Connects
    //cout << "passive mode good" << endl;
    pid_t pid = fork(); // forks a child
    pid_t childStatus = pid;
    if (pid < 0) { // fork must be == 0 to be a child
        cerr << "Fork Fail!" << endl;
        closeSock(passiveSD);
        return false;
    } else if (pid == 0) {
        //cout << "pid is a child" << endl;
        serverReply(passiveSD); // expect this call to be blocked
        closeSock(passiveSD);
        exit(EXIT_SUCCESS);
    } else {
        if (sendCommand("LIST", 4)) {
            wait(&childStatus);
            convertCode();
        }

        return true;
    }
}


// place a file -- requires target (file)
bool FTPClient::put(char *fileName) {
    if (!openConnection) {
        cerr << "Not connected" << endl;
        return false;
    }
    string sendCmd = "STOR ";
    string remotePath;
    char *localPath;
    char userInput[USER_BUF];
    char *userIn;

    if (fileName == nullptr) {
        // retrieval for server file dir
        cout << "(remote-file) ";
        cin.getline(userInput, USER_BUF);
        userIn = strtok(userInput, " , ");
        remotePath = userIn;

        // retrieval for local file dir
        cout << "(local-file) ";
        bzero(userInput, sizeof(userInput));
        cin.getline(userInput, USER_BUF);
        userIn = tokenize(userInput);
        localPath = userIn;
    }
        // User input only has get and remote file, local == remote file name
    else {
        userIn = strtok(fileName, " , ");
        remotePath = userIn;
        userIn = strtok(userIn, " , ");
        localPath = userIn;
    }

    // set read only permission on file descriptor and check path
    if (clientFD = open(localPath, O_RDONLY) < 0) {
        cout << "local: " << localPath << ": No such file or directory" << endl;
        return false;
    }
    //set passive mode and activate passiveSD
    passive();

    // set type to binary
    string binType = "TYPE I";
    sendCommand(binType, binType.length());

    ftpCode = replyCode;
    // 200 Command okay.
    if (ftpCode != 200) {
        return false;
    }
    //Remote path is appended to sent command
    sendCmd += remotePath;
    sendCommand(sendCmd, sendCmd.length());

    long fileSize = 0;

    // 150 File status okay; about to open data connection.
    ftpCode = replyCode;
    if (ftpCode == 150) {
        pid_t childStatus, pid = fork();
        if (pid < 0) {
            printf("Fork failed!\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            //time to get the file size
            fileSize = receiveFile(localPath);

            //Close socket and file descriptors
            closeSock(passiveSD);
            close(clientFD);

            //See the message the server sends and set reply code
            convertCode();
            exit(EXIT_SUCCESS);
        } else {
            wait(&childStatus);
        }
    }
    closeSock(passiveSD);
    return false;
}


// used with the 'get' command
int FTPClient::receiveFile(char *filename) {

    // code provided from FAQ Q 6
    //the client should set these file modes before writing a file received
    // from the server
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    clientFD = open(filename, O_WRONLY | O_CREAT, mode);

    return clientFD;
}

// close current ftp connection
void FTPClient::closeConnect() {
    if (!openConnection) {
        cerr << "No active connection!" << endl;
        return;
    }
    sendCommand("QUIT", 4);

    close(clientSD);

}

// exits out of FTP mode
void FTPClient::quit() {
    if (isActive()) {
        closeConnect();
    }
    activityMode = false;
}

// allows for return to main for loop
bool FTPClient::isActive() {
    return activityMode;
}


// reads message from server
void FTPClient::serverReply(int socketSD) {
    struct pollfd ufds;

    ufds.fd = socketSD;              // a socket descriptor to examine for read
    ufds.events = POLLIN;            // check if this sd is ready to read
    ufds.revents = 0;                // simply zero-initialized
    // poll this socket for 1000msec (=1sec)
    //int val = poll( &ufds, 1, 1000 );
    serverMsg.clear();
    int fileSize = 0;
    int nread = 0;
    //if ( val > 0 ) {                  // the socket is ready to read
    char buf[BUFLEN];
    // guaranteed to return from read
    //int nread = read(socketSD, buf, BUFLEN);
    // even if nread < BUFLEN
    while (poll(&ufds, 1, 1000) > 0) {
        bzero(buf, sizeof(buf));
        nread = read(socketSD, buf, BUFLEN);
        if (nread == 0) {
            //cerr << "While poll- Read from server was 0" << endl;
            break;
            return;
        }
        // add to message
        serverMsg.append(buf);
        fileSize += nread;
    }
    if (fileSize > 0) {
        cout << serverMsg;
    }
}

// function for reading user commands and calling appropriate function
bool FTPClient::userIn(char *command) {

    char *userCmd;
    // assigns userCmd char pointer to first token in command
    userCmd = strtok(command, " , (_)-");
    //userCmd = tokenize(command);

    if (strncmp(userCmd, "open", 4) == 0) {
        // this checks to see if server argument is supplied
        userCmd = tokenize(userCmd);
        if (userCmd != nullptr) {
            // checks to see if port argument is supplied
            char *portCh = tokenize(userCmd);
            port = DEFAULT_PORT;
            if (portCh != nullptr) {
                port = atoi(portCh);
            }
            cout << userCmd << endl;
            return openServer(userCmd, port);
        }

    } else if (strncmp(userCmd, "cd", 2) == 0) {
        userCmd = tokenize(userCmd);
        return cd(userCmd);

    } else if (strncmp(userCmd, "ls", 2) == 0) {
        return ls();

    } else if (strncmp(userCmd, "get", 3) == 0) {
        userCmd = tokenize(userCmd);
        return get(userCmd);

    } else if (strncmp(userCmd, "put", 3) == 0) {
        userCmd = tokenize(userCmd);
        return put(userCmd);

    } else if (strncmp(userCmd, "close", 5) == 0) {
        closeConnect();
        return true;

    } else if (strncmp(userCmd, "quit", 4) == 0) {
        quit();
        return true;
    }
    cout << "No matching command found" << endl;
    return false;

}

// creates passive connection to server
void FTPClient::passive() {
    if (sendCommand("PASV", 4)) {
        ftpCode = replyCode;
    }
    char *passV;
    int p1;
    int p2;
    int passPort;

    if (ftpCode == 227) { //227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
        //cout << "ftpcode is " << ftpCode << endl;
        //cout << serverMsg << endl;
        char *temp = const_cast<char *>(serverMsg.c_str());
        passV = strtok(temp, " ,");
        // loop through and assign p1 and p2 to end values
        for (int i = 0; i < 5; i++) {
            p1 = atoi(passV);
            passV = tokenize(passV);
            p2 = atoi(passV);
            passV = tokenize(passV);
        }


        // p1 and p2 are part of the formula for the new port
        passPort = (p1 * PORT_MULT) + p2;
        bzero(passV, sizeof(passV));
        passV = const_cast<char *>(serverName.c_str());
        auto *passSock = new Socket(passPort);
        passiveSD = passSock->getClientSocket(passV);
    }
}

// converts server reply message to int ftpCode, returns true if success
void FTPClient::convertCode() {

    char buf[BUFLEN];
    bzero(buf, sizeof(buf));
    //cout << "attempting to convert from " << clientSD << endl;
    int nread = read(clientSD, buf, sizeof(buf));
    //printf("read value is %d \n", nread);

    if (nread > 0) {
        serverMsg.clear();
        serverMsg.append(buf);
        cout << buf;
        char *ftpVal = strtok(buf, " ,/'!-()@");
        replyCode = atoi(ftpVal);
        //cout << "The ftpVal was " <<  ftpVal << endl;
    }
}


// sends a message of intent to the server
bool FTPClient::sendCommand(const string &cmd, int mSize) {
    mSize += 2;
    char cmdBuf[mSize];
    bzero(cmdBuf, sizeof(cmdBuf));
    strcpy(cmdBuf, cmd.c_str()); // Copy cmd into cmdBuf
    //cout << "copy success " << cmdBuf << endl;
    strcat(cmdBuf, "\r\n"); // add carriage return and new line to message
    //cout << "line carriage added" << endl;
    int nwrite = write(clientSD, cmdBuf, mSize);
    //cout << "nwrite size is " << nwrite << endl;
    if (nwrite > 0) {
        convertCode();
        return true;
    }
    cerr << "Error sending command to server" << endl;
    return false;
}

// helper method to split up and handle individual char tokens
char *FTPClient::tokenize(const char *input) {
    //cout << "tokenizing " << input << endl;
    return input != nullptr ? strtok(nullptr, " ,(_)") : nullptr;
}

// helper method to close specified sockets
void FTPClient::closeSock(int sockSD) {
    shutdown(sockSD, SHUT_WR);
    close(sockSD);
    if (sockSD == clientSD) {
        openConnection = false;
    }
}







