#include <string>
#include <map>
#include <vector>
#include "../include/event.h"
using namespace std;
using std::string;
#pragma once

class Channel
{
    struct Comparator
    {
        bool operator()(const std::pair<bool, int> &lhs, const std::pair<bool, int> &rhs) const
        {
            if (lhs.first == rhs.first)
            { // returns true if lhs is "smaller" and false otherwise
                return lhs.second < rhs.second;
            }
            else
            {
                return lhs.first > rhs.first;
            }
        }
    };

private:
    string gameName;
    map<string, map<string, string>> userToGeneralStats;
    map<string, map<string, string>> userToTeam_A_Stats;
    map<string, map<string, string>> userToTeam_B_Stats;
    map<string, map<std::pair<bool, int>, vector<string>, Comparator>> userGameReports;

public:
    Channel();
    Channel(string &_gameName);
    void updateEvent(string &user, Event &event);
    string getName();
    string getGeneralStats(string &user);
    string getTeam_a_stats(string &user);
    string getTeam_b_stats(string &user);
    string getGameReports(string &user);
};


