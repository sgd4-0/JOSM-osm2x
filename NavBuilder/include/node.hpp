#ifndef NAV_NODE_HPP_
#define NAV_NODE_HPP_

#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <math.h>
#include <limits>
#include <sstream>
#include <iomanip>

#include "way.hpp"

#define PI 3.14159265
#define LAT_DEG 9.009E-6
#define LON_DEG 1.5166E-5

namespace nav_sgd
{
class Node
{

struct ref
{
    Node* start_node;
	Node* end_node;
    double angle;
    ref(Node* end_node_, double angle_) : end_node(end_node_), angle(angle_) {};
};

struct cmp_ref
{
    bool operator()(const ref& a, const ref& b) { return a.angle < b.angle; };
};

struct cmp_abs
{
    bool operator()(const ref& a, const ref& b) { return abs(a.angle) < abs(b.angle); };
};

public:
	Node(long id, double lat, double lon);
	~Node();

	void add_sgd_id(long sgd_id);
	bool add_tag(std::string key, std::string value);
	void add_way(Node* ziel, Way way);
	long get_id();
	Way get_way(Node* ziel);

	std::vector<long> get_neighbor_ids();
	double distance_to_node(Node* ziel);

	//! \brief Calculate new nodes for this node based on previously added ways. The created nodes
	//! are connected to the corresponding reference nodes and to each other if more than 2 references
	//! are available.
	//! \returns A vector containing the new nodes
	void calc_child_nodes();
	void connect_to_node(Node* ziel);
	std::vector<Node*> get_child_nodes();

	//! \brief Returns the child nodes belonging to the specified reference node.
	//! \param ref_id id of reference node
	//! \return Vector containing nodes.
	std::vector<Node*> get_child_nodes(long ref_id);
	std::string to_string();
	std::string to_osm();
	std::string to_osm_way(long id);

	std::vector<std::string> tags;
	std::set<std::string> node_tags{"curb", "barrier"};
	std::set<std::string> addr_tags{"addr:street", "addr:housenumber"};
protected:
	friend bool operator==(const Node&, const Node&);
private:
	const long id_;
	long sgd_id_;
	const double lat_, lon_;

	std::set<ref, cmp_ref> refs;
	std::vector<std::pair<long, long>> node_mapping;	// neighbor node, child node

	// Helper function
	std::string to_string( double d ) {
        std::ostringstream stm ;
        stm << std::setprecision(10) << d ;
        return stm.str() ;
    }

	std::vector<Node> child_nodes;
	std::unordered_map<long, Way> ways;	// ziel id, way
};

bool operator==(const Node& A, const Node& B)
{
	return typeid(A) == typeid(B) && A.sgd_id_ == B.sgd_id_;
}

Node::Node(long id, double lat, double lon)
        : id_(id), lat_(lat), lon_(lon)
{
}

Node::~Node()
{
    // destruct
}

long
Node::get_id()
{
	return sgd_id_;
}

void
Node::add_sgd_id(long sgd_id)
{
	sgd_id_ = sgd_id;
}

bool
Node::add_tag(std::string key, std::string value)
{
    // if tag is for adress -> save as adress
	// if tag is for node -> save in node
    if (node_tags.count(key) > 0)
    {
        std::string s("<" + key + ">");
        s.append(value);
        s.append("</" + key + ">\n");

        tags.push_back(s);
        return true;
    }
	
    return false;
}

void
Node::add_way(Node* ziel, Way way)
{
	if (ziel == nullptr)
	{
		std::cout << "Could not add way. End node is nullptr.\n";
		return;
	}
	ways.insert(std::make_pair(ziel->sgd_id_, way));

	// angle to node
	double angle = atan2(ziel->lat_ - lat_,(ziel->lon_ - lon_) * cos(ziel->lat_/180*PI));
    refs.insert(ref(ziel, angle));
}

Way
Node::get_way(Node* ziel)
{
	if (ways.count(ziel->sgd_id_) < 1)
	{
		return Way();
	}
	else
	{
		return ways.find(ziel->sgd_id_)->second;
	}
}

std::vector<long>
Node::get_neighbor_ids()
{
	std::vector<long> ids;
	for (auto w : ways)
	{
		ids.push_back(w.first);
	}
	return ids;
}

double
Node::distance_to_node(Node* ziel)
{
	return sqrt(pow(ziel->lat_ - lat_, 2) + pow(ziel->lon_ - lon_, 2));
}

std::vector<Node*>
Node::get_child_nodes()
{
	std::vector<Node*> cn;
	for (auto it = child_nodes.begin(); it != child_nodes.end(); it++)
	{
		cn.push_back(&*it);
	}
	return cn;
}

std::vector<Node*>
Node::get_child_nodes(long ref_id)
{
	std::vector<Node*> c;
	for (auto it = node_mapping.begin(); it != node_mapping.end(); it++)
	{
		if (it->first == ref_id)
		{	
			for (auto cit = child_nodes.begin(); cit != child_nodes.end(); cit++)
			{
				if (cit->sgd_id_ == it->second)
				{
					std::cout << "Node " << sgd_id_ << " has child node " << cit->sgd_id_ << "\n";
					c.push_back(&*cit);
				}
			}
		}
	}
	return c;
}

void
Node::calc_child_nodes()
{
	if (refs.size() < 2)
    {
        refs.clear();
    }
    else
    {
        // get first ref and add to end
        double a = refs.begin()->angle + 2*PI;
        refs.insert(ref(refs.begin()->end_node, a));
    }

    // Sortiere refs nach absolutem Winkel
    // Winkel zu ref-1 und ref+1 -> zwei Winkel -> gut
    ref last_ref(nullptr,0);
	long new_id = sgd_id_;
    for (auto r : refs)
    {
		// Berechnung des Winkels
        if (last_ref.end_node != nullptr)
        {
            double a = (last_ref.angle + r.angle)/2;
			// get way width
			auto w1 = ways.find(r.end_node->sgd_id_)->second;
			auto w2 = ways.find(last_ref.end_node->sgd_id_)->second;
			
			double width1 = 1;
			double width2 = 1;

			try
			{
				width1 = stod(w1.get_tag("width"));
				width2 = stod(w2.get_tag("width"));
			}
			catch(const std::exception& e)
			{
				std::cerr << "Could not get width from way " << r.end_node->sgd_id_ 
						<< " or " << last_ref.end_node->sgd_id_ << std::endl;
			}
			
			double dist_to_node = (width1 + width2) / 6;

			new_id++;
            Node n(new_id, lat_ + sin(a) * LAT_DEG * dist_to_node,
					lon_ + cos(a) * LON_DEG * dist_to_node);
			n.add_sgd_id(new_id);
            
			// add references to new node, only id is required
            //n.add_way(r.id, 0.0, 0.0, w1);
            //n.add_way(last_ref.id, 0.0, 0.0, w2);
			child_nodes.push_back(n);
			node_mapping.push_back(std::make_pair(r.end_node->sgd_id_, new_id));
			node_mapping.push_back(std::make_pair(last_ref.end_node->sgd_id_, new_id));
        }

        last_ref = r;
    }
	// std::cout << "Node " << id_ << " has " << child_nodes.size() << " child nodes\n";
	if (child_nodes.size() > 2)
	{
		// connect new nodes
		Node* last_node = &child_nodes.back();
		for (auto it = child_nodes.begin(); it != child_nodes.end(); it++)
		{
			Way w;
			last_node->add_way(&*it, w);
			it->add_way(last_node, w);

			last_node = &*it;
		}
	}
}

void
Node::connect_to_node(Node* ziel)
{
	






	/*
	// get two shortest routes from this node to ziel
	std::set<ref, cmp_abs> angles_to_ziel;

	double az = atan2(ziel->lat_ - lat_,(ziel->lon_ - lon_) * cos(ziel->lat_/180*PI));	// angle to zielnode

	// minimiere angle und distance
	for (uint8_t i = 0; i < ziel->child_nodes.size(); i++)
	{
		double acn1 = atan2(ziel->child_nodes[i].lat_ - lat_,
			(ziel->child_nodes[i].lon_ - lon_) * cos(ziel->lat_/180*PI));	// angle to childnode
		double acn2 = acn1 > 0 ? acn1 - 2*PI : acn1 + 2*PI;		// angle to child node with switched sign

		double dcn1 = 10000*sqrt(pow(ziel->child_nodes[i].lat_ - lat_,2) + pow(ziel->child_nodes[i].lon_ - lon_,2));
		std::cout << "Abstand: " << to_string(dcn1) << "\n";
		angles_to_ziel.insert(ref(&(ziel->child_nodes[i]), acn1 - az < 0 ? acn1 - az - dcn1 : acn1 - az + dcn1));
		angles_to_ziel.insert(ref(&(ziel->child_nodes[i]), acn2 - az < 0 ? acn2 - az - dcn1 : acn2 - az + dcn1));
	}

	auto it = angles_to_ziel.begin();
	Node* right_node = nullptr;
	Node* left_node = nullptr;

	for (auto it = angles_to_ziel.begin(); it != angles_to_ziel.end(); it++)
	{
		//std::cout << "Angles to ziel: " << it->angle << "\n";
		if (it->angle < 0 && right_node == nullptr)
		{
			right_node = it->end_node;
		}
		else if (left_node == nullptr)	// angle >= 0
		{
			left_node = it->end_node;
		}
	}
	//std::cout << "---- \n";
	if (right_node == nullptr || left_node == nullptr)
	{
		std::cout << "One node is nullptr\n";
		return;
	}
	// get way from this node to ziel
	auto way_to_ziel = ways.find(ziel->sgd_id_)->second;
	
	// add way from this node to ziel child nodes
	add_way(left_node, way_to_ziel);
	add_way(right_node, way_to_ziel);

	std::map<double, Node*> cn_dist_l;
	std::map<double, Node*> cn_dist_r;
	for (uint8_t i = 0; i < child_nodes.size(); i++)
	{
		double dist_l = sqrt(pow(child_nodes[i].lat_ - left_node->lat_,2) + pow(child_nodes[i].lon_ - left_node->lon_,2));
		double dist_r = sqrt(pow(child_nodes[i].lat_ - right_node->lat_,2) + pow(child_nodes[i].lon_ - right_node->lon_,2));

		if (dist_l < dist_r)
		{
			cn_dist_l.insert(std::make_pair(dist_l, &child_nodes[i]));
		}
		else
		{
			cn_dist_r.insert(std::make_pair(dist_r, &child_nodes[i]));
		}
	}
	
	Node* right_child_node = cn_dist_r[0];
	Node* left_child_node = cn_dist_l[0];

	// add way from child node to ziel
	if (right_child_node != nullptr)
	{
		right_child_node->add_way(right_node, way_to_ziel);
		right_child_node->add_way(ziel, way_to_ziel);
	}
	if (left_child_node != nullptr)
	{
		Way w = way_to_ziel;
		double hazard = 1.0;
		try
		{
			hazard = stod(w.get_tag("hazard"));
		}
		catch(const std::exception& e)
		{
			// std::cout << "Could not get hazard from way\n";
		}
		w.add_tag("hazard", to_string(hazard * 1.5));
		left_child_node->add_way(left_node, w);
		left_child_node->add_way(ziel, w);
	}*/
}

std::string
Node::to_string()
{
	if (tags.size() <= 0 && ways.size() <= 0) return "";
	
	std::string out("  <node ");
	out.append("id=\"" + std::to_string(sgd_id_) + "\" ");
	out.append("lat=\"" + to_string(lat_) + "\" ");
	out.append("lon=\"" + to_string(lon_) + "\" >\n");
	//if (tags.size() > 0 || ways.size() > 0)	

	if (tags.size() > 0)
	{
		for (auto t : tags)
		{
			out.append("    " + t);
		}
	}

	if (ways.size() > 0)
	{
		for (auto w : ways)
		{
			out.append("    <nd ref=\"" + std::to_string(w.first) + "\">\n");
			out.append(w.second.to_string("      "));
			out.append("    </nd>\n");
		}
	}
	out.append("  </node>\n");

	return out;
}

std::string
Node::to_osm()
{
	std::string out("  <node ");
	out.append("id='" + std::to_string(sgd_id_) + "' ");
	out.append("version='1' ");
	out.append("lat='" + to_string(lat_) + "' ");
	out.append("lon='" + to_string(lon_) + "' />\n");
	return out;
}

std::string
Node::to_osm_way(long id)
{
	std::string out("");

	for (auto w : ways)
	{
		out.append("  <way ");
		out.append("id='" + std::to_string(id++) + "' ");
		out.append("version='1'>\n");
		out.append("    <nd ");
		out.append("ref='" + std::to_string(sgd_id_) + "' />\n");
		out.append("    <nd ");
		out.append("ref='" + std::to_string(w.first) + "' />\n");
		out.append("  </way>\n");
		//out.append(w.second.to_string("    "));
	}

	return out;
}

}

#endif
