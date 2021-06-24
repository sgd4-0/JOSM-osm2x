#ifndef NAV_SGD_OSM_TO_NAV_HPP_
#define NAV_SGD_OSM_TO_NAV_HPP_

#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <set>
#include <stdio.h>

#include "rapidxml-1.13/rapidxml.hpp"
#include "node.hpp"
#include "way.hpp"
#include "address.hpp"
#include "nav_params.hpp"

namespace nav_sgd
{

class Osm2Nav
{
private:
    Nav_Params *parameters;
    std::unordered_map<long, Node> nodes;
public:
    //! \brief Constructor for Osm2Nav
    //! \param params A pointer to the parameter class
    Osm2Nav(Nav_Params& params);
    ~Osm2Nav();

    //! \brief Import osm file and parse nodes
    //! \returns the number of parsed nodes
    int osm_to_nav();

    //! \brief If the distance between two nodes is larger than a threshold calulate node in between.
    //! \returns the number of added nodes
    int interpolate_nodes();

    //! \brief Calculate new nodes and add ways to connect the new nodes
    int mesh_nodes();

    //! \brief Write all nodes to a file with the specified filename.
    //! \param filename the name of the file
    //! \param file_format osm or nav, defaults to nav
    //! \returns the number of saved nodes
    int write_to_file(std::string filename, std::string file_format = "nav");
};

Osm2Nav::Osm2Nav(Nav_Params& params)
{
    parameters = &params;
}

Osm2Nav::~Osm2Nav()
{
}


int
Osm2Nav::osm_to_nav()
{
    // read osm file
    std::string osm_input_file = parameters->param_as_string("osm_input_file");
    std::ifstream file(osm_input_file);

    if (file.fail())
    {
        std::cerr << "Could not open osm input file " << osm_input_file << "\n";
        return -1;
    }

    std::cout << "Parse osm file " << osm_input_file << std::endl;
    
    rapidxml::xml_document<> doc;
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
            long sid = id_mapping.at(first);
            n1 = &nodes.at(sid);

            if (n2 != nullptr)    // skip first round to get two nodes
            {
                n1->add_way(n2, w);
                n2->add_way(n1, w);
            }
            n2 = n1;
        }
    }
    std::cout << "Write addresses to file\n";
    std::ofstream addr_file("../Karten/3_lohmuehlenpark.adr", std::ios::out | std::ios::trunc);
    addr_file << "# Address list\n";
    for (auto a : addresses)
    {
        addr_file << a.to_string();
    }
    addr_file.close();

    doc.clear();

    std::cout << "Done!\n";
    return 0;
}

int
Osm2Nav::interpolate_nodes()
{
    // gehe durch alle nodes
    // get all ways
    // calculate distance between nodes
    // if distance > threshold add new node, update ways
    
    // get a list of all node ids
    std::set<long> id_set;
    for (auto n : nodes)
    {
        id_set.insert(n.first);
    }
    long next_id = ceil((*(id_set.rbegin()))/100+0.1)*100;
    std::cout << "Last id " << next_id << "\n";

    for (auto id : id_set)
    {
        // get node with id
        Node* node = &nodes.at(id);
        for (auto nid : node->get_neighbor_ids())
        {
            // if length is larger than threshold
            Node* nn = &nodes.at(nid);
            Node* last_new_node = node;
            Way w = node->get_way(nn);

            double l = node->distance_to_node_m(nn);
            int k = floor(l / parameters->param_as_double("max_node_distance"));    // number of nodes to be added
            for (int i = 1; i <= k; i++)
            {
                // create node: i*Abstand * lat/lon
                auto latlon = node->generate_position(i*l/(k+1), node->angle_to_node(nn));
                nodes.insert(std::make_pair(next_id, Node(next_id, latlon.first, latlon.second)));
                Node *new_node_ = &nodes.at(next_id);

                // update ways
                node->remove_way(nn->get_id());
                nn->remove_way(node->get_id());

                last_new_node->add_way(new_node_, w);
                new_node_->add_way(last_new_node, w);

                if (i == k)
                {
                    nn->add_way(new_node_, w);
                    new_node_->add_way(nn, w);
                }

                last_new_node = new_node_;
                next_id += 100;
            }
        }
    }

    return 0;
}

int
Osm2Nav::mesh_nodes()
{
    // Mesh nodes
    // 1. calculate positions of new nodes
    // 2. connect new nodes to old nodes
    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {   
        // create child nodes only if width of ways is larger than threshold
        bool create_child_nodes = false;
        for (auto w : it->second.get_ways())
        {
            try
            {
                double width = std::stod(w.get_tag("width"));
                if (width > parameters->param_as_double("way_width_thresh"))
                {
                    create_child_nodes = true;
                    break;
                }
            }
            catch(const std::exception& e) { }
        }
        
        if (create_child_nodes)
        {
            it->second.calc_child_nodes();
        }
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

int
Osm2Nav::write_to_file(std::string filename, std::string file_format)
{
    if (nodes.empty())
    {
        std::cout << "Nodelist is empty. No nodes to write.\n";
        return 0;
    }
    std::cout << "Write nodes to file " << filename << "." << file_format << "\n";
    std::ofstream out_file(filename + "." + file_format, std::ios::out | std::ios::trunc);
    bool to_nav = (file_format == "nav");

    out_file << "<?xml version='1.0' encoding='utf-8'?>\n";
    out_file << (to_nav ? "<nodelist name='lohmuehlenpark' version='0.1'>\n" : "<osm version='0.6'>\n");

    int i = 0;
    for (auto p : nodes)
    {
        std::cout << "\rWriting node " << ++i << "/" << nodes.size();
        out_file << p.second.to_string(to_nav);

        int k = 0;
        for (auto cn : p.second.get_child_nodes())
        {
            out_file << cn->to_string(to_nav);
        }
    }
    std::cout << "\n";

    // Write ways (only osm format)
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

#endif