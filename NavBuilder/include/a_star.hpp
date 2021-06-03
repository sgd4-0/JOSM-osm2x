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

namespace nav_sgd
{

class A_Star
{

//! \brief Struct to hold node id, latitude and longitude
struct NODE
{
    rapidxml::xml_node<char> *xml_node;
    rapidxml::xml_node<char> *parent_xml_node;
    double f,g,h;
};

//! \brief Struct to hold position information latitude, longitude and angle around z in radians.
struct POSE
{
    double lat, lon, angle;
};

private:
    const std::string map_file_;
    // Store waypoints from start to destination
    std::vector<POSE> waypoints;
    // Store map data
    std::vector<POSE> map_data;
    A_Star_Users* users;

    rapidxml::xml_node<char>* get_node_from_lat_lon(rapidxml::xml_node<>* root, double const lat, double const lon);
    void trace_path(std::unordered_map<rapidxml::xml_node<char> *, rapidxml::xml_node<char> *> closedList, rapidxml::xml_node<char> * dest);
    double calc_cost_factor(rapidxml::xml_node<char> *node);
    double distance_node_to_node(rapidxml::xml_node<char> *start, rapidxml::xml_node<char> *dest);
    bool isDestination(rapidxml::xml_node<char> *pos, rapidxml::xml_node<char> *dest);
    
public:
    A_Star(std::string osm_map_file, std::string users_file);
    ~A_Star();

    int compute_path(double start_lat, double start_lon, double end_lat, double end_lon, std::string username="default");
};

A_Star::A_Star(std::string osm_map_file, std::string users_file)
    : map_file_(osm_map_file)
{
    users = new A_Star_Users(users_file);
}

A_Star::~A_Star()
{ 
    // destrcutor
}

int
A_Star::compute_path(double start_lat, double start_lon, double end_lat, double end_lon, std::string username)
{
    // read nav file and try to find start and destination node
    std::cout << "Parse map file " << map_file_ << std::endl;
    std::ifstream t(map_file_);

    auto buffer = std::vector<char>((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    rapidxml::xml_document<> osm;
    osm.parse<0>(&buffer[0]);

    rapidxml::xml_node<>* root = osm.first_node("nodelist");
    rapidxml::xml_node<char> *start_node = get_node_from_lat_lon(root, start_lat, start_lon);
    rapidxml::xml_node<char> *dest_node = get_node_from_lat_lon(root, end_lat, end_lon);

    // set user
    std::cout << "Set user to " << username << std::endl;
    users->set_user(username);

    // create open list and closed list
    auto compare = [](NODE a, NODE b) {return a.f > b.f;};
    std::priority_queue<NODE, std::vector<NODE>, decltype(compare)> openList(compare);
    std::unordered_map<rapidxml::xml_node<char> *, rapidxml::xml_node<char> *> closedList;

    // Initialise parameters of starting node
    NODE d;
    d.xml_node = start_node;
    d.parent_xml_node = NULL;
    d.f = 0.0;
    d.g = 0.0;  // bisherige Kosten
    d.h = 0.0;  // estimated cost to destination
    openList.push(d);

    // initially the destination is not reached
    bool foundDest = false;
    while (!openList.empty())
    {
        NODE olistData = openList.top();

        rapidxml::xml_node<char> *pos = olistData.xml_node;

        // Remove this node from the open list
        openList.pop();

        if (isDestination(pos, dest_node))
        {
            closedList.insert(std::make_pair(olistData.xml_node, olistData.parent_xml_node));
            foundDest = true;
            trace_path(closedList, dest_node);

            break;
        }

        for (rapidxml::xml_node<> *nd = pos->first_node("nd"); nd; nd = nd->next_sibling("nd"))
        {
            long pos_id = strtol(nd->first_attribute("ref")->value(), NULL, 10);

            for (rapidxml::xml_node<> *np = root->first_node("node"); np; np = np->next_sibling())
            {
                if ( strtol(np->first_attribute("id")->value(), NULL, 10) == pos_id )
                {
                    // If successor is already on the closed list ignore it.
                    if (closedList.find(np) != closedList.end()) continue;

                    std::cout << "Check path from node " << olistData.xml_node->first_attribute("id")->value() 
                            << " to node " << np->first_attribute("id")->value() << std::endl;
                    std::cout << "Distance: " << std::to_string(distance_node_to_node(np, dest_node)) << std::endl;
                    std::cout << "Factor: " << calc_cost_factor(nd) << std::endl;


                    NODE n;
                    n.xml_node = np;
                    n.parent_xml_node = olistData.xml_node;
                    n.g = olistData.g + distance_node_to_node(np, dest_node) * calc_cost_factor(nd);
                    n.h = distance_node_to_node(np, dest_node);
                    n.f = n.g + n.h;

                    openList.push(n);

                    break;
                }
            }
        }
        closedList.insert(std::make_pair(olistData.xml_node, olistData.parent_xml_node));
    }

    if (!foundDest)
    {
        std::cout << "Failed to compute path to destination.\n";
        return 1;
    }
    return 0;
    // TODO write nodes to file
    

}

rapidxml::xml_node<char>*
A_Star::get_node_from_lat_lon(rapidxml::xml_node<>* root, const double lat, const double lon)
{
    rapidxml::xml_node<char> *nnode = nullptr;
    double lastDist = 10000.0;

    for (rapidxml::xml_node<>* node = root->first_node("node"); node; node = node->next_sibling("node"))
    {
        double lat_ = std::stod(node->first_attribute("lat")->value(), NULL);
        double lon_ = std::stod(node->first_attribute("lon")->value(), NULL);

        double lastDist_;
        if((lastDist_ = std::sqrt(std::pow(lat_ - lat, 2) + std::pow(lon_ - lon, 2))) < lastDist )
        {
            lastDist = lastDist_;
            
            nnode = node;
        }
    }

    if (lastDist == 10000.0)
    {
        //RCLCPP_INFO(this->get_logger(), "Could not find node at position %.7f, %.7f", lat, lon);
        return nullptr;
    }
    return nnode;
}

bool
A_Star::isDestination(rapidxml::xml_node<char> *pos, rapidxml::xml_node<char> *dest)
{
    //RCLCPP_DEBUG(this->get_logger(),"Check if node with id %s equals desination node (id: %s).",
    //    pos->first_attribute("id")->value(),
    //    dest->first_attribute("id")->value());
    return (pos->first_attribute("id")->value() == dest->first_attribute("id")->value());
}

double
A_Star::distance_node_to_node(rapidxml::xml_node<char> *start, rapidxml::xml_node<char> *dest)
{
    double start_lat = strtod(start->first_attribute("lat")->value(), NULL);
    double start_lon = strtod(start->first_attribute("lon")->value(), NULL);
    double dest_lat = strtod(dest->first_attribute("lat")->value(), NULL);
    double dest_lon = strtod(dest->first_attribute("lon")->value(), NULL);

    return std::sqrt( std::pow(start_lat - dest_lat, 2.0) + std::pow(start_lon - dest_lon, 2.0) );
}

double
A_Star::calc_cost_factor(rapidxml::xml_node<char> *node)
{
    // gehe durch alle child nodes 
    // -> get name and value
    double f = 0;
    for (rapidxml::xml_node<> *n = node->first_node(); n; n = n->next_sibling())
    {
        f += users->calculate_factor(n->name(), n->value());
    }
    return f <= 0 ? 1E6 : f;
}

void
A_Star::trace_path(std::unordered_map<rapidxml::xml_node<char> *, rapidxml::xml_node<char> *> closedList,
        rapidxml::xml_node<char> * dest)
{
    if (!waypoints.empty()) waypoints.clear();

    rapidxml::xml_node<char> *pid, *id;
    std::stack<rapidxml::xml_node<char> *> path;
    pid = dest;

    while (pid != NULL)
    {
        id = pid;
        path.push(id);
        pid = closedList.at(id);
    }

    //RCLCPP_INFO(this->get_logger(), "Computed path:");

    double lat_, lon_;
    POSE pose;
    rapidxml::xml_node<char> *nextnode = path.top();
    pose.lat = strtod(nextnode->first_attribute("lat")->value(), NULL);
    pose.lon = strtod(nextnode->first_attribute("lon")->value(), NULL);
    path.pop();

    while (!path.empty())
    {   
        rapidxml::xml_node<char> *p = path.top();
        lat_ = strtod(p->first_attribute("lat")->value(), NULL);
        lon_ = strtod(p->first_attribute("lon")->value(), NULL);
        path.pop();
        // Calculate angle from nextnode to p
        // tan = G/A = y/x = lat/lon
        // tan 

        if (pose.lon == 0 && pose.lat == 0 )
        {
            pose.angle = 0.0;   // undefined
        } else {
            pose.angle = atan2(lat_ - pose.lat, lon_ - pose.lon);
        }

        //RCLCPP_INFO(this->get_logger(), "Added waypoint with lat: %.8f, lon: %.8f, angle: %f.",
        //       pose.lat, pose.lon, pose.angle);
        waypoints.push_back(pose);
        std::cout << "Waypoint id: " << p->first_attribute("id")->value() << std::endl;

        pose.lat = lat_;
        pose.lon = lon_;
    }

    waypoints.push_back(pose);
}

}   // namespace nav_sgd

#endif