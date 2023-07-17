#include "../include/Channel.h"
using std::string;

Channel::Channel() : gameName(), userToGeneralStats(), userToTeam_A_Stats(), userToTeam_B_Stats(), userGameReports(){}

Channel::Channel(string& _gameName) : gameName(_gameName), userToGeneralStats(), userToTeam_A_Stats(), userToTeam_B_Stats(), userGameReports(){}


void Channel::updateEvent(string& user, Event& event){ 
    map<string,string> gameUpdates = event.get_game_updates();
    map<string,string> team_a_updates = event.get_team_a_updates();
    map<string,string> team_b_updates = event.get_team_b_updates();
    for (const auto& pair : gameUpdates) {
        userToGeneralStats[user][pair.first] = pair.second;
    }
    for (const auto& pair : team_a_updates) {
        userToTeam_A_Stats[user][pair.first] = pair.second;
    }
    for (const auto& pair : team_b_updates) {
        userToTeam_B_Stats[user][pair.first] = pair.second;
    }
    if(userToGeneralStats[user]["before halftime"]=="true")
        userGameReports[user][{true,event.get_time()}] = {event.get_name(),event.get_discription()};
    else
        userGameReports[user][{false,event.get_time()}] = {event.get_name(),event.get_discription()};
}


string Channel::getName(){
    return gameName;
}

string Channel::getGeneralStats(string& user){
    string ans = "";
    for (const auto& pair : userToGeneralStats[user]) {
        ans += pair.first + ": " + pair.second +"\n"; 
    }
    return ans;
}

string Channel::getTeam_a_stats(string& user){
 string ans = "";
    for (const auto& pair : userToTeam_A_Stats[user]) {
        ans += pair.first + ": " + pair.second +"\n"; 
    }
    return ans;
}

string Channel::getTeam_b_stats(string& user){
 string ans = "";
    for (const auto& pair : userToTeam_B_Stats[user]) {
        ans += pair.first + ": " + pair.second +"\n"; 
    }
    return ans;
}

string Channel::getGameReports(string& user){
 string ans = "";
    for (const auto& pair : userGameReports[user]) {
        ans += std::to_string(pair.first.second) + " - " + pair.second.at(0) + ":\n" + "\n"; 
        ans += pair.second.at(1) + "\n" + "\n" + "\n";
    }
    return ans;
}



