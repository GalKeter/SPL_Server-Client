#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using std::string;
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{
    team_a_name = findString(frame_body,"team a:");
    team_b_name = findString(frame_body,"team b:");
    name = findString(frame_body,"event name:");
    time = stoi(findString(frame_body,"time:"));
    description = findString(frame_body,"description:");
    //updating the hashMaps
    updateHash("game updates",hashBody(frame_body,"game updates:", "team a updates:"));
    updateHash("team a updates",hashBody(frame_body,"team a updates:", "team b updates:"));
    updateHash("team b updates",hashBody(frame_body,"team b updates:", "description:"));
}

void Event::updateHash(string hashName, string hashBody){
    int i=0;
        while(hashBody[i]!='\0'){
            string key="";
            string value="";
            while(hashBody[i]!=':'){
                key+=hashBody[i];
                i++;
            }i++;
            while(hashBody[i]!='\n'){
                value+=hashBody[i];
                i++;
            }
            i++;
            if (hashName == "game updates"){
                game_updates.insert({key,value});
            }
            else if (hashName == "team a updates"){
                team_a_updates.insert({key,value});
            }
            else if (hashName == "team b updates"){
                team_b_updates.insert({key,value});
            }
        }
}

string Event::hashBody(string str, string hashName, string nextHash){
    int pos1 = str.find(hashName);
    pos1+=hashName.length()+1;
    int pos2 = str.find(nextHash);
    string result = str.substr(pos1, pos2-pos1);
    result +='\0' ;
    return result;
}

string Event::findString(string str, string sub){
    //every sub we will search for will have 1 occurance in the str
    std::size_t pos = str.find(sub);
    string result ="";
    pos += sub.length();
    if (sub == "description:"){
        while (str[pos]!= '\0'){
        result+=str[pos];
        pos++;
        }
    }
    else{
        while (str[pos]!= '\n'){
        result+=str[pos];
        pos++;
        }
    }
    return result;
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}
