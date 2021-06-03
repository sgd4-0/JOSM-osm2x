#ifndef NAV_WAY_HPP_
#define NAV_WAY_HPP_

#include <string>
#include <vector>
#include <set>

namespace nav_sgd
{

class Way
{

private:
    // start, ziel
    // way attributes
    //const std::vector<std::string> valid_keys;
    std::unordered_map<std::string, std::string> tags;
    std::set<std::string> valid_keys{"highway", "surface", "width", "slope", "hazard"};

public:
    Way();
    ~Way();

    //! \brief Add a tag to this way. If a tag with the specified key already exists the value is updated.
    //! \param key the key of the tag
    //! \param value the value of the tag as a string
    //! \returns true if key is a valid key and tag could be added
    bool add_tag(std::string key, std::string value);
    std::string get_tag(std::string key);
    std::string to_string(std::string indent);
};

Way::Way()
{
}

Way::~Way()
{
}

bool
Way::add_tag(std::string key, std::string value)
{
    if (valid_keys.count(key) > 0)
    {
        tags.insert(std::make_pair(key, value));
        return true;
    }
    return false;
}

std::string
Way::get_tag(std::string key)
{
    if (tags.count(key) > 0)
    {
        return tags.find(key)->second;
    }
    return "";
}

std::string
Way::to_string(std::string indent)
{
    std::string s;
    for (auto c : tags)
    {
        s.append(indent + "<" + c.first + ">");
        s.append(c.second);
        s.append("</" + c.first + ">\n");
    }
    return s;
}

}

#endif