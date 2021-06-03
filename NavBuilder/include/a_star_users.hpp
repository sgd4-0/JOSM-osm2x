#ifndef NAV_SGD_A_STAR_USERS_HPP_
#define NAV_SGD_A_STAR_USERS_HPP_

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "rapidxml-1.13/rapidxml.hpp"

namespace nav_sgd
{

class A_Star_Users
{

struct USER
{
    std::string name;
    std::unordered_map<std::string, double> factors;
    void add_factor(std::string key, std::string value)
    {
        try
        {
            double d = std::stod(value);
            factors.insert(std::pair<std::string, double>(key, d));
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Could not add factor to user " << name << std::endl;
            std::cout << "Key: " << key << ", value: " << value << std::endl;
            //std::cerr << e.what() << '\n';
        }
    }
    double get_factor(std::string key)
    {
        if (factors.count(key) > 0) {
            return factors[key];
        } else if (factors.count("default") > 0) {
            return factors["default"];
        } else {
            return -1.0;
        }
    }
};

private:
    const std::string users_file_;
    std::unordered_map<std::string, USER> users;
    USER default_user;
    USER current_user;

    USER read_user(rapidxml::xml_node<>* user);
public:
    A_Star_Users(std::string users_file);
    ~A_Star_Users();

    //! \brief Set current user to username. If username is not a valid user set default user.
    //! \param username The username of the user
    void set_user(std::string username);
    //! \brief Calculate factor for given key
    //! \param key
    //! \param value
    //! \returns 
    double calculate_factor(std::string key, std::string value);
};

A_Star_Users::A_Star_Users(std::string users_file)
    : users_file_(users_file)
{
    // read xml
    std::ifstream t(users_file_);

    auto buffer = std::vector<char>((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    rapidxml::xml_document<> doc;
    doc.parse<0>(&buffer[0]);
    auto root = doc.first_node("users");

    // go through xml and add users
    bool has_default_user = false;
    for (rapidxml::xml_node<> *np = root->first_node("user"); np; np = np->next_sibling())
    {
        // new user
        USER user = read_user(np);
        if (user.name.compare("default") == 0)
        {
            has_default_user = true;
            default_user = user;
        }
        else
        {
            users.insert(std::pair<std::string, USER>(user.name, user));
        }
    }
    if (!has_default_user)
    {
        std::cerr << "No default user found!\n";   // TODO error handling
    }
    std::cout << "Users initialized.\n";
}

A_Star_Users::~A_Star_Users()
{
    // destructor
}

A_Star_Users::USER
A_Star_Users::read_user(rapidxml::xml_node<>* user)
{
    USER u;
    u.name = user->first_attribute("name")->value();

    for (rapidxml::xml_node<> *n = user->first_node(); n; n = n->next_sibling())
    {
        //if (n->first_node() != nullptr)
        //{
            std::string prefix = n->name();
            // gehe durch alle child nodes
            for (rapidxml::xml_node<> *nn = n->first_node(); nn; nn = nn->next_sibling())
            {
                if (nn->name_size() > 1)
                {
                    u.add_factor(prefix + ":" + nn->name(), nn->value());
                }
                else
                {
                    u.add_factor(n->name(), n->value());
                }
                
            }
        //}
    }
    std::cout << "Successfully read user " << u.name << std::endl;
    return u;
}

double
A_Star_Users::calculate_factor(std::string key, std::string value)
{
    double v = 1.0;
    try
    {
        v = stod(value);
    }
    catch(const std::invalid_argument& e)
    {
        // value is string
        key = key + ":" + value;
    }

    // try to get factor 
    double f = current_user.get_factor(key) * v;
    if (f >= 0) return f;
    
    f = default_user.get_factor(key) * v;
    return f < 0 ? 1E6 : f;
}

void
A_Star_Users::set_user(std::string username)
{
    current_user = users.count(username) > 0 ? users[username] : default_user;
}

} // namespace nav_sgd


#endif