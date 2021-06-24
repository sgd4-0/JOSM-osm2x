#ifndef NAV_SGD_A_STAR_NODE_HPP_
#define NAV_SGD_A_STAR_NODE_HPP_

#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <cmath>

namespace nav_sgd
{

class A_Star_Node
{
private:
    long parent_id_;
    double g_,h_;

    // Helper function
	std::string to_string(double d) {
        std::ostringstream stm ;
        stm << std::setprecision(10) << d ;
        return stm.str();
    };
    
protected:
	friend bool operator==(const A_Star_Node& A, const A_Star_Node& B);
    bool operator()(const A_Star_Node& A, const A_Star_Node& B);
    friend bool operator<(const A_Star_Node& A, const A_Star_Node& B);
public:
    A_Star_Node(long, double, double);
    ~A_Star_Node();
    
    //! \brief 
    //! \param parent_node
    //! \param dest_node
    std::string set_parent(A_Star_Node, A_Star_Node, double add_cost = 1.0);
    double cost_to_node(long);

    long id;
    double lat, lon;
    
    std::map<long, double> cost_map;  // cost to node with id
    bool is_blocked = false;

    double f();
    double g();
    long parent_id();

    std::string to_string();
};

bool operator==(const A_Star_Node& A, const A_Star_Node& B)
{   
    // typeid(A) == typeid(B)
    return A.id == B.id;
}

bool
A_Star_Node::operator()(const A_Star_Node& A, const A_Star_Node& B)
{
    return A.id == B.id;
}

bool operator<(const A_Star_Node& A, const A_Star_Node& B)
{
    return (A.g_ + A.h_) < (B.g_ + B.h_);
}

A_Star_Node::A_Star_Node(long id_, double lat_, double lon_)
    : id(id_), lat(lat_), lon(lon_)
{
    g_ = 0.0;
    h_ = 0.0;
    parent_id_ = -1;
}

A_Star_Node::~A_Star_Node()
{
}

std::string
A_Star_Node::set_parent(A_Star_Node parent_node, A_Star_Node dest_node, double add_cost)
{
    parent_id_ = parent_node.id;
    // calculate g_ from distance and cost factor
    g_ = parent_node.g_ + parent_node.cost_to_node(id) * add_cost +
                    sqrt(pow(parent_node.lat - lat, 2) + pow(parent_node.lon - lon, 2));
    std::string out(" g: " + to_string(g_));

    // calculate h_ from distance to destination node
    h_ = sqrt(pow(dest_node.lat - lat, 2) + pow(dest_node.lon - lon, 2));
    out.append(", h: " + to_string(h_));
    out.append(", f: " + to_string(f()));
    out.append(", cost: " + to_string(parent_node.cost_to_node(id)) + "\n");
    return out;
}

double
A_Star_Node::f()
{
    return g_ + h_;
}

double
A_Star_Node::g()
{
    return g_;
}

long
A_Star_Node::parent_id()
{
    return parent_id_;
}

double
A_Star_Node::cost_to_node(long id)
{
    return (cost_map.count(id) > 0 ? cost_map.at(id) : 10000);
}

std::string
A_Star_Node::to_string()
{
    return (to_string(lat) + "," + to_string(lon));
}

}

#endif