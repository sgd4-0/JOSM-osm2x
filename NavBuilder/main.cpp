/*
 * File:   main.cpp
 * Author: Aike Banse
 *
 * Created on January 7, 2021, 11:47 AM
 *
 * Dieses Programm liest eine .osm-Datei ein und filtert diese. Die Datei wird zuerst nach dem Tag "SharedGuideDog" durchsucht und schreibt die gefilterten Inhalte
 * in eine zweite .osm-Datei. Nachfolgend werden in den Nodes und Ways mit dem Tag "SharedGuideDog" die Referenzen zu anderen Nodes rausgefiltert und ebenfalls in die
 * Zieldatei geschrieben. Nachdem nun vollst�ndig gefiltert wurde muss die Datei navigierbar gemacht werden und wird daf�r umgeschrieben. Folgend muss die erstellte
 * Datei in eine A*-Funktion eingelesen werden und ein Pfad mit der niedrigsten Gesamtsumme erstellt werden, welcher wieder in einer Datei festgehalten wird.
*/

// Einbinden von bereits vorhandenen Bibliotheken
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <chrono>
#include <thread>

// includes for server
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#define PORT 8080

#include "include/rapidxml-1.13/rapidxml.hpp"

// Einbinden von erzeugten Header-Dateien
#include "include/node.hpp"
#include "include/way.hpp"
#include "include/address.hpp"
//#include "include/nav_start_args.hpp"

#include "include/a_star.hpp"

using namespace std::chrono_literals;

namespace nav_sgd
{

std::string INPUT_FILENAME;
std::string OUTPUT_FILENAME;
std::string USERS_FILENAME;
std::string PROPERTIES_FILENAME;

std::unordered_map<long, Node> nodes;

int osm_to_nav(std::string osm_filename)
{
    // read osm file
    rapidxml::xml_document<> doc;
    std::cout << "Parse osm file " << osm_filename << std::endl;
    std::ifstream file(osm_filename);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    doc.parse<0>(&buffer[0]);
    rapidxml::xml_node<> *root_node = doc.first_node("osm");

    long sgd_id = 100000000;
    std::unordered_map<long, long> id_mapping;
    std::vector<Address> addresses;

    for (rapidxml::xml_node<>* node = root_node->first_node("node");
        node; node = node->next_sibling("node"))
    {
        rapidxml::xml_attribute<>* attr;
        if (attr = node->first_attribute("action"))
        {
            std::string v(attr->value());
            if (!v.compare("delete")) continue;
        }
        // parse and save node
        // Every node has attribute id, lat, lon
        long id;
        double lat, lon;
        try
        {
            id = std::stol(node->first_attribute("id")->value());
            lat = std::stod(node->first_attribute("lat")->value());
            lon = std::stod(node->first_attribute("lon")->value());
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error parsing node\n";
            std::cerr << e.what() << '\n';
            continue;
        }

        // Initialize new node and address
        bool has_adress_tag = false;
        sgd_id += 100;
        Address addr(id);
        addr.add_sgd_id(sgd_id);
        Node n(id, lat, lon);
        n.add_sgd_id(sgd_id);
        id_mapping.insert(std::make_pair(id, sgd_id));

        for (rapidxml::xml_node<>* tag = node->first_node("tag");
            tag; tag = tag->next_sibling("tag"))
        {
            std::string key = tag->first_attribute("k")->value();
            if (key.rfind("addr",0) == 0)
            {
                has_adress_tag = true;
                addr.add_tag(key, tag->first_attribute("v")->value());

            } else {
                n.add_tag(key, tag->first_attribute("v")->value());
            }
        }

        if (has_adress_tag)
        {
            addresses.push_back(addr);
        }

        nodes.insert(std::make_pair(n.get_id(), n));
    }

    std::cout << nodes.size() << " nodes found.\n";

    for (rapidxml::xml_node<>* way = root_node->first_node("way");
        way; way = way->next_sibling("way"))
    {
        rapidxml::xml_attribute<>* attr;
        if (attr = way->first_attribute("action"))
        {
            std::string v(attr->value());
            if (!v.compare("delete")) continue;
        }
        // parse and save way
        // get all tags
        Way w;
        for (rapidxml::xml_node<>* tag = way->first_node("tag");
           tag; tag = tag->next_sibling("tag"))
        {
            w.add_tag(tag->first_attribute("k")->value(), tag->first_attribute("v")->value());
        }

        Node *n1, *n2 = nullptr;
        for (rapidxml::xml_node<>* nd = way->first_node("nd");
           nd; nd = nd->next_sibling("nd"))
        {
            // get node and add way
            long first = std::stol(nd->first_attribute("ref")->value());
            long sid = id_mapping.find(first)->second;
            n1 = &nodes.find(sid)->second;

            if (n2 != nullptr)    // skip first round to get two nodes
            {
                n1->add_way(n2, w);
                n2->add_way(n1, w);
            }
            n2 = n1;
        }
    }
    std::cout << "\nWrite addresses to file\n";
    std::ofstream addr_file("../Karten/3_lohmuehlenpark.adr", std::ios::out | std::ios::trunc);
    addr_file << "# Address list\n";
    for (auto a : addresses)
    {
        addr_file << a.to_string();
    }
    addr_file.close();

    std::cout << "Done!\n";
    return 0;
}

int mesh_nodes()
{
    // Mesh nodes
    // 1. calculate positions of new nodes
    // 2. connect new nodes to old nodes

    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        it->second.calc_child_nodes();
    }

    for (auto nds : nodes)
    {
        Node* pNode = &nodes.at(nds.first);
        for (auto i : pNode->get_neighbor_ids())
        {
            if (nodes.count(i) < 1)
            {
                continue;
            }
            auto rNode = &nodes.at(i);
            auto pNode_cn = pNode->get_child_nodes(i);
            auto rNode_cn = rNode->get_child_nodes(pNode->get_id());

            Way w = pNode->get_way(rNode);
            for (uint8_t k = 0; k < pNode_cn.size(); k++)
            {
                pNode_cn[k]->add_way(rNode, w);
                rNode->add_way(pNode_cn[k], w);
            }

            if (rNode_cn.size() < 1 || pNode_cn.size() < 1) continue;
            // calculate distance to other child nodes
            // connect to nearest child node
            double d1 = pNode_cn[0]->distance_to_node(rNode_cn[0]) + pNode_cn[1]->distance_to_node(rNode_cn[1]);
            double d2 = pNode_cn[0]->distance_to_node(rNode_cn[1]) + pNode_cn[1]->distance_to_node(rNode_cn[0]);

            if (d1 < d2)
            {
                pNode_cn[0]->add_way(rNode_cn[0], w);
                pNode_cn[1]->add_way(rNode_cn[1], w);
            }
            else
            {
                pNode_cn[0]->add_way(rNode_cn[1], w);
                pNode_cn[1]->add_way(rNode_cn[0], w);
            }
        }
    }
    return 0;
}

//! \brief Write nodes to file.
//! \param filename
//! \param file_format one of osm, nav. Defaults to nav
//! \return number of written nodes
int write_to_file(std::string filename, std::string file_format = "nav")
{
    if (nodes.empty())
    {
        std::cout << "Nodelist is empty. No nodes to write.\n";
        return 0;
    }
    std::cout << "Write nodes to file\n";
    std::ofstream out_file(filename + "." + file_format, std::ios::out | std::ios::trunc);
    bool to_nav = (file_format == "nav");

    out_file << "<?xml version='1.0' encoding='utf-8'?>\n";
    out_file << (to_nav ? "<nodelist name='lohmuehlenpark' version='0.1'>\n" : "<osm version='0.6'>\n");

    int i = 0;
    for (auto p : nodes)
    {
        std::cout << "Write node " << ++i << "/" << nodes.size() << "\r";
        out_file << p.second.to_string(to_nav);
        //osm_file << p.second.to_osm();

        int k = 0;
        for (auto cn : p.second.get_child_nodes())
        {
            out_file << cn->to_string(to_nav);
            //osm_file << cn->to_osm();
        }
    }

    // Write ways
    if (!to_nav)
    {
        long id = 1000000;
        for (auto p : nodes)
        {
            id += 10;
            out_file << p.second.to_osm_way(id);

            int k = 0;
            for (auto cn : p.second.get_child_nodes())
            {
                id += 10;
                out_file << cn->to_osm_way(id);
            }
        }
    }

    out_file << (to_nav ? "</nodelist>" : "</osm>");
    out_file.close();
    return i;
}

}

void args_from_msg(std::string msg, nav_sgd::A_Star& a_stern)
{
    double slat = 0.0, slon = 0.0, dlat = 0.0, dlon = 0.0;

    std::cout << "Parse message: -" << msg << "-\n";
    std::string token;
    std::istringstream tokenStream(msg);
    bool is_nav_request_;
    while (std::getline(tokenStream, token, ','))
    {
        //k_v.push_back(token);
        int del_pos = token.find(':');
        int key = stoi(token.substr(0,del_pos));

        switch (key)
        {
        case 0:     // obstacle or nav
            is_nav_request_ = stoi(token.substr(del_pos+1)) < 1;
            break;
        case 1: // username
            {
                auto uname = token.substr(del_pos+1);
                if (uname.size() > 2)
                {
                    a_stern.set_user(uname);
                }
            }
            break;
        case 2: // start lat
            slat = stod(token.substr(del_pos+1));
            break;
        case 3: // start lon
            slon = stod(token.substr(del_pos+1));
            break;
        case 4: // destination lat
            dlat = stod(token.substr(del_pos+1));
            break;
        case 5: // destination lon
            dlon = stod(token.substr(del_pos+1));
            break;
        case 6: // address
            {
                auto adr = token.substr(del_pos+1);
                if (adr.size() > 2)
                {
                    a_stern.set_destination(adr);
                }
            }
            break;
        default:
            break;
        }
    }
    if (is_nav_request_)
    {
        if (slat != 0.0 && slon != 0.0)  a_stern.set_start(slat, slon);
        if (dlat != 0.0 && dlon != 0.0)  a_stern.set_destination(dlat, dlon);
    }
    else
    {
        // add hindernis
        std::cout << "Add hindernis at position " << slat << ", " << slon << "\n";
    }
}

int main(int argc, char *argv[])
{
    nav_sgd::osm_to_nav("../Karten/2_augmentiert.osm");
    nav_sgd::mesh_nodes();
    nav_sgd::write_to_file("../Karten/3_lohmuehlenpark", "nav");
    nav_sgd::write_to_file("../Karten/3_lohmuehlenpark", "osm");

    nav_sgd::A_Star *astern = new nav_sgd::A_Star("../Karten/3_lohmuehlenpark.nav", "../Karten/0_users.xml");

    // create server
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::string msg("Hello from server");
    const char *hello = msg.c_str();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    std::string buffer;
    bool IN_MSG = false;
    while (1)
    {
        auto tp = std::chrono::system_clock::now();
        char c;
        read( new_socket , &c, 1);

        if (c == '{' && !IN_MSG)
        {
            buffer.clear();
            IN_MSG = true;
        }
        else if (IN_MSG && c == '}')
        {
            // end of message -> parse msg and compute path
            args_from_msg(buffer, *astern);
            astern->compute_path();

            std::string message_("{");
            // TODO send waypoints {x.xxx,y.yyy;x.xxx,y.yyy;...}
            for (auto n : astern->waypoints)
            {
                if (message_.size() > 3) message_.append(";");
                message_.append(n.to_string());
            }
            message_.append("}");
            send(new_socket, message_.c_str(), message_.length(), 0);

            message_.clear();
            IN_MSG = false;
        }
        else if (IN_MSG)
        {
            buffer.push_back(c);
        }

        if (!IN_MSG)
        {
            std::this_thread::sleep_until(tp + 100ms);
        }
    }
    return 0;
}
