#ifndef MAP_SRC
#define MAP_SRC

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "data.h"
#include "handlers.h"

#include <osmium/osm/types.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/relations/relations_manager.hpp>
#include <osmium/geom/tile.hpp>

#define ZOOM 17
/* Zoom Value Meaning
* 11 - City
* 12 - Town/City District
* 13 - Suburb
* 14 -
* 15 - Small Road
* 16 - Street
* 17 - Block/Park
* 18 - Building/Tree
*/

class Map 
{
    public:
        Map(std::vector<std::vector<locationEntry>> &data, std::string osmFile);
        std::vector<int> getIds(osmium::Location &loc);
        std::vector<building> getBuildings(osmium::Location &loc);
        std::vector<building> getBuildings();
        std::vector<highway> getHighways(osmium::Location &loc);

    private:
        void gatherNodes();
        bool checkForId(int id);
        osmium::Location getIdLocation(int id);
        std::vector<osmium::geom::Tile> tiles;
        std::vector<building> nearbyBuildings;
        std::vector<highway> nearbyHighways;
        std::string osmFile;
        std::map<int, osmium::Location> nodeHashMap;
};

/*  Constructor
*   Input: locationEntry vector and name of osm file
*   Output: Map object that contains the id of all necesarry nodes
*/
Map::Map(std::vector<std::vector<locationEntry>> &data, std::string file)
{
    osmFile = file;

    std::vector<osmium::Location> coords;

    for(int i = 0; i < data.size(); i++)
    {
        for(int j = 0; j < data[i].size(); j++)
        {
            osmium::Location temp{data[i][j].navLon, data[i][j].navLat};
            coords.push_back(temp);
        }
    }
    sort( coords.begin(), coords.end() );
    coords.erase( unique( coords.begin(), coords.end() ), coords.end() );

    for(auto& loc : coords)
    {
        osmium::geom::Tile tempTile(ZOOM, loc);
        tiles.push_back(tempTile);
    }
    sort( tiles.begin(), tiles.end() );
    tiles.erase( unique( tiles.begin(), tiles.end() ), tiles.end() );

    gatherNodes();
}

/*
*   Input: location
*   Output: A list of the id of every node that exists within the same tile as the input
*/
std::vector<int> Map::getIds(osmium::Location &loc)
{
    std::vector<int> ids;

    osmium::geom::Tile userTile(ZOOM, loc);
    
    for(std::map<int, osmium::Location>::iterator i = nodeHashMap.begin(); i != nodeHashMap.end(); i++){
        osmium::geom::Tile tempTile(ZOOM, i->second);
        if(userTile == tempTile)
            ids.push_back(i->first);
    }

    return ids;
}

/*
*   Input: location
*   Output: A list of buildings that exist within the same tile as the users location and their information
*/
std::vector<building> Map::getBuildings(osmium::Location &loc)
{
    std::vector<building> buildings;

    osmium::geom::Tile userTile(ZOOM, loc);
    for(auto& b : nearbyBuildings)
    {
        if(b.location.is_defined()){
            osmium::geom::Tile tempTile(ZOOM, b.location);
            if(tempTile == userTile)
                buildings.push_back(b);
        }else if(b.nodeLocations.size() > 0){
            for(auto& l : b.nodeLocations){
                osmium::geom::Tile tempTile(ZOOM, l);
                if(tempTile == userTile){
                    buildings.push_back(b);
                    break;
                }
            }
        }
    }
    return buildings;
}

std::vector<building> Map::getBuildings()
{
    return nearbyBuildings;
}

/*
* Input: user location
* Output: roads near the user
* Description: Takes the users location and returns all nearby residential roads
*/
std::vector<highway> Map::getHighways(osmium::Location &loc)
{
    return nearbyHighways;
}

/*
*   Input: osm file name
*   Output: Nothing
*   Description: Given the provided osm file, it stores information about every node we could possibly be interested in
*/
void Map::gatherNodes()
{
    std::vector<std::pair<osmium::Location, int>> nodes;
    std::vector<building> buildings;
    std::vector<highway> highways;
    try{
        osmium::io::Reader reader{osmFile, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
        nodeHandler nHandler;
        buildingHandler bHandler;
        highwayHandler hHandler;

        osmium::apply(reader, nHandler, bHandler, hHandler);
        reader.close();

        nodes = nHandler.getNodes();
        buildings = bHandler.getBuildings();
        highways = hHandler.getHighways();

        std::cout << "\nTotals" << std::endl;
        std::cout << "Nodes: " << nodes.size() << " | Buildings: " << buildings.size() << " | " << "Highways: " << highways.size() << std::endl;

        for(auto& loc: nodes){
            osmium::geom::Tile tempTile(ZOOM, loc.first);
            for(auto& tile : tiles)
                if(tile == tempTile)
                    nodeHashMap[loc.second] = loc.first;
                
                   
        }

        for(auto& b : buildings){
            if(b.location.is_defined()){
                osmium::geom::Tile tempTile(ZOOM, b.location);
                for(auto& tile: tiles)
                    if(tile == tempTile){
                        nearbyBuildings.push_back(b);
                        break;
                        //std::cout << "Building:" << b.type << " at " << b.houseNumber << " " << b.street << " called " << b.name << std::endl;
                    }
            }else if(b.nodeIds.size() > 0){
                for(auto& id : b.nodeIds){
                    if (nodeHashMap.find(id) != nodeHashMap.end()){
                        b.nodeLocations.push_back(nodeHashMap[id]);
                    }
                }
                if(b.nodeLocations.size() > 0){
                    nearbyBuildings.push_back(b);
                    //std::cout << "Building:" << b.type << " at " << b.houseNumber << " " << b.street << " called " << b.name << std::endl;
                }
            }
        }
        for(auto& highway : highways){
            for(auto& nid : highway.nodeIds){
                if(checkForId(nid)){
                    nearbyHighways.push_back(highway);
                    break;
                }
            }       
        }

        std::cout << "\nRelevant" << std::endl;
        std::cout << "Nodes: " << nodeHashMap.size() << " | Buildings: " << nearbyBuildings.size() << " | " << "Highways: " << nearbyHighways.size() << std::endl;
        
    } catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        std::exit(1);
    }
}

/*
* Input: node id
* Output: Whether or not that node is nearby the user
* Description: Uses hashmap search to determine whether or not the specified node is nearby the user
*/
bool Map::checkForId(int id)
{
    if(nodeHashMap.find(id) != nodeHashMap.end())
        return true;
    return false;
}

/*
* Input: id
* Output: Location of node
* Description: Determines the location of a specified node using hashmap search 
*/
osmium::Location Map::getIdLocation(int id)
{
   if (nodeHashMap.find(id) != nodeHashMap.end()){
        return nodeHashMap[id];
    }
}

#endif