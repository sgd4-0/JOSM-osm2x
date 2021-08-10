
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <ctime>

void add_obstacle(std::string mapfile, double lat, double lon)
{
    // go through file and search for lat lon
    std::string line;
    std::fstream myfile(mapfile, std::ios::in);
    std::regex reg_lat("lat='([\\d.]*)'");
    std::regex reg_lon("lon='([\\d.]*)'");

    std::smatch res_lat;
    std::smatch res_lon;

    time_t now = time(0);
    std::vector<std::string> lines;

    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            lines.push_back(line);
            if (std::regex_search(line, res_lat, reg_lat))
            {
                std::regex_search(line, res_lon, reg_lon);

                double node_lat = 0;
                double node_lon = 0;
                try
                {
                    node_lat = std::stod(res_lat[0]);
                    node_lon = std::stod(res_lon[0]);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
                
                if ((lat == node_lat) && (lon == node_lon))
                {
                    lines.push_back("    <barrier>temp_" + std::to_string(now) + "</barrier>");
                }
            }
        }
        myfile.close();
    }

    myfile.open(mapfile, std::ios::out | std::ios::trunc);
    if (myfile.is_open())
    {
        for (std::string line : lines)
        {
            // write line to file
            myfile << line << "\n";
        }
        myfile.close();
    }
}

void remove_obstacle(std::string mapfile)
{
    std::fstream myfile(mapfile, std::ios::in);

    std::regex reg_barrier("barrier>temp_(\\d*)</barrier>");
    std::smatch res_barrier;

    std::string line;
    std::vector<std::string> lines;

    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            if (!std::regex_search(line, res_barrier, reg_barrier))
            {
                lines.push_back(line);
            }
        }
        myfile.close();
    }

    myfile.open(mapfile, std::ios::out | std::ios::trunc);
    if (myfile.is_open())
    {
        for (std::string line : lines)
        {
            // write line to file
            myfile << line << "\n";
        }
        myfile.close();
    }
}