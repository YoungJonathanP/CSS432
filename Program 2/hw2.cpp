/*
 * Starter code provided by Prof. Robert Dimpsey - University of Washington,*
 *      Bothell, CSS422 as described in Lecture 7
 * Code altered by Jonathan Young 4-20-2020
 * Program 2: Sliding Window
 * Purpose: This assignment implements the sliding window algorithm and
 *      evaluates its performance improvement over a 1Gbps network.
 */

#include <iostream>
#include "UdpSocket.h"
#include "Timer.h"

using namespace std;

const int PORT = 51285;       // my UDP port - last 5 SID = 78512, using 51285.
const int MAX = 20000;        // times of message transfer
const int MAX_WIN = 30;       // maximum window size
const bool verbose = true;   //use verbose mode for more information during run

// client packet sending functions
void ClientUnreliable(UdpSocket &sock, int max, int message[]);

int ClientStopWait(UdpSocket &sock, int max, int message[]);

int ClientSlidingWindow(UdpSocket &sock, int max, int message[], int windowSize);

// server packet receiving functions
void ServerUnreliable(UdpSocket &sock, int max, int message[]);

void ServerReliable(UdpSocket &sock, int max, int message[]);

void ServerEarlyRetrans(UdpSocket &sock, int max, int message[], int windowSize);

enum myPartType {
    CLIENT, SERVER
} myPart;

int main(int argc, char *argv[]) {
    int message[MSGSIZE / 4];      // prepare a 1460-byte message: 1460/4 = 365 ints;

    // Parse arguments
    if (argc == 1) {
        myPart = SERVER;
    } else if (argc == 2) {
        myPart = CLIENT;
    } else {
        cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
        return -1;
    }

    // Set up communication
    // Use different initial ports for client server to allow same box testing
    UdpSocket sock(PORT + myPart);
    if (myPart == CLIENT) {
        if (!sock.setDestAddress(argv[1], PORT + SERVER)) {
            cerr << "cannot find the destination IP name: " << argv[1] << endl;
            return -1;
        }
    }

    int testNumber;
    cerr << "Choose a testcase" << endl;
    cerr << "   1: unreliable test" << endl;
    cerr << "   2: stop-and-wait test" << endl;
    cerr << "   3: sliding windows" << endl;
    cerr << "--> ";
    cin >> testNumber;

    if (myPart == CLIENT) {
        Timer timer;
        int retransmits = 0;
        cerr << "Test number " << testNumber << " now running" << endl;
        switch (testNumber) {
            case 1:
                timer.Start();
                ClientUnreliable(sock, MAX, message);
                cerr << "Elapsed time = ";
                cerr << timer.End() << endl;
                break;
            case 2:
                timer.Start();
                retransmits = ClientStopWait(sock, MAX, message);
                cerr << "Elapsed time = ";
                cerr << timer.End() << endl;
                cerr << "retransmits = " << retransmits << endl;
                break;
            case 3:
                for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++) {
                    timer.Start();
                    retransmits = ClientSlidingWindow(sock, MAX, message, windowSize);
                    cerr << "Window size = ";
                    cerr << windowSize << " ";
                    cerr << "Elapsed time = ";
                    cerr << timer.End() << endl;
                    cerr << "retransmits = " << retransmits << endl;
                }
                break;
            default:
                cerr << "no such test case" << endl;
                break;
        }
    }
    if (myPart == SERVER) {
        switch (testNumber) {
            case 1:
                ServerUnreliable(sock, MAX, message);
                break;
            case 2:
                ServerReliable(sock, MAX, message);
                break;
            case 3:
                for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++) {
                    ServerEarlyRetrans(sock, MAX, message, windowSize);
                }
                break;
            default:
                cerr << "no such test case" << endl;
                break;
        }

        // The server should make sure that the last ack has been delivered to client.

        if (testNumber != 1) {
            if (verbose) {
                cerr << "server ending..." << endl;
            }
            for (int i = 0; i < 10; i++) {
                sleep(1);
                int ack = MAX - 1;
                sock.ackTo((char *) &ack, sizeof(ack));
            }
        }
    }
    cerr << "finished" << endl;
    return 0;
}

// Test 1 Client
void ClientUnreliable(UdpSocket &sock, int max, int message[]) {
    // transfer message[] max times; message contains sequences number i
    for (int i = 0; i < max; i++) {
        message[0] = i;
        sock.sendTo((char *) message, MSGSIZE);
        if (verbose) {
            cerr << "message = " << message[0] << endl;
        }
    }
    cerr << max << " messages sent." << endl;
}

// Test1 Server
void ServerUnreliable(UdpSocket &sock, int max, int message[]) {
    // receive message[] max times and do not send ack
    for (int i = 0; i < max; i++) {
        sock.recvFrom((char *) message, MSGSIZE);
        if (verbose) {
            cerr << message[0] << endl;
        }
    }
    cerr << max << " messages received" << endl;
}

/*
 * Helper method for starting connection and timer
 */
void StartSequence(UdpSocket &sock, int *message, Timer *timer) {
    sock.sendTo((char *) message, MSGSIZE); // initiates message sent
    timer->Start(); // starts timeout timer
}

/*
 * Helper method for verifying reception
 */
int &VerifyReceipt(UdpSocket &sock, int &ack, int &sequence) {
    sock.recvFrom((char *) &ack, sizeof(ack)); // checks if message received matches ack value
    return ack == sequence ? ++sequence : sequence; // ternary operator
}

/*
 * Implements Stop-and-Wait algorithm.
 * Repeats sending message[] and receiving an ACK at the client side max
 * times using the sock obj. If the client cannot receive an ACK immediately,
 * it should start a timer and wait 1500µsec. If the wait timeouts, the
 * client should resend the same message. The function must count the number
 * of messages retransmitted and return it to the main function as its return
 * value.
*/
int ClientStopWait(UdpSocket &sock, int max, int message[]) {
    int retransmitted = 0;
    int ack = 0;
    Timer timer;
    // tracks sequencing values
    int sequence = 0;

    // while loop that accounts for all sequence values
    while (sequence < max) {
        // write to message sequence number in message[0]
        message[0] = sequence;
        // Sends message[] to given server and waits until it receives int ACK
        // If ACK is not received immediately, start timer and wait 1500µsec
        StartSequence(sock, message, &timer);
        // UDP does not guarantee every single packet's delivery, once your
        // client is blocked, it may not be resumed. Call pollRecvFrom()
        // before reading the socket.
        while (sock.pollRecvFrom() < 1) {
            // Compare timer for timeout condition
            if (timer.End() >= 1500) {
                StartSequence(sock, message, &timer);
                retransmitted++;
                //cout << "Time exceeded, retransmit count is now : " << retransmitted << endl;
            }
        }
        VerifyReceipt(sock, ack, sequence);
    }
    return retransmitted;
}

/*
 * Implements sliding window algorithm.
 * Repeats sending message[] and receiving an acknowledgment at client side
 * max times using the sock obj. Client can continuously send a new message[]
 * and incrementing its sequence number as long as the number of in-transit
 * message, (i.e., # of unacknowledged messages) is less than windowSize. If
 * the # unacknowledged messages reaches windowSize, the client should start
 * a timer for 1500µsec. If the timer timeouts, it must follow the sliding
 * window algorithm and resend the message with the minimum sequence number
 * among those which have net yet been ACKed. The function must count the
 * number of messages retransmitted and return it to the main function as its
 * return value.
 */
int ClientSlidingWindow(UdpSocket &sock, int max, int message[], int windowSize) {
    int retransmitted = 0;
    int ack = 0;
    int ackSeq = 0;
    int sequence = 0;
    Timer timer;
    while (sequence < max || ackSeq < max) {
        // checks that if we are within the sliding window size
        // does NOT start timer
        if ((ackSeq + windowSize) > sequence && sequence < max) {
            message[0] = sequence;
            sock.sendTo((char *) message, MSGSIZE);
            // if (ack arrived) && (ack is the same as ackSeq) then increment
            // ackSeq increment sequence
            if (sock.pollRecvFrom() > 0) {

                //cerr << "ackSeq was : " << ackSeq << endl;
                sock.recvFrom((char *) &ack, sizeof(ack));
                //cerr << "ack value is: " << ack << endl;
                if (ack == ackSeq) {
                    ackSeq++;
                }
                //ackSeq = VerifyReceipt(sock, ack, ackSeq);
                //cerr << "ackSeq is now: " << ackSeq << endl;
            }
            sequence++;
            //cerr << "sequence value is now " << sequence << endl;
        } else {
            //cerr << "Window is full" << endl;
            // this portion is for when sliding window is full
            if (ackSeq >= max) { // exit condition for if you are at the end
                //cerr << "ackSeq is at max, ackSeq =" << ackSeq << endl;
                break;
            }
            if (timer.End() > 1500) {
                StartSequence(sock, message, &timer);
                retransmitted++;
                //cerr << "Time elapsed, retransmitting" << endl;
            }
            timer.Start();
            while (timer.End() <= 1500 && sock.pollRecvFrom()) {
                sock.recvFrom((char *) &ack, sizeof(ack));
                if (ack >= ackSeq) {
                    ackSeq = ack + 1; // update total ack
                    //cerr << "frame acknowledged, ackSeq incremented" << endl;
                    break;
                } else {
                    // resend message we are waiting for
                    StartSequence(sock, message, &timer);
                    retransmitted++;
                    //cerr << "Last ack did not move window, retransmit count: " << retransmitted << endl;
                }

            }
        }
    }
    return retransmitted;
}

/*
 * Server portion of Stop-and-wait
 * Repeats receiving message[] and sending an acknowledgement at a server
 * side max times using the sock obj.
 */
void ServerReliable(UdpSocket &sock, int max, int message[]) {
    int ack = 0;
    int sequence = 0;
    // while loop that accounts for all sequence numbers
    while (sequence < max) {
        // Receives and sends ACK max times using sock obj
        sock.recvFrom((char *) &ack, sizeof(ack));
        // Check if received message matches current sequence
        // if true, return sequence number and increment sequence
        // if false, continue reception until they match
        if (ack == sequence) {
            sock.ackTo((char *) &sequence, sizeof(sequence));
            sequence++;
        }
    }
    cout << "UDP stop and wait complete" << endl;
}

/*
 * Server portion of Sliding window
 * Repeats receiving message[] and sending an ACK at the server side max
 * times using the sock obj. Every time the server receives a new message[],
 * it must record this message's sequence number in its array and returns a
 * cumulative ACK.
 */
void ServerEarlyRetrans(UdpSocket &sock, int max, int message[], int windowSize) {
    int ack = 0;
    bool array[max];
    int sequence = 0;
    for (int j = 0; j < max; j++) {
        array[j] = false;
    }
    while (sequence < max) {
        sock.recvFrom((char *) message, MSGSIZE);
        if (message[0] == sequence) {
            // sequence found, mark boolean position as true
            array[sequence] = true;
            // loop through to find last true element
            for (int i = max - 1; i >= 0; i--) {
                if (array[i] == true) {
                    ack = i;
                    sequence = i + 1;
                    break;
                }
            }
        } else {
            // marke array at message[0] position as being received
            array[message[0]] = true;
        }
        // cumulative ack for the series of messages received so far
        sock.ackTo((char *) &ack, sizeof(ack));

    }
}
