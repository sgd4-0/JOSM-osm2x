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
#include <vector>
#include <unordered_map>
#include <stdio.h>

#include "include/rapidxml-1.13/rapidxml.hpp"

// Einbinden von erzeugten Header-Dateien
#include "include/node.hpp"
#include "include/way.hpp"
#include "include/address.hpp"

#include "include/mesher.hpp"

#include "include/Filtern.hpp"
#include "include/aStern.hpp"
#include "include/a_star.hpp"

namespace nav_sgd
{

int osm_to_nav(std::string osm_filename)
{
    // read osm file
    rapidxml::xml_document<> doc;
    std::cout << "Parse osm file " << osm_filename << std::endl;
    std::ifstream file(osm_filename);
    std::vector<char> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    buffer.push_back('\0');
    doc.parse<0>(&buffer[0]);
    rapidxml::xml_node<> *root_node = doc.first_node("osm");
    
    long sgd_id = 100000000;
    std::unordered_map<long, Node> nodes;
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
            long first = stol(nd->first_attribute("ref")->value());
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

    // Mesh nodes
    // 1. calculate positions of new nodes
    // 2. connect new nodes to old nodes

    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        it->second.calc_child_nodes();
    }

    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        Node* pNode = &(it->second);
        std::cout << "Node id " << pNode->get_id() << "\n";
        for (auto i : pNode->get_neighbor_ids())
        {
            if (nodes.count(i) < 1)
            {
                continue;
            }
            auto rNode = &(nodes.find(i)->second);
            std::cout << "Ref node id " << rNode->get_id() << "\n";
            auto pNode_cn = pNode->get_child_nodes(i);
            auto rNode_cn = rNode->get_child_nodes(pNode->get_id());
            Way w = pNode->get_way(rNode);

            for (uint8_t k = 0; k < pNode_cn.size(); k++)
            {
                // pNode_cn[0] hat fast immer id 100012601
                std::cout << "Connect cn " << pNode_cn[k]->get_id() << "\n";
                pNode_cn[k]->add_way(rNode, w);    // TODO Unterscheidung rechts/links
                rNode->add_way(pNode_cn[k], w);
            }

            if (rNode_cn.size() < 1 || pNode_cn.size() < 1) continue;
            // calculate distance to other child nodes
            // connect to nearest child node
            std::cout << "Calculate distance\n";
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

    std::cout << "Write nodes to file\n";
    ofstream nav_file("../Karten/3_lohmuehlenpark.nav", std::ios::out | std::ios::trunc);
    nav_file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    nav_file << "<nodelist name=\"lohmuehlenpark\" version=\"0.1\">\n";

    ofstream osm_file("../Karten/3_lohmuehlenpark.osm", std::ios::out | std::ios::trunc);
    osm_file << "<?xml version='1.0' encoding='utf-8'?>\n";
    osm_file << "<osm version='0.6'>\n";

    int i = 0;
    for (auto p : nodes)
    {
        //std::cout << "Node " << p.first << " has " << p.second.get_child_nodes().size() << " child nodes\n";
        std::cout << "Write node " << ++i << "/" << nodes.size() << "\r";
        nav_file << p.second.to_string();
        osm_file << p.second.to_osm();

        int k = 0;
        for (auto cn : p.second.get_child_nodes())
        {
            nav_file << cn->to_string();
            osm_file << cn->to_osm();
        }
    }

    nav_file << "</nodelist>";
    nav_file.close();
    doc.clear();

    // add ways to osm file
    long id = 1000000;
    for (auto p : nodes)
    {
        id += 10;
        osm_file << p.second.to_osm_way(id);

        int k = 0;
        for (auto cn : p.second.get_child_nodes())
        {
            id += 10;
            osm_file << cn->to_osm_way(id);
        }
    }

    osm_file << "</osm>";
    osm_file.close();

    std::cout << "\nWrite addresses to file\n";
    ofstream addr_file("../Karten/3_lohmuehlenpark.adr", std::ios::out | std::ios::trunc);
    addr_file << "# Address list\n";
    for (auto a : addresses)
    {
        addr_file << a.to_string();
    }

    addr_file.close();

    std::cout << "Done!\n";
    return 0;
}

}

// Hauptprogramm
int main(int argc, char *argv[]) {

    int s = 0;
    float StartZielKoords[4] = { 0.0, 0.0, 0.0, 0.0 };

    // Einbinden der Dateipfade der Quell-, Ziel- und Navigationsdatei
    string ursprungsDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\01_Lohmuehlenpark_augmentiert_way.osm";
    string gefiltertDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\10_FilterErgebnis.osm";
    string navigationsfaehigeDatei = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\20_NavigationsFaehigeDaten.osm";
    string endDateiOSM = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\30_EndNavigationOSM.osm";
    string endDateiNavigation = "C:\\Users\\Aike\\Desktop\\Uni\\Semester 7\\VSCodeProjekte\\SharedGuideDog\\31_EndNavigation.osm";

    // Abfrage welche Programmteile aufgerufen werden sollen
    //std::cout << "Bitte geben Sie fuer das komplette Ausfuehren der Datei eine 0 ein oder 1, um nur die Navigationsdatei zu erstellen:" << "\n";
    //std::cin >> s;

    // Erstellen neuer Filterdatei und erster navigationsf�higer Datei
    if (s == 0) {
        //filtern(ursprungsDatei, gefiltertDatei);
        //navigationDatWay(gefiltertDatei, navigationsfaehigeDatei);
        s = 1;
    }

    // Durchlaufen der ersten navigationsf�higen Datei mit dem A*-Algorithmus und erstellen der geforderten Dateien
    //if (s == 1) {

        // Abfrage der Start- und Zielkoordinaten
        //std::cout << "Bitte geben Sie die Start und Ziel Koordinaten ein. Benutzen Sie bitte die folgende Reihenfolge: Startlaenge, Startbreite, Ziellaenge, Zielbreite" << "\n";
        //for (int i = 0; i < 4; i++) {
        //    cin >> StartZielKoords[i];
        //}

        //aStern(navigationsfaehigeDatei, StartZielKoords, gefiltertDatei, endDateiNavigation, endDateiOSM);
    //}


    // ##### ab hier gehts los #####
    // TODO read properties
    nav_sgd::osm_to_nav("../Karten/2_augmentiert.osm");

    //nav_sgd::Nav_Mesher mesher("../Karten/3_lohmuehlenpark.nav");
    //mesher.mesh();

    //nav_sgd::A_Star astern("../Karten/3_lohmuehlenpark.nav", "../Karten/0_users.xml");
    // from node 5126614335 to node 6043758127
    // lat="53.5565441" lon="10.0204562"
    // lat="53.5564935" lon="10.0205392"
    //astern.compute_path(53.5570711, 10.0205999, 53.5560435, 10.0221197);

    return 0;
}