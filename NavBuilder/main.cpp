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

using namespace std::chrono_literals;

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
