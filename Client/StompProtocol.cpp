#include "../include/StompProtocol.h"
#include <fstream>
using namespace std;

StompProtocol::StompProtocol() : subId(0), receiptId(0), userName(""), mutex(), socketReaderThread(), handler(nullptr) {}

StompProtocol::StompProtocol(const StompProtocol & other): subId(other.subId), receiptId(other.receiptId), userName(other.userName), mutex(), socketReaderThread(), handler(nullptr) {}

StompProtocol::StompProtocol(StompProtocol && other): subId(other.subId), receiptId(other.receiptId), userName(other.userName), mutex(), socketReaderThread(), handler(nullptr) {}

StompProtocol &StompProtocol::operator=(const StompProtocol & other){
    if (this != &other){
        subId = other.subId;
        receiptId = other.receiptId;
        userName = other.userName;
    }
    return *this;
}
StompProtocol &StompProtocol::operator=(StompProtocol && other){
    subId = other.subId;
    receiptId = other.receiptId;
    userName = other.userName;
    return *this;
}

void StompProtocol::runClient()
{
    while (true)
    {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        string input(buf);
        std::vector<string> parts;
        string delimiter = " ";
        size_t pos = 0;
        while ((pos = input.find(delimiter)) != string::npos)
        {
            parts.push_back(input.substr(0, pos));
            input.erase(0, pos + delimiter.length());
        }
        parts.push_back(input);
        string command = parts[0];
        if (command == "login")
        {
            if (handler != nullptr)
            {
                std::cerr << "The client is already logged in, log out before trying again" << std::endl;
                continue; 
            }
            string host_port = parts[1];
            int pos = host_port.find(":");
            string host = host_port.substr(0, pos);
            short port = stoi(host_port.substr(pos + 1));
            handler = new ConnectionHandler(host, port);
            if (!handler->connect())
            {
                std::cerr << "Could not connect to server" << std::endl;
                continue; 
            }
            else
            {
                if (socketReaderThread.joinable()) //check if socketReader thread finished
                {
                    socketReaderThread.join();
                }
                socketReaderThread = std::thread(&StompProtocol::runSocketReaderThread, this);
                userName = parts[2];
                string connectFrame = "CONNECT\n";
                connectFrame += "accept-version:1.2\n";
                connectFrame += "host:" + host + "\n";
                connectFrame += "login:" + parts[2] + "\n";
                connectFrame += "passcode:" + parts[3] + "\n" + "\n";
                if (!handler->sendLine(connectFrame))
                {
                    std::cerr << "Could not send message to server" << std::endl;
                    continue; 
                }
            }
        }
        else if (command == "join")
        {
            subId++;
            receiptId++;
            string gameName = parts[1];
            string subscribeFrame = "SUBSCRIBE\n";
            subscribeFrame += "destination:/" + gameName + "\n";
            subscribeFrame += "id:" + std::to_string(subId) + "\n";
            subscribeFrame += "receipt:" + std::to_string(receiptId) + "\n" + "\n";
            if (!handler->sendLine(subscribeFrame))
            {
                std::cerr << "Could not send message to server" << std::endl;
                continue; 
            }
            else
            {
                mutex.lock();
                handler->receiptIdToScreenPrint[receiptId] = "Joined channel " + gameName;
                mutex.unlock();
                handler->channelToSubId[gameName] = subId;
            }
        }
        else if (command == "exit")
        {
            receiptId++;
            string gameName = parts[1];
            int subId = handler->channelToSubId[gameName];
            string unsubscribeFrame = "UNSUBSCRIBE\n";
            unsubscribeFrame += "id:" + std::to_string(subId) + "\n";
            unsubscribeFrame += "receipt:" + std::to_string(receiptId) + "\n" + "\n";
            if (!handler->sendLine(unsubscribeFrame))
            {
                std::cerr << "Could not send message to server" << std::endl;
                continue; 
            }
            else
            {
                mutex.lock();
                handler->receiptIdToScreenPrint[receiptId] = "Exited channel " + gameName;
                mutex.unlock();
                if(handler->channelToSubId.count(gameName)>0){
                    handler->channelToSubId.erase(gameName);
                }
            }
        }

        else if (command == "logout")
        {
            receiptId++;
            string disconnectFrame = "DISCONNECT\n";
            disconnectFrame += "receipt:" + std::to_string(receiptId) + "\n" + "\n";
            if (!handler->sendLine(disconnectFrame))
            {
                std::cerr << "Could not send message to server" << std::endl;
                continue; 
            }
            else
            {
                mutex.lock();
                handler->receiptIdToScreenPrint[receiptId] = "disconnect";
                mutex.unlock();
            }
        }

        else if (command == "report")
        {
            string fileName = parts[1];
            names_and_events nne = parseEventsFile(fileName);
            string gameName = nne.team_a_name + "_" + nne.team_b_name;
            string destination = "/" + nne.team_a_name + "_" + nne.team_b_name;
            if (handler->channelToSubId.count(gameName) == 0)
            {
                std::cerr << "You are not subscribed to this channel" << std::endl;
            }
            else
            {
                vector<Event> events = nne.events;
                for (const auto &event : events)
                {
                    string sendFrame = "SEND\n";
                    sendFrame += "destination:" + destination + "\n" + "\n";
                    ;
                    sendFrame += "user:" + userName + "\n";
                    sendFrame += "team a:" + nne.team_a_name + "\n";
                    sendFrame += "team b:" + nne.team_b_name + "\n";
                    sendFrame += "event name:" + event.get_name() + "\n";
                    sendFrame += "time:" + std::to_string(event.get_time()) + "\n";
                    sendFrame += "general game updates:\n";
                    map<string, string> gameUpdates = event.get_game_updates();
                    for (const auto &key : gameUpdates)
                    {
                        sendFrame += key.first + ":" + key.second + "\n";
                    }
                    sendFrame += "team a updates:\n";
                    map<string, string> team_a_updates = event.get_team_a_updates();
                    for (const auto &key : team_a_updates)
                    {
                        sendFrame += key.first + ":" + key.second + "\n";
                    }
                    sendFrame += "team b updates:\n";
                    map<string, string> team_b_updates = event.get_team_b_updates();
                    for (const auto &key : team_b_updates)
                    {
                        sendFrame += key.first + ":" + key.second + "\n";
                    }
                    sendFrame += "description:\n" + event.get_discription();
                    if (!handler->sendLine(sendFrame))
                    {
                        std::cerr << "Could not send message to server" << std::endl;
                        continue; 
                    }
                }
            }
        }

        else if (command == "summary")
        {
            string gameName = parts[1];
            string user = parts[2];
            string file = parts[3];
            mutex.lock();
            Channel channel = handler->gameNameToChannel[gameName];
            mutex.unlock();
            int i = gameName.find("_");
            string team_a = gameName.substr(0, i);
            string team_b = gameName.substr(i + 1, gameName.length() - i - 1);
            string summary = team_a + " vs " + team_b + "\n";
            summary += "Game stats:\n";
            summary += "General stats:\n";
            summary += channel.getGeneralStats(user);
            summary += team_a + " stats:\n";
            summary += channel.getTeam_a_stats(user);
            summary += team_b + " stats:\n";
            summary += channel.getTeam_b_stats(user);
            summary += "Game event reports:\n";
            summary += channel.getGameReports(user);
            openfile(file, summary);
        }
    }
}

int StompProtocol::runSocketReaderThread()
{
    while (true)
    {
        string answer;
        if (!handler->getLine(answer))
        {
            std::cout << "Disconnected. Exiting...\n"
                      << std::endl;
            return 0; 
        }
        string frameType;
        int i = 0;
        while (answer[i] != '\n')
        {
            i++;
        }
        frameType = answer.substr(0, i);
        if (frameType == "CONNECTED")
        {
            std::cout << "login successful" << std::endl;
        }
        else if (frameType == "MESSAGE")
        {
            // find destination (channel)
            int desIndex = findSpot(answer, "destination:/");
            desIndex += 13; 
            string gameName = "";
            while (answer[desIndex] != '\n')
            {
                gameName += answer[desIndex];
                desIndex++;
            }
            // find user
            int userIndex = findSpot(answer, "user:");
            userIndex += 5; 
            string user = "";
            while (answer[userIndex] != '\n')
            {
                user += answer[userIndex];
                userIndex++;
            }
            // find frame's body
            int j = 0;
            while (!(answer[j] == '\n' && answer[j + 1] == '\n'))
            {
                j++;
            }
            j += 2;
            string body = "";
            while (answer[j] != '\0')
            {
                body += answer[j];
                j++;
            }
            size_t count = handler->gameNameToChannel.count(gameName);
            if (count == 0)
            {
                mutex.lock();
                handler->gameNameToChannel[gameName] = Channel(gameName);
                mutex.unlock();
            }
            Event event = Event(body);
            mutex.lock();
            handler->gameNameToChannel[gameName].updateEvent(user, event);
            mutex.unlock();
        }
        else if (frameType == "RECEIPT")
        {
            int i = findSpot(answer, ":");
            i++;
            string receiptStr = "";
            while (answer[i] != '\n')
            {
                receiptStr += answer[i];
                i++;
            }
            int receipt = stoi(receiptStr);
            mutex.lock();
            if (handler->receiptIdToScreenPrint[receipt] == "disconnect")
            {
                std::cerr << "Disconnecting user. Exiting...\n"
                          << std::endl;

                delete handler;
                handler = nullptr;
                mutex.unlock();
                return 0;
            }
            else
            {
                mutex.unlock();
                std::cerr << handler->receiptIdToScreenPrint[receipt] << std::endl;
                mutex.unlock();
            }
        }
        else if (frameType == "ERROR")
        {
            int messagePlace = findSpot(answer, "message:");
            int i = messagePlace + 8; // the length of the string "message:"
            string errorStr = "";
            while (answer[i] != '\n')
            {
                errorStr += answer[i];
                i++;
            }
            std::cerr << errorStr + ". disconnecting user..." << std::endl;
            mutex.lock();
            delete handler;
            handler = nullptr;
            mutex.unlock();
            return 0;
        }
    }
}

int StompProtocol::findSpot(string str, string sub)
{
    std::size_t pos = str.find(sub);
    if (pos != std::string::npos)
    {
        return pos;
    }
    else
    {
        return -1;
    }
}

void StompProtocol::openfile(string namefile, string summary)
{
    std::ofstream outfile; // write to the file
    outfile.open(namefile, ofstream::out | ofstream::trunc);
    if (outfile.is_open())
    {
        outfile << summary;
        outfile.close();
    } // if the file is already opened, writing over its content
}

StompProtocol ::~StompProtocol()
{
    if (handler != nullptr)
        delete handler;
};
