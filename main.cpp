#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <chrono>
#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <boost/tokenizer.hpp>

// Holds the values for a single entry in the user log file
struct locationEntry {
    double timestamp;
    double navLat;
    double navLon;
    double navAlt;
    double gpsLat;
    double gpsLon;
    double gpsAlt;
};

/*
*   Input: Tokenized string vector of a single line from a csv file
*   Output: A struct populated with each value from the csv file line
*   Description: Takes a tokenized string from a csv files, pulls the values out one by one and puts them into a temporary struct
*                that is returned
*/
locationEntry newEntry(std::vector<std::string> &vec)
{

    locationEntry temp;

    temp.timestamp = stod(vec[0]);
    temp.navLat = stod(vec[1]);
    temp.navLon = stod(vec[2]);
    temp.navAlt = stod(vec[3]);
    temp.gpsLat = stod(vec[4]);
    temp.gpsLon = stod(vec[5]);
    temp.gpsAlt = stod(vec[6]);

    return temp;
}

/*
*   Input: Filename and vector to hold each entry
*   Output: Vector that has been populated with the data from the provided csv files
*   Description: Parses the data from each line of the provided file (assuming it exists) and stores them in a struct vector
*/
void tokenizeLog(std::vector<locationEntry> &data, std::string filename)
{
    using namespace std;
    using namespace boost;

    ifstream in(filename.c_str());

    if (!in.is_open()) return;

    typedef tokenizer< escaped_list_separator<char> > Tokenizer;

    vector< string > vec;
    string line;

    while (getline(in,line))
    {
        Tokenizer tok(line);
        vec.assign(tok.begin(),tok.end());

        // Make sure no data is missing from the line
        if (vec.size() < 7) continue;
        
        // Add a new entry to the struct vector that holds all the values from the current line of the csv file
        data.push_back(newEntry(vec));
    }
}

int64_t keyPressed(sf::Event event)
{
    using namespace std;

    if(event.key.code == sf::Keyboard::Q)
        return 1;
    else if(event.key.code == sf::Keyboard::P)
        return 2;
    else if(event.key.code == sf::Keyboard::R)
        return 3;
    else if(event.key.code == sf::Keyboard::U)
        return 4;

    return 0;
}

void displayLocationData(locationEntry loc){
    //std::cout << "Timestamp: " << loc.timestamp << std::endl;
    std::cout << "Navisense Latitude: " << loc.navLat << " | GPS Latitude: " << loc.gpsLat << std::endl;
    std::cout << "Navisense Longitude: " << loc.navLon << " | GPS Longitude: " << loc.gpsLon << std::endl;
    std::cout << "Navisense Altitude: " << loc.navAlt << " | GPS Altitude: " << loc.gpsAlt << std::endl;
}

void displayLocationData(std::vector<locationEntry> loc){
    std::cout.precision(15);
    std::cout << "\n\n\n\n\n\n\n";
    for(int i = 0; i < loc.size(); i++){
        std::cout << "User " << i + 1 << std::endl;
        if(loc[i].timestamp != NULL){
            std::cout << "Navisense Latitude: " << loc[i].navLat << " | GPS Latitude: " << loc[i].gpsLat << std::endl;
            std::cout << "Navisense Longitude: " << loc[i].navLon << " | GPS Longitude: " << loc[i].gpsLon << std::endl;
            std::cout << "Navisense Altitude: " << loc[i].navAlt << " | GPS Altitude: " << loc[i].gpsAlt << std::endl;
            
        }
    }
}

void parseData(std::vector<std::vector<locationEntry>> data)
{
    bool playPause = false, forwardReverse = true, update = false;
    float speed = 1;

    /*std::vector<locationEntry> next;
    for(int i = 0; i < data.size(); i++)
        next.push_back(data[i][0]);*/

    locationEntry blank;
    blank.timestamp = NULL;

    std::vector<locationEntry>  current(data.size(), blank);
    std::vector<std::vector<locationEntry>::iterator> it(data.size());
    for(int i = 0; i < data.size(); i++)
        it[i] = data[i].begin();
    
    std::cout << it[0][0].timestamp << " " << it[1][0].timestamp << std::endl;
    it[1]++;
    std::cout << it[0][0].timestamp << " " << it[1][0].timestamp << std::endl;
    it[1]--;
    std::cout << it[0][0].timestamp << " " << it[1][0].timestamp << std::endl;
    

    
    //Create stopwatch for each entry in current (each user)
    
    for(int i = 0; i < current.size(); i++)
        std::cout << "User " << i + 1 << std::endl;

    clock_t startTime = clock();
    clock_t sumTime = 0;
    double secondsPassed;
    
    sf::RenderWindow window(sf::VideoMode(1, 1), "Dungeon Generator");
    while (window.isOpen()) {
		sf::Event event;

		// Poll for a key press
		while (window.pollEvent(event)){
			switch (event.type){
                case sf::Event::Closed:
					window.close();
					break;
                case sf::Event::KeyPressed:
                    switch(keyPressed(event)){
                        case 0:
                            std::cout << "Key not recognized" <<std::endl;
                            break;
                        case 1:
                            std::cout << "Quitting..." << std::endl;
                            window.close();
                            break;
                        case 2:
                            if(playPause)
                                std::cout << "Pausing..." << std::endl;

                            else
                                std::cout << "Resuming..." << std::endl;
                            playPause = !playPause;
                            break;
                        case 3:
                            if(forwardReverse)
                                std::cout << "Reversing" << std::endl;
                            else
                                std::cout << "Forwarding" << std::endl;
                            forwardReverse = !forwardReverse;
                            break;
                        case 4:
                            std::cout << "Incrementing speed" << std::endl;
                            if(speed == 2){
                                sumTime += (clock() - startTime)*speed;
                                speed = .5;
                                startTime = clock();
                            }else{
                                sumTime += (clock() - startTime)*speed;
                                speed *= 2;
                                startTime = clock();
                            }
                            break;
                    }
					break;
            }
        }

        // If we are currently playing
        if(playPause)
        {
            // If we are going forward
            if(forwardReverse)
            {
                /*std::cout << "Forward at " << speed << std::endl;
                secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
                for(int i = 0; i < next.size(); i++){
                    if(secondsPassed >= next[i].timestamp){
                        current[i] = next[i];
                    }
                }*/
                secondsPassed = (((clock() - startTime)*speed) + sumTime) / CLOCKS_PER_SEC;
                for(int i = 0; i < current.size(); i++){
                    if(secondsPassed >= it[i][0].timestamp){
                        current[i] = it[i][0];
                        it[i]++;
                        update = true;
                    }
                }
            }else
            {
                std::cout << "Backward at " << speed << std::endl;

            }

            if(update){
                displayLocationData(current);
                update = false;
            }
                
        }
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
}

int main(int argc, char *argv[])
{
    using namespace std;

    // Vector to hold any ammount of user log files
    vector<string>users;

    // Load each argument (csv filename)
    for(int i = 1; i < argc; i++)
        users.push_back(argv[i]);

    // Vector to hold the entries to each users log files
    vector<vector<locationEntry>> data;

    // For each user log file, tokenize its entries and store them in the data vector
    for(int i = 0; i < users.size(); i++)
    {
        vector<locationEntry> temp;
        tokenizeLog(temp, users[i]);
        data.push_back(temp);
    }
    
    // Function to handle moving through data
    parseData(data);
    
    return 0;
}