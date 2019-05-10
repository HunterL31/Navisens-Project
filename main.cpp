#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <string>

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

int main(int argc, char *argv[])
{

    // Filenames of csv files to be parsed
    std::string log1(argv[1]);
    std::string log2(argv[2]);

    // Vector arrays to hold each entry in the log files
    std::vector<locationEntry> user1Data, user2Data;

    // Populate the vectors with user data
    tokenizeLog(user1Data, log1);
    tokenizeLog(user2Data, log2);

    // Print the size of each vector for debugging purposes
    std::cout << user1Data.size() << std::endl;
    std::cout << user2Data.size() << std::endl;
    
    return 0;
}