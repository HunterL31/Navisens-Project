#ifndef DATA_SRC
#define DATA_SRC

#include <vector>
#include <iostream>
#include <osmium/osm/types.hpp>
#include <osmium/geom/tile.hpp>

// Holds the values for a single entry in the user log file
struct locationEntry {
    double timestamp = -1;    // Timestamp value
    double navLat;              // Navisens Latitude/Longitude/Altitude
    double navLon;
    double navAlt;
    double gpsLat;              // GPS Latitude/Longitude/Altitude
    double gpsLon;
    double gpsAlt;
};

struct building {
    osmium::Location location;                   // Used for nodes that represent buildings
    std::vector<osmium::Location> nodeLocations; // Used when a way represents the building
    std::vector<int> nodeIds;                    // Used when a way represents the building
    std::string type;
    std::string street;
    std::string houseNumber;
    std::string postalCode;
    std::string height;
    std::string name;
    bool entered = false;
};

struct highway {
    std::vector<int> nodeIds;
    std::string type;
    std::string name;
};

struct featurePolygon {
    std::string type;
    std::vector<std::string> coordinates;
    bool entered = false;
};

struct featureMultiLineString {
    std::string type;
    std::vector<std::string> coordinates1;
    std::vector<std::string> coordinates2;
};

struct featureLineString {
    std::string type;
    std::string user;
    std::vector<std::string> coordinates;
};
#endif