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

#include "include/nav_params.hpp"
#include "include/osm_to_nav.hpp"
#include "include/a_star.hpp"
#include "include/obstacle.hpp"

#include "include/simdjson.h"
#include "include/simdjson.cpp" // TODO nicht so gut

using namespace std::chrono_literals;

void parse_json(std::string buffer, nav_sgd::A_Star& a_stern)
{
    simdjson::ondemand::parser parser;
    
    simdjson::dom::parser prser;
    simdjson::padded_string json(buffer);
    simdjson::dom::element doc = prser.parse(json);

    // first: parse mode
    int64_t mode = doc["mode"];

    if (mode == 0)
    {
        // parse json for a star
        std::vector<double> start;
        for (double val : doc["start"])     start.push_back(val);

        std::vector<double> dest;
        for (double val : doc["dest"])  dest.push_back(val);

        auto username = simdjson::minify(doc["username"]);
        auto address = simdjson::minify(doc["address"]);

        // remove leading and trailing "
        address = address.substr(1, address.length() - 2);
        username = username.substr(1, username.length() - 2);
        
        a_stern.set_user(username);
        a_stern.set_start(start[0], start[1]);
        if (address.length() > 1)
        {
            a_stern.set_destination(address);
        }
        else
        {
            a_stern.set_destination(dest[0], dest[1]);
        }
    }
    else if (mode == 1)
    {
        // parse json for obstacle
        std::vector<double> pos;
        for (double val : doc["position"])     pos.push_back(val);

        auto username = simdjson::minify(doc["username"]);
        username = username.substr(1, username.length() - 2);

        add_obstacle("../Karten/3_lohmuehlenpark.nav", pos[0], pos[1]);
    }
    else
    {
        remove_obstacle("../Karten/3_lohmuehlenpark.nav");
    }

    // astern->set_arg
    std::cout << "Message parsed\n";
}

int main(int argc, char *argv[])
{
    nav_sgd::Nav_Params *params = new nav_sgd::Nav_Params("parameters.cfg");

    // interpolation

    // osm to nav:
    // - input : osm file
    // - Unterscheidung, was durchgeführt werden soll:
    //      - parse
    //      - interpolation
    //      - mesh
    // - output: nav/osm/adr file
    nav_sgd::Osm2Nav osm2nav(*params);
    int ret = osm2nav.osm_to_nav();
    osm2nav.interpolate_nodes();

    if (params->param_as_bool("mesh_nodes"))
    {
        osm2nav.mesh_nodes();
    }

    if (params->param_as_string("out_file").find("nav") != std::string::npos)
    {
        osm2nav.write_to_file("../Karten/3_lohmuehlenpark", "nav");
    }
    if (params->param_as_string("out_file").find("osm") != std::string::npos)
    {
        osm2nav.write_to_file("../Karten/3_lohmuehlenpark", "osm");
    }
    
    nav_sgd::A_Star *astern = new nav_sgd::A_Star(*params, "../Karten/3_lohmuehlenpark.nav", "../Karten/0_users.xml");

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
            buffer.append("{");
            IN_MSG = true;
        }
        else if (IN_MSG && c == '}')
        {
            // end of message -> parse msg and compute path
            std::cout << "end of message\n";
            buffer.append("}");
            parse_json(buffer, *astern);
            //args_from_msg(buffer, *astern);
            astern->compute_path();

            std::string message_("{");
            // send waypoints {x.xxx,y.yyy;x.xxx,y.yyy;...}
            for (auto n : astern->waypoints)
            {
                if (message_.size() > 3) message_.append(";");
                message_.append(n.to_string());
                std::cout << "WP: " << n.to_string() << "\n";
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
