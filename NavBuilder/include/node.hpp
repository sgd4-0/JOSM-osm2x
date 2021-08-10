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
//#define LAT_DEG 1/111000
#define LON_DEG 1.5166E-5

namespace nav_sgd
{
class Node
{

struct ref
{
    //Node* start_node;
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

	long get_id();
	void add_sgd_id(long sgd_id);
	bool add_tag(std::string key, std::string value);

	// manage ways
	void add_way(Node* ziel, Way way);
	void remove_way(long id);
	Way get_way(Node* ziel);
	std::vector<Way> get_ways();

	std::vector<long> get_neighbor_ids();

	// geometric functions
	double distance_to_node(Node* ziel);
	double distance_to_node_m(Node* ziel);
	double angle_to_node(Node* ziel);
	std::pair<double, double> generate_position(double distance, double angle);

	//! \brief Calculate new nodes for this node based on previously added ways. The created nodes
	//! are connected to the corresponding reference nodes and to each other if more than 2 references
	//! are available.
	void calc_child_nodes();
	std::vector<Node*> get_child_nodes();

	//! \brief Returns the child nodes that belong to the specified reference node.
	//! \param ref_id id of reference node
	//! \return Vector containing nodes.
	std::vector<Node*> get_child_nodes(long ref_id);
	std::string to_string(bool to_nav = true);
	std::string to_osm_way(long id);

	std::unordered_map<std::string, std::string> tags;
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

	std::vector<Node> child_nodes;
	std::unordered_map<long, Way> ways;	// ziel id, way

	// Helper function
	std::string to_string( double d ) {
        std::ostringstream stm ;
        stm << std::setprecision(10) << d ;
        return stm.str() ;
    }
};

bool operator==(const Node& A, const Node& B)
{
	return typeid(A) == typeid(B) && A.sgd_id_ == B.sgd_id_;
}

Node::Node(long id, double lat, double lon)
        : id_(id), lat_(lat), lon_(lon)
{
	sgd_id_ = id;
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
        tags.insert(std::make_pair(key, value));
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
	remove_way(ziel->get_id());
	
	ways.insert(std::make_pair(ziel->get_id(), way));

	// angle to node
    refs.insert(ref(ziel, angle_to_node(ziel)));
}

void
Node::remove_way(long id)
{
	if (ways.count(id))
	{
		ways.erase(id);
	}
	for (auto r : refs)
	{
		if (r.end_node->get_id() == id)
		{
			refs.erase(r);
			break;
		}
	}
}

Way
Node::get_way(Node* ziel)
{
	if (ways.count(ziel->get_id()) < 1)
	{
		return Way();
	}
	else
	{
		return ways.find(ziel->get_id())->second;
	}
}

std::vector<Way>
Node::get_ways()
{
	std::vector<Way> w;
	for (auto w_ : ways)
	{
		w.push_back(w_.second);
	}
	return w;
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

double
Node::distance_to_node_m(Node* ziel)
{
	// m -> lat => 1/111000*m
	// m -> lon => 1/(111000*cos(lon))*m
	double diff_lat = (ziel->lat_ - lat_) * 111000;
	double diff_lon = (ziel->lon_ - lon_) * 1/LON_DEG;

	return sqrt(pow(diff_lat, 2) + pow(diff_lon, 2));
}

double
Node::angle_to_node(Node* ziel)
{
	return atan2(ziel->lat_ - lat_,(ziel->lon_ - lon_) * cos(ziel->lat_/180*PI));
}

std::pair<double, double>
Node::generate_position(double distance, double angle)
{
	return std::make_pair(lat_ + sin(angle) * 1/111319.49 * distance,
					lon_ + cos(angle) * 1/(111319.49*cos(lat_/180*PI)) * distance);
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
	Node* last_node = nullptr;
	Way w1;
    ref last_ref(nullptr,0);
	long new_id = sgd_id_;
    for (auto r : refs)
    {
		// Berechnung des Winkels
        if (last_ref.end_node != nullptr)
        {
            double a = (last_ref.angle + r.angle)/2;
			// get way width
			w1 = ways.find(r.end_node->sgd_id_)->second;
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
			auto latlon = generate_position(dist_to_node, a);
            Node n(new_id, latlon.first, latlon.second);
            
			// add references to new node, only id is required
			if (refs.size() > 3 && child_nodes.size() > 0)
			{
				Node* last_node = &child_nodes.back();
				n.add_way(last_node, w1);
				last_node->add_way(&n, w1);
			}
			
			child_nodes.push_back(n);
			node_mapping.push_back(std::make_pair(r.end_node->sgd_id_, new_id));
			node_mapping.push_back(std::make_pair(last_ref.end_node->sgd_id_, new_id));
        }
        last_ref = r;
    }
	// add way from first to last child node
	if (child_nodes.size() > 2)
	{
		Node* n1 = &*child_nodes.begin();
		Node* n2 = &child_nodes.back();
		n1->add_way(n2, w1);
		n2->add_way(n1, w1);
	}
}

std::string
Node::to_string(bool to_nav)
{
	if (tags.size() <= 0 && ways.size() <= 0) return "";
	
	std::string out("  <node ");
	out.append("id='" + std::to_string(sgd_id_) + "' ");
	if (!to_nav) out.append("version='1' ");
	out.append("lat='" + to_string(lat_) + "' ");
	out.append("lon='" + to_string(lon_) + "' ");
	if (tags.size() > 0 || (ways.size() > 0))
	{
		out.append(">\n");
	}
	else
	{
		out.append("/>\n");
		return out;
	}

	for (auto t : tags)
	{
		if (to_nav)
		{
			out.append("    <" + t.first + ">" + t.second + "</" + t.first + ">\n");
		}
		else
		{
			out.append("    <tag k='" + t.first + "' v='" + t.second + "' />\n");
		}
	}

	if (ways.size() > 0 && to_nav)
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
Node::to_osm_way(long id)
{
	std::string out("");

	for (auto w : ways)
	{
		out.append("  <way id='" + std::to_string(id++) + "' version='1'>\n");
		out.append("    <nd ref='" + std::to_string(sgd_id_) + "' />\n");
		out.append("    <nd ref='" + std::to_string(w.first) + "' />\n");
		out.append("    <tag k='highway' v='" + w.second.get_tag("highway") + "' />\n");
		out.append("    <tag k='surface' v='" + w.second.get_tag("surface") + "' />\n");
		out.append("  </way>\n");
		//out.append(w.second.to_string("    "));
	}

	return out;
}

}

#endif
