#pragma once

#include "ConnectionHandler.h"
#include <string>
#include <mutex>
#include <thread>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler* handler;
    int subId;
    int receiptId;
    string userName;
    std::mutex mutex;
    std::thread socketReaderThread;
    int findSpot(string str, string sub);
    int runSocketReaderThread();

public:
    StompProtocol();
    virtual ~StompProtocol();
    StompProtocol(const StompProtocol & other);
    StompProtocol(StompProtocol && other);
    StompProtocol &operator=(const StompProtocol & other);
    StompProtocol &operator=(StompProtocol && other);
    void runClient();
    void openfile(string namefile, string summary);
};
