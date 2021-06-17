#ifndef NAV_ADDRESS_HPP_
#define NAV_ADDRESS_HPP_

#include <string>

namespace nav_sgd
{

class Address
{

private:
    const static uint8_t size_array = 2;
    const std::string valid_keys[size_array] = {"addr:street", "addr:housenumber"};
    std::string address[size_array];
    const long id_;
    long sgd_id_;
    
public:
    Address(long id);
    ~Address();

    void add_sgd_id(long sgd_id);
    bool add_tag(std::string key, std::string value);
    std::string to_string();
};

Address::Address(long id)
    : id_(id)
{
}

Address::~Address()
{
}

void
Address::add_sgd_id(long sgd_id)
{
    sgd_id_ = sgd_id;
}

bool
Address::add_tag(std::string key, std::string value)
{
    //std::cout << "Add key - value " << key << ": " << value << std::endl;
    for (size_t i = 0; i < size_array; i++)
    {
        if (key.compare(valid_keys[i]) == 0)
        {
            address[i] = value;
            return true;
        }
    }
    return false;
}

std::string
Address::to_string()
{
    std::string s;
    for (size_t i = 0; i < size_array; i++)
    {
        if (s.size() > 1) s.append(" ");
        s.append(address[i]);
    }
    s.append(":" + std::to_string(sgd_id_) + "\n");
    return s;
}

}

#endif

