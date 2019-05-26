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
#include <math.h>
#include <cmath>

#include "map.h"
#include "data.h"

#include <osmium/osm/types.hpp>
#include <osmium/io/xml_input.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <boost/tokenizer.hpp>

void outputJson(std::vector<building> &buildings, std::string filename){

    std::ofstream myFile;
    
    // Every entry is a new building which has a set of coordinates that outline it
    std::vector<featurePolygon> featureCollection;

   for(auto& building : buildings)
   {
       if(building.nodeLocations.size() > 2)
       {
           featurePolygon newFeature;
           newFeature.type = "\"Polygon\"";
           newFeature.entered = building.entered;
           for(auto& node : building.nodeLocations)
           {
                // Coordinate for this node
                std::string coord = "[" + std::to_string(node.lon()) + ", " + std::to_string(node.lat()) + "]";

                newFeature.coordinates.push_back(coord);
           }
           featureCollection.push_back(newFeature);
       }
   }

/*
 *  Syntax for geojson file is as follows
 * {
 *  "type": "FeatureCollection",
 *  "features": [
 *      {
 *          "type": "Feature",
 *          "geometry": {
 *              "type": "Polygon",
 *              "coordinates": [
 *                  [[lon1, lat1], [lon2, lat2]...]
 *              ]
 *          },
 *          "properties": {
 *              "stroke": "#color"
 *          }
 *      }, ...
 *  ]
 * }
*/

   myFile.open(filename);
   myFile << "{\"type\": \"FeatureCollection\", \"features\": [";
   for(int i = 0; i < featureCollection.size(); i++)
   {
        myFile << "{\"type\": \"Feature\", \"geometry\": { \"type\": ";
        myFile << featureCollection[i].type + ", \"coordinates\":  [[";
        for(int j = 0; j < featureCollection[i].coordinates.size(); j++){
            myFile << featureCollection[i].coordinates[j];
            if( j != featureCollection[i].coordinates.size() - 1)
                myFile << ", ";
        }
        myFile << "]] }, \"properties\": {";
        myFile << "\"stroke\":\" ";
        if(featureCollection[i].entered)
            myFile << "#16e333";
        else
            myFile << "#449186";
        myFile << "\" ";
        myFile << "} }";
        if(i != featureCollection.size() - 1)
            myFile << ",";
   }
   myFile << "] }";
   myFile.close();
   
}

void outputJson(std::vector<locationEntry> &user, std::string filename)
{
    std::ofstream myFile;
    std::cout << "Outputting user location data to " << filename << std::endl;
    std::vector<featureLineString> featureCollection;

    featureLineString newFeature1, newFeature2;
    newFeature1.type = "\"LineString\"";
    newFeature2.type = "\"LineString\"";
    newFeature1.user = "nav";
    newFeature2.user = "gps";
    for(auto& location : user)
    {
        std::string navCoord = "[" + std::to_string(location.navLon) + ", " + std::to_string(location.navLat) + "]";
        std::string gpsCoord = "[" + std::to_string(location.gpsLon) + ", " + std::to_string(location.gpsLat) + "]";
        newFeature1.coordinates.push_back(navCoord);
        newFeature2.coordinates.push_back(gpsCoord);
    }

    featureCollection.push_back(newFeature1);
    featureCollection.push_back(newFeature2);

    myFile.open(filename);
    myFile << "{\"type\": \"FeatureCollection\", \"features\": [";
    for(int i = 0; i < featureCollection.size(); i++)
    {
        myFile << "{\"type\": \"Feature\", \"geometry\": { \"type\": ";
        myFile << featureCollection[i].type + ", \"coordinates\":  [";
        for(int j = 0; j < featureCollection[i].coordinates.size(); j++){
                myFile << featureCollection[i].coordinates[j];
                if( j != featureCollection[i].coordinates.size() - 1)
                    myFile << ", ";
        }
        myFile << "] }, \"properties\": {";
        myFile << "\"stroke\":\" ";
        if(i == 0)
            myFile << "#d11414";
        else
            myFile << "#1423d1";
        myFile << "\"";
        myFile << "} }";
        if(i != featureCollection.size() - 1)
            myFile << ",";
    }
    myFile << "] }";
   myFile.close();
}

/*
*   Input: locationEntry vector and name of osm file
*   Output: Map object that contains the id of every node that exists in the same osm tile as one of the coordinates in data
*   Description: createMap creates a map object that can be queried with a location to return the id of every node that exists
*                within the same osm tile
*/
Map createMap(std::vector<std::vector<locationEntry>> &data, std::string osmFile){

    Map map(data, osmFile);

    return map;
}

/*
*   Input: Tokenized string vector of a single line from a csv file
*   Output: A struct populated with each value from the csv file line
*   Description: Takes a tokenized string from a csv files, pulls the values out one by one and puts them into a temporary struct
*                that is returned
*/
locationEntry newEntry(std::vector<std::string> &vec)
{
    // Temporary object to hold the extracted values
    locationEntry temp;

    temp.timestamp = stod(vec[0]);
    temp.navLat = stod(vec[1]);
    temp.navLon = stod(vec[2]);
    temp.navAlt = stod(vec[3]);
    temp.gpsLat = stod(vec[4]);
    temp.gpsLon = stod(vec[5]);
    temp.gpsAlt = stod(vec[6]);

    // Return the temp object so that it can be added to the main vector
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

/*
*   Input: Key press event
*   Output: Number denoting which button was pressed
*   Description: Informs data parser if an action needs to be taken based on the last key that was pressed
*/
int keyPressed(sf::Event event)
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

/*
*   Input: locationEntry object
*   Output: Values stored inside object
*   Description: Prints out all the values in a single locationEntry object
*/
void displayLocationData(locationEntry loc)
{
    std::cout << "Timestamp: " << loc.timestamp << std::endl;
    std::cout << "Navisense Latitude: " << loc.navLat << " | GPS Latitude: " << loc.gpsLat << std::endl;
    std::cout << "Navisense Longitude: " << loc.navLon << " | GPS Longitude: " << loc.gpsLon << std::endl;
    std::cout << "Navisense Altitude: " << loc.navAlt << " | GPS Altitude: " << loc.gpsAlt << std::endl;
    
}

/*
* Input: two pairs of latitudes/longitudes
* Output: Difference between the two points
* Description: Calculates the angle between two points on a plane
*/
double angle2D(double lat1, double lon1, double lat2, double lon2)
{
    double theta1 = atan2(lat1, lon1);
    double theta2 = atan2(lat2, lon2);
    double dtheta = theta2 - theta1;
    while(dtheta > M_PI)
        dtheta -= (M_PI * 2);
    while(dtheta <  -M_PI)
        dtheta += (M_PI * 2);
    return(dtheta);
}

/*
* Input: Building vecotr, user latitude, user longitude
* Output: none:
* Checks each building for whether or not it contains the users location and if it does, it marks that building
*/
void pointWithinBuilding(std::vector<building> &buildings, double lat, double lon)
{
    int counter = 0;
    for(auto& building : buildings)
    {
        if(building.nodeLocations.size() < 3 || building.entered == true)
            continue;
        
        int i;
        double angle=0;
        int n = building.nodeLocations.size();
        double p1lat, p1lon;
        double p2lat, p2lon;

        for (i=0;i<n;i++) {
            p1lat = building.nodeLocations[i].lat() - lat;
            p1lon = building.nodeLocations[i].lon() - lon;
            p2lat = building.nodeLocations[(i+1)%n].lat() - lat;
            p2lon = building.nodeLocations[(i+1)%n].lon() - lon;
            angle += angle2D(p1lat,p1lon,p2lat,p2lon);
        }
        if (abs(angle) >= M_PI)
            building.entered = true;    
    }
}

/*
* Input: Map object, User locaitons, unique user identifier
* Output: A GeoJSON file containing the buildings the user encounters as well as which ones are entered
* Description: Takes in a users location data, their user number, and the buildings in order to calculate which buildings this specific user enters
*/
void getOccupiedBuildings(Map &map, std::vector<locationEntry> &user, int usernum)
{
    //Get a list of every building on the map
    std::vector<building> buildings = map.getBuildings();
    std::cout << "Checking " << buildings.size() << " buildings against " << user.size() << " locations for user " << usernum << std::endl;

    for(auto& location : user)
    {
        pointWithinBuilding(buildings, location.navLat, location.navLon);
    }

    std::string filename = "user";
    filename.append(std::to_string(usernum));
    filename.append("buildings.geojson");
    outputJson(buildings, filename);
}

/*
*   Input: locationEntry object vector and Map object
*   Output: Neatly formatted values for each object in the vector including a list of node ids nearby
*   Description: Used to print out the values of every locationEntry object in a vector
*/
void displayLocationData(std::vector<locationEntry> &loc, Map &map)
{
    std::cout.precision(10);

    for(int i = 0; i < loc.size(); i++)
    {
        osmium::Location userLocation(loc[i].navLon, loc[i].navLat);
        
        osmium::geom::Tile userTile(ZOOM, userLocation);
        
        // The id of every node within the same tile as the user
        //std::vector<int> nodeIds = map.getIds(userLocation);

        // Information on every building within the same tile as the user
        std::vector<building> buildings = map.getBuildings(userLocation);
        
        // Information on every highway that the user will encounter
        std::vector<highway> highways = map.getHighways(userLocation);
        
        if(loc[i].timestamp != -1)
        {
            std::cout << "Timestamp: " << loc[i].timestamp << std::endl;
            std::cout << "Navisense Latitude: " << loc[i].navLat << " | GPS Latitude: " << loc[i].gpsLat << std::endl;
            std::cout << "Navisense Longitude: " << loc[i].navLon << " | GPS Longitude: " << loc[i].gpsLon << std::endl;
            std::cout << "Navisense Altitude: " << loc[i].navAlt << " | GPS Altitude: " << loc[i].gpsAlt << std::endl;
            
            std::cout << "Buildings: " << buildings.size() << " | " << "Highways: " << highways.size() << std::endl; 
            std::cout << "Current tile: " << userTile.x << ", " << userTile.y << std::endl; 
            
        }
    }
}

/*
*   Input: Tokenized data from provided csv files
*   Output: Continuous real time data stream of Latitude/Longitude/Altitude values from both the GPS and Navisens
*   Description: Once the csv files have been tokenized this function prints out the data values in real time 
*                allowing for speed and directional changes 
*/
void parseData(std::vector<std::vector<locationEntry>> &data, Map &map)
{
    std::cout << "Starting data stream" << std::endl;
    bool play = false;      // Whether or not the data stream is currently being ran
    bool forward = true;    // Which direction data stream is moving in. true = forward, false = backwards
    bool update = false;    // Whether or not the "current" location entries have been updated
    float speed = 1;        // Current speed of playback

    locationEntry blank;

    std::vector<locationEntry>  current(data.size(), blank);
    std::vector<std::vector<locationEntry>::iterator> it(data.size());
    for(int i = 0; i < data.size(); i++)
        it[i] = data[i].begin();

    clock_t startTime;
    clock_t sumTime = 0;//35000000;
    double secondsPassed;
    
    sf::RenderWindow window(sf::VideoMode(1, 1), "Location Data Parser");
    while (window.isOpen()) 
    {
		sf::Event event;

		// Poll for a key press
		while (window.pollEvent(event))
        {
			switch (event.type)
            {
                case sf::Event::Closed:
					window.close();
					break;
                case sf::Event::KeyPressed:
                    switch(keyPressed(event))
                    {
                        case 0:
                            std::cout << "Key not recognized" <<std::endl;
                            break;
                        case 1:
                            std::cout << "Quitting..." << std::endl;
                            window.close();
                            break;
                        case 2:
                            if(play)
                            {
                                std::cout << "Pausing playback" << std::endl;

                                // if you are moving forward then add the elapsed time to the total
                                if(forward)
                                    sumTime += (clock() - startTime) * speed;

                                // otherwise subtract the elapsed time from the total
                                else
                                    sumTime -= (clock() - startTime) * speed;
                            }else
                            {
                                // Start counting from now
                                std::cout << "Resuming" << std::endl;
                                startTime = clock();
                            }
                                
                            play = !play;
                            break;
                        case 3:
                            if(forward)
                            {
                                // If moving forwards, decrement the iterators by 2, once to reach the current value and again to move behind it
                                std::cout << "Scanning backwards" << std::endl;
                                for(int i = 0; i < it.size(); i++)
                                    it[i] -= 2;
                                if(play)
                                    sumTime += (clock() - startTime) * speed;
                            }else
                            {
                                // Otherwise increment the iterators by 2, once to reach the current value and again to move ahead of it
                                std::cout << "Scanning forwards" << std::endl;
                                for(int i = 0; i < it.size(); i++)
                                    it[i] += 2;
                                if(play)
                                    sumTime -= (clock() - startTime) * speed;
                            }
                            if(play)
                                startTime = clock();
                            forward = !forward;
                            break;
                        case 4:
                            if(play){
                                if(forward)
                                    sumTime += (clock() - startTime) * speed;
                                else
                                    sumTime -= (clock() - startTime) * speed;
                                startTime = clock();
                            }
                            // Speed moves cyclically from .5->1->2->.5
                            if(speed == 2)
                                speed = .5;
                            else
                                speed *= 2;
                            std::cout << "Speed set to " << speed << "x speed" << std::endl;
                            break;
                    }
					break;
            }
        }

        // If playback is not paused
        if(play)
        {
            // If we are going forward
            if(forward)
            { 
                // Total time plus the elapsed time since the last change
                secondsPassed = (((clock() - startTime) * speed) + sumTime) / CLOCKS_PER_SEC;
                for(int i = 0; i < current.size(); i++)
                {
                    // If the current time is equal to or greater than the next timestamp in ascending chronological order
                    if(secondsPassed >= it[i][0].timestamp)
                    {
                        // Assign a new value to the current object and increment the iterator
                        current[i] = it[i][0];
                        it[i]++;
                        update = true;
                    }
                }
            // Otherwise we are moving backwards
            }else
            {
                // Total time minus the elapsed time since the last change
                secondsPassed = (sumTime - ((clock() - startTime) * speed)) / CLOCKS_PER_SEC;
                for(int i = 0; i < current.size(); i++)
                {
                    // If the current time is less than or equal to the next timestamp in descending chronological order
                    if(secondsPassed <= it[i][0].timestamp)
                    {
                        // Assign a new value to the current object and decrement the iterator
                        current[i] = it[i][0];
                        it[i]--;
                        update = true;
                    }
                }
            }

            // If the current data vector has been updated, display the new data
            if(update)
            {
                displayLocationData(current, map);
                update = false;
            }
        }
    }
}

/*
*   Input: Comman line arguments specifying the names of each users csv file
*   Output: N/A
*   Description: Takes in the names of each csv file (no practical limit) and, assuming they exist sends them to the tokenizer function
*/
int main(int argc, char *argv[])
{
    using namespace std;

    // Vector to hold any ammount of user log files
    vector<string>users;

    string osmFile = argv[1];

    // Load each argument (csv filename)
    for(int i = 2; i < argc; i++)
        users.push_back(argv[i]);

    // Vector to hold the entries to each users log files
    vector<vector<locationEntry>> data;

    // For each user log file, tokenize its entries and store them in the data vector
    std::cout << "Tokenizing csv files" << std::endl;
    for(int i = 0; i < users.size(); i++)
    {
        std::string filename;
        vector<locationEntry> temp;
        tokenizeLog(temp, users[i]);
        data.push_back(temp);
        
        filename.append("user");
        filename.append(std::to_string(i+1));
        filename.append("path.geojson");
        outputJson(data[i], filename);
    }

    std::cout << "Gathering map data from osm file" << std::endl;
    Map map = createMap(data, osmFile);

    for(int i = 0; i < data.size(); i++)
        getOccupiedBuildings(map, data[i], i+1);
    
    // Function to handle moving through data
    parseData(data, map);
    
    return 0;
}