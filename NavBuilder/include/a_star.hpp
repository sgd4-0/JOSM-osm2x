#ifndef NAV_SGD_A_STAR_HPP_
#define NAV_SGD_A_STAR_HPP_

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <math.h>

#include <stack>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <list>

#include "rapidxml-1.13/rapidxml.hpp"
#include "a_star_users.hpp"
#include "a_star_node.hpp"
#include "nav_params.hpp"

namespace nav_sgd
{

class A_Star
{

private:
    Nav_Params *parameters;

    const std::string map_file_;
    std::string adr_filename_;
    std::unordered_map<long, A_Star_Node> nodelist;
    std::unordered_map<long, A_Star_Node> closedList;
    
    // input data
    double st_lat_ = 0.0, st_lon_ = 0.0, de_lat_ = 0.0, de_lon_ = 0.0;
    std::string address_;

    // Store map data
    A_Star_Users* users;
    
    A_Star_Node get_node_from_lat_lon(double const lat, double const lon);

    void trace_path(long);
    double calc_cost_factor(rapidxml::xml_node<char> *node);
    //! \brief checks if the given node is contained in the set.
    //! \returns a copy of the node with id
    A_Star_Node is_in_set(std::set<A_Star_Node>*, A_Star_Node);
    
    int compute_path(A_Star_Node start_node, A_Star_Node dest_node);
    void import_nav_file();

    bool compare_strings(std::string s1, std::string s2);

    //! \brief Double to string with 10 digit precision
	std::string to_string( double d ) {
        std::ostringstream stm ;
        stm << std::setprecision(10) << d ;
        return stm.str() ;
    }

public:
    A_Star(Nav_Params& params, std::string osm_map_file, std::string users_file);
    ~A_Star();

    // Store waypoints from start to destination
    std::vector<A_Star_Node> waypoints;

    void set_user(std::string username);
    void set_start(double lat, double lon);
    void set_destination(double lat, double lon);
    void set_destination(std::string address);

    int compute_path();
};

A_Star::A_Star(Nav_Params& params, std::string osm_map_file, std::string users_file)
    : map_file_(osm_map_file)
{
    parameters = &params;
    users = new A_Star_Users(users_file);
    adr_filename_ = map_file_.substr(0,map_file_.rfind('.')) + ".adr";
}

A_Star::~A_Star()
{ 
    // destrcutor
}

void
A_Star::set_user(std::string username)
{
    std::cout << "Set username to " << username << "\n";
    users->set_user(username);
}

void
A_Star::set_start(double lat, double lon)
{
    std::cout << "Set start to " << lat << ", " << lon << "\n";
    st_lat_ = lat;
    st_lon_ = lon;
}

void
A_Star::set_destination(double lat, double lon)
{
    std::cout << "Set destination to " << lat << ", " << lon << "\n";
    de_lat_ = lat;
    de_lon_ = lon;
}

void
A_Star::set_destination(std::string address)
{
    std::cout << "Set destination to " << address << "\n";
    address_ = address;
}

int
A_Star::compute_path()
{
    // reload params
    parameters->reload();

    // import nav file and get start node
    import_nav_file();
    A_Star_Node start_node = get_node_from_lat_lon(st_lat_, st_lon_);
    if (start_node.id < 0)
    {
        std::cerr << "Could not find startnode near " << to_string(st_lat_) << ", " << to_string(st_lon_) << "\n";
        return -1;
    }

    // try to find address in addresslist
    if (!address_.empty())
    {
        // read address file and try to find address
        // open address file
        std::ifstream adr_file(adr_filename_);
        std::string line;
        long dest_id = 0;
        while (getline(adr_file, line))
        {
            // search for given address
            if (*line.begin() == '#')    continue;

            std::string a = line.substr(0, line.find(':'));
            std::cout << "Compare: -" << a << "- und -" << address_ << "- \n";
            if (compare_strings(a, address_))
            {
                try
                {
                    std::cout << "Set dest id to " << line.substr(line.find(':') + 1) << "\n";
                    dest_id = stol(line.substr(line.find(':') + 1));
                    break;
                }
                catch(const std::exception& e)
                {
                    std::cerr << line.substr(line.find(':') + 1) << '\n';
                }
            }
        }
        if (nodelist.count(dest_id) > 0)
        {
            std::cout << "Get id " << dest_id << " from nodelist.\n";
            A_Star_Node dest_node = nodelist.at(dest_id);
            compute_path(start_node, dest_node);
            return 0;
        }
        std::cout << "Address " << address_ << " not found. Try to find destination with lat, lon.\n";
    }
    
    // if address is not given or not found
    if (de_lat_ != 0)
    {
        A_Star_Node dest_node = get_node_from_lat_lon(de_lat_, de_lon_);
        if (dest_node.id < 0)
        {
            std::cout << "Could not find start or destination node. Terminating program.\n";
            return -1;
        }
        compute_path(start_node, dest_node);
        return 0;
    }
    return -1;
}

int
A_Star::compute_path(A_Star_Node start_node, A_Star_Node dest_node)
{
    // create open list and closed list
    std::cout << "Compute path from " << start_node.id << " to " << dest_node.id << "\n";
    auto compare = [](A_Star_Node a, A_Star_Node b) {return a.f() > b.f();};
    
    std::ofstream out_file("../Karten/a_star_debug.txt", std::ios::out | std::ios::trunc);
    out_file << "Compute path from " << start_node.id << " to " << dest_node.id << "\n";
    std::set<A_Star_Node> openList;
    closedList.clear();

    // get parameters
    double factor_a = parameters->param_as_double("factor_a");
    double factor_b = parameters->param_as_double("factor_b");
    double factor_c = parameters->param_as_double("factor_c");

    std::cout << "Faktor a: " << factor_a << "\n";
    std::cout << "Faktor b: " << factor_b << "\n";
    std::cout << "Faktor c: " << factor_c << "\n";

    // Initialise parameters of starting node
    openList.insert(start_node);
    
    // initially the destination is not reached
    bool foundDest = false;
    while (!openList.empty())
    {
        // get node with lowest f value and remove from openList
        A_Star_Node pos = *openList.begin();
        out_file << "Check node " << pos.id << "\n";

        openList.erase(openList.begin());
        closedList.insert(std::make_pair(pos.id, pos));

        if (pos == dest_node)
        {
            // destination found
            foundDest = true;
            trace_path(pos.id);
            break;
        }

        if (pos.is_blocked)
        {
            out_file << "Node is blocked.\n";
            continue;
        }

        // expand node
        out_file << "Expand ids:\n";
        //std::set<long> n;
        long last_id_ = 0;
        double phi_transf_ = 0.0, lo_tr = 0.0, la_tr = 0.0;
        double y_abst_;
        for (auto entry : pos.cost_map)
        {
            long next_id = entry.first;
            out_file << next_id << ":";

            if (closedList.count(next_id) > 0)
            {
                out_file << "Already on closed list.\n";
                continue;  // successor already on closed list
            }

            auto next_pos = nodelist.find(next_id)->second;
            double add_cost = 1.0;
            if (next_id % 100 == 0)  // -> 00 node
            {
                // calc transform
                last_id_ = next_id;
                phi_transf_ = atan2(next_pos.lat - pos.lat, next_pos.lon - pos.lon);
                lo_tr = pos.lon * cos(phi_transf_) + pos.lat * sin(phi_transf_);
                la_tr = pos.lat * cos(phi_transf_) - pos.lon * sin(phi_transf_);

                y_abst_ = next_pos.lat * cos(phi_transf_) - next_pos.lon * sin(phi_transf_);
                out_file << "phi:" << phi_transf_ << ",";
            }
            else if (next_id - last_id_ < 99)   // child node from last_id
            {
                // transform coordinates
                double abstand = y_abst_ - (next_pos.lat * cos(phi_transf_) - next_pos.lon * sin(phi_transf_));
                //add_cost = (abstand > 0 ? 0.8 : 1.4);   // atan / Funktion

                // transform coordinates
                double lo = next_pos.lon * cos(phi_transf_) + next_pos.lat * sin(phi_transf_);
                double la = next_pos.lat * cos(phi_transf_) - next_pos.lon * sin(phi_transf_);

                //double angle = atan2(next_pos.lat - pos.lat, next_pos.lon - pos.lon);
                double angle = atan2(la - la_tr, lo - lo_tr);
                out_file << "ang: " << angle << ",";
                add_cost =  factor_a * exp(-pow(angle*factor_b,2)) * angle
                            + 1 + factor_c * tanh(5*angle);
                
                //if (abstand > 0) {
                //    out_file << "r";
                //} else {
                //    out_file << "l";
                //}
            }

            // update node
            out_file << next_pos.set_parent(pos, dest_node, add_cost);

            // search for next pos in openList
            A_Star_Node p = is_in_set(&openList, next_pos);
            if (p.id > 0 && p.g() > next_pos.g())
            {
                openList.erase(p);
            }
            else if (p.id > 0 && p.g() < next_pos.g())
            {
                continue;
            }
            openList.insert(next_pos);
        }
    }

    if (!foundDest)
    {
        out_file << "Failed to compute path to destination.\n";
        std::cout << "Failed to compute path to destination.\n";
        return 1;
    }
    out_file.close();

    // reset start values except user
    st_lat_ = 0.0;
    st_lon_ = 0.0;
    de_lat_ = 0.0;
    de_lon_ = 0.0;
    address_.clear();

    return 0;
    // TODO write nodes to file
}

void
A_Star::import_nav_file()
{
    // read nav file and try to find start and destination node
    std::cout << "Parse map file " << map_file_ << std::endl;
    std::ifstream t(map_file_);

    auto buffer = std::vector<char>((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    rapidxml::xml_document<> osm;
    osm.parse<0>(&buffer[0]);

    nodelist.clear();
    rapidxml::xml_node<>* root = osm.first_node("nodelist");
    for (rapidxml::xml_node<> *node = root->first_node("node"); node; node = node->next_sibling())
    {
        long id = std::stol(node->first_attribute("id")->value());
        double lat = std::stod(node->first_attribute("lat")->value());
        double lon = std::stod(node->first_attribute("lon")->value());
        
        A_Star_Node n(id, lat, lon);

        // search for barrier attribute
        if (node->first_node("barrier") != nullptr)
        {
            n.is_blocked = true;
        }

        for (rapidxml::xml_node<> *nd = node->first_node("nd"); nd; nd = nd->next_sibling("nd"))
        {
            long ndid = std::stol(nd->first_attribute("ref")->value());
            n.cost_map.insert(std::make_pair(ndid, calc_cost_factor(nd)));
        }
        nodelist.insert(std::make_pair(id, n));
    }
}

A_Star_Node
A_Star::get_node_from_lat_lon(const double lat, const double lon)
{
    double lastDist = 10000.0;
    long nnode = -1;

    for (auto node : nodelist)
    {
        double lastDist_;
        if((lastDist_ = 111319*std::sqrt(std::pow(node.second.lat - lat, 2) 
                + std::pow((node.second.lon - lon) * cos(lat*180/PI), 2))) < lastDist )
        {
            lastDist = lastDist_;
            nnode = node.first;
        }
    }

    if (nnode < 0 || lastDist > parameters->param_as_double("max_dist_from_node"))
    {
        return A_Star_Node(-1,0,0);
    }
    return nodelist.at(nnode);
}

double
A_Star::calc_cost_factor(rapidxml::xml_node<char> *node)
{
    double f = 0;
    for (rapidxml::xml_node<> *n = node->first_node(); n; n = n->next_sibling())
    {
        f += users->calculate_factor(n->name(), n->value());
    }
    return f <= 0 ? 1E6 : f;
}

void
A_Star::trace_path(long dest_id)
{
    if (!waypoints.empty()) waypoints.clear();

    std::stack<A_Star_Node> path;
    long pid = dest_id;

    while (pid > -1)
    {
        long id = pid;
        auto node = *closedList.find(id);
        path.push(node.second);
        pid = node.second.parent_id();
    }

    double lat_, lon_;

    std::ofstream out_file("../Karten/4_waypoints.osm", std::ios::out | std::ios::trunc);
    out_file << "<?xml version='1.0' encoding='utf-8'?>\n";
    out_file << "<osm version='0.6'>\n";

    for (auto p : nodelist)
    {
        out_file << "  <node id='" << p.second.id << "' ";
        out_file << "version='1' lat='" << to_string(p.second.lat) << "' ";
        out_file << "lon='" << to_string(p.second.lon) << "' />\n";
    }

    out_file << "  <way id='123456' version='1'>\n";
    while (!path.empty())
    {
        A_Star_Node p = path.top();
        path.pop();

        waypoints.push_back(p);
        out_file << "    <nd ref='" << p.id << "'/>\n";
        //std::cout << "Waypoint id: " << p.id << std::endl;
    }
    out_file << "  </way>\n";
    out_file << "</osm>";
    out_file.close();
}

A_Star_Node
A_Star::is_in_set(std::set<A_Star_Node> *set, A_Star_Node node)
{
    int i = 0;
    for (auto n = set->begin(); n != set->end(); n++)
    {
        if (n->id == node.id)
        {
            return *n;
        }
        i++;
    }
    return A_Star_Node(-1,0,0);
}

bool
A_Star::compare_strings(std::string s1, std::string s2)
{
    // remove trailing whitespaces
    // TODO remove leading whitespaces
    s1.erase(s1.find_last_not_of(" \n\r\t")+1);
    s2.erase(s2.find_last_not_of(" \n\r\t")+1);

    if (s1.size() == s2.size())
    {
        for (int i = 0; i < s1.size(); i++)
        {
            if (s1.at(i) != s2.at(i))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

}   // namespace nav_sgd

#endif