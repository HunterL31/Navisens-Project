#ifndef HANDLERS_SRC
#define HANDLERS_SRC

#include "data.h"

#include <osmium/osm/types.hpp>
#include <osmium/geom/tile.hpp>
#include <osmium/relations/relations_manager.hpp>

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
                    tempBuilding.postalCode = tag.value();
                else if (!std::strncmp(tag.key(), "building", 8))
                    tempBuilding.type = tag.value();
                else if (!std::strncmp(tag.key(), "height", 6))
                    tempBuilding.height = tag.value();
                else if(!std::strncmp(tag.key(), "name", 4))
                    tempBuilding.name = tag.value();
           }
           buildings.push_back(tempBuilding);
        }
    }

    void outputBigBuildings(const osmium::Way& way, std::vector<building> &buildings){
        const osmium::TagList& tags = way.tags();
        if(tags.has_key("building")){
            building tempBuilding;

            for(auto& node : way.nodes())
                tempBuilding.nodeIds.push_back(node.ref());
            
            for(const osmium::Tag& tag : tags) {
                if(!std::strncmp(tag.key(), "addr:street", 11))
                    tempBuilding.street = tag.value();
                else if (!std::strncmp(tag.key(), "addr:housenumber", 16))
                    tempBuilding.houseNumber = tag.value();
                else if (!std::strncmp(tag.key(), "addr:postcode", 13))
                    tempBuilding.postalCode = tag.value();
                else if (!std::strncmp(tag.key(), "building", 8))
                    tempBuilding.type = tag.value();
                else if (!std::strncmp(tag.key(), "height", 6))
                    tempBuilding.height = tag.value();
                else if(!std::strncmp(tag.key(), "name", 4))
                    tempBuilding.name = tag.value();
            }
           buildings.push_back(tempBuilding);
        }
    }

    public:
        void node(const osmium::Node& node){
            outputBuildings(node, buildings);
        }
        void way(const osmium::Way& way){
            outputBigBuildings(way, buildings);
        }
        std::vector<building> getBuildings(){
            return buildings;
        }
    private:
        std::vector<building> buildings;
};

// Handler for osmium reader that gathers all highways and stores their information in a custom struct
class highwayHandler : public osmium::handler::Handler {
    void outputWays(const osmium::Way& way) {
        const osmium::TagList& tags = way.tags();
        if(tags.has_key("highway")){
            highway tempHighway;
            for(auto& node : way.nodes())
                tempHighway.nodeIds.push_back(node.ref());

            for(const osmium::Tag& tag : tags) {
                if(!std::strncmp(tag.key(), "highway", 7))
                    tempHighway.type = tag.value();
                if(!std::strncmp(tag.key(), "name", 4))
                    tempHighway.name = tag.value();
            }
            if(tempHighway.type == "residential")
                highways.push_back(tempHighway);
        
        }
    }
    public:
        void way(const osmium::Way& way){
            outputWays(way);
        }
        std::vector<highway> getHighways(){
            return highways;
        }
    private:
        std::vector<highway> highways;
};

#endif