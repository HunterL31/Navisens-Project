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

// Handler for osmium reader that gathers relevant data for all nodes
class nodeHandler : public osmium::handler::Handler {

    public:
        std::vector<std::pair<osmium::Location, int>> nodes;
        void node(const osmium::Node& node){
            nodes.push_back(std::make_pair(node.location(), node.id()));
        }
};

// Handler for osmium reader that prints out all bars. Can easily be modified to search for any tag and to limit its search to a range of ids
class nameHandler : public osmium::handler::Handler {

    void output_bars(const osmium::OSMObject& object){
        const osmium::TagList& tags = object.tags();
        if (tags.has_tag("amenity", "bar")) {

            // Print name of the pub if it is set.
            const char* name = tags["name"];
            if (name) {
                std::cout << name << "\n";
            } else {
                std::cout << "bar with unknown name\n";
            }

            // Iterate over all tags finding those which start with "addr:"
            // and print them.
            for (const osmium::Tag& tag : tags) {
                if (!std::strncmp(tag.key(), "addr:", 5)) {
                    std::cout << "  " << tag.key() << ": " << tag.value() << "\n";
                }
            }
        }
    }

    public:
        void node(const osmium::Node& node){
            output_bars(node);
        }

        void way(const osmium::Way& way) {
            output_bars(way);
        }
};

class Map 
{
    public:
        Map(std::vector<std::vector<locationEntry>> &data, std::string osmFile);
        std::vector<int> getIds(locationEntry &loc);

    private:
        void gatherNodes(std::string osmFile);
        std::vector<osmium::geom::Tile> tiles;
        std::vector<std::pair<int, osmium::geom::Tile>> nodeIds;
};

/*  Constructor
*   Input: locationEntry vector and name of osm file
*   Output: Map object that contains the id of all necesarry nodes
*/
Map::Map(std::vector<std::vector<locationEntry>> &data, std::string osmFile)
{

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

    gatherNodes(osmFile);
}

/*
*   Input: osm file name
*   Output: Nothing
*   Description: Given the provided osm file, it loads the id and respective tile of every node we could use
*/
void Map::gatherNodes(std::string osmFile)
{

    try{
        osmium::io::Reader reader{osmFile, osmium::osm_entity_bits::node};
        nodeHandler handler;

        osmium::apply(reader, handler);
        reader.close();

        for(auto& loc: handler.nodes){
            osmium::geom::Tile tempTile(16, loc.first);
            for(auto& tile : tiles)
                if(tile == tempTile)
                    nodeIds.push_back(std::make_pair(loc.second, tempTile));
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
std::vector<int> Map::getIds(locationEntry &loc)
{
    std::vector<int> ids;

    osmium::Location tempLocation(loc.navLon, loc.navLat);
    osmium::geom::Tile tempTile(16, tempLocation);
    for(auto& node : nodeIds)
    {
        if(node.second == tempTile)
            ids.push_back(node.first);
    }

    return ids;
}

#endif