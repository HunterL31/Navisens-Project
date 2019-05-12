#ifndef MAP_SRC
#define MAP_SRC

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <osmium/osm/types.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/relations/relations_manager.hpp>
#include <osmium/geom/tile.hpp>

// Holds the values for a single entry in the user log file
struct locationEntry {
    double timestamp = NULL;    // Timestamp value
    double navLat;              // Navisens Latitude/Longitude/Altitude
    double navLon;
    double navAlt;
    double gpsLat;              // GPS Latitude/Longitude/Altitude
    double gpsLon;
    double gpsAlt;
};

struct building {
    osmium::Location location;
    std::string type;
    std::string street;
    std::string houseNumber;
    std::string postalCode;
    std::string height;
};

// Handler for osmium reader that gathers relevant data for all nodes
class nodeHandler : public osmium::handler::Handler {

    public:
        
        void node(const osmium::Node& node){
            nodes.push_back(std::make_pair(node.location(), node.id()));
        }
        std::vector<std::pair<osmium::Location, int>> getNodes(){
            return nodes;
        }
    private:
        std::vector<std::pair<osmium::Location, int>> nodes;

};

// Handler for osmium reader that gathers all buildings and stores their information in a custom struct
class buildingHandler : public osmium::handler::Handler {

    void outputBuildings(const osmium::Node& node, std::vector<building> &buildings){
        const osmium::TagList& tags = node.tags();
        if (tags.has_key("building")) {
           building tempBuilding;
           tempBuilding.location = node.location();
           for(const osmium::Tag& tag : tags) {
                if(!std::strncmp(tag.key(), "addr:street", 11))
                    tempBuilding.street = tag.value();
                else if (!std::strncmp(tag.key(), "addr:housenumber", 16))
                    tempBuilding.houseNumber = tag.value();
                else if (!std::strncmp(tag.key(), "addr:postcode", 13))
                    tempBuilding.postalCode == tag.value();
                else if (!std::strncmp(tag.key(), "building", 8))
                    tempBuilding.type == tag.value();
                else if (!std::strncmp(tag.key(), "height", 6))
                    tempBuilding.height == tag.value();
           }
           buildings.push_back(tempBuilding);
        }
    }

    public:
        void node(const osmium::Node& node){
            outputBuildings(node, buildings);
        }
        std::vector<building> getBuildings(){
            return buildings;
        }
    private:
        std::vector<building> buildings;
};

class Map 
{
    public:
        Map(std::vector<std::vector<locationEntry>> &data, std::string osmFile);
        std::vector<int> getIds(osmium::Location &loc);
        std::vector<building> getBuildings(osmium::Location &loc);

    private:
        void gatherNodes();
        std::vector<osmium::geom::Tile> tiles;
        std::vector<std::pair<int, osmium::geom::Tile>> nodeIds;
        std::vector<building> nearbyBuildings;
        std::string osmFile;
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
        osmium::geom::Tile tempTile(16, loc);
        tiles.push_back(tempTile);
    }
    sort( tiles.begin(), tiles.end() );
    tiles.erase( unique( tiles.begin(), tiles.end() ), tiles.end() );

    gatherNodes();
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
    try{
        osmium::io::Reader reader{osmFile, osmium::osm_entity_bits::node};
        nodeHandler nHandler;
        buildingHandler bHandler;

        osmium::apply(reader, nHandler, bHandler);
        reader.close();

        nodes = nHandler.getNodes();
        buildings = bHandler.getBuildings();

        for(auto& loc: nodes){
            osmium::geom::Tile tempTile(16, loc.first);
            for(auto& tile : tiles)
                if(tile == tempTile)
                    nodeIds.push_back(std::make_pair(loc.second, tempTile));
        }

        for(auto& b : buildings){
            osmium::geom::Tile tempTile(16, b.location);
            for(auto& tile: tiles)
                if(tile == tempTile)
                    nearbyBuildings.push_back(b);
        }
    } catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        std::exit(1);
    }
}

/*
*   Input: locationEntry
*   Output: A list of the id of every node that exists within the same tile as the input
*/
std::vector<int> Map::getIds(osmium::Location &loc)
{
    std::vector<int> ids;

    osmium::geom::Tile tempTile(16, loc);
    for(auto& node : nodeIds)
    {
        if(node.second == tempTile)
            ids.push_back(node.first);
    }

    return ids;
}

/*
*   Input: locationEntry
*   Output: A list of buildings that exist within the same tile as the users location and their information
*/
std::vector<building> Map::getBuildings(osmium::Location &loc)
{
    std::vector<building> buildings;

    osmium::geom::Tile userTile(16, loc);
    for(auto& b : nearbyBuildings)
    {
        osmium::geom::Tile tempTile(16, b.location);
        if(tempTile == userTile)
            buildings.push_back(b);
    }

    return buildings;
}
#endif