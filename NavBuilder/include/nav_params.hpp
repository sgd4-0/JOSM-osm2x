#ifndef NAV_SGD_START_ARGS_HPP_
#define NAV_SGD_START_ARGS_HPP_

#include <unordered_map>
#include <iostream>
#include <fstream>


namespace nav_sgd
{

class Nav_Params
{
private:
    std::string params_filename_;
    std::unordered_map<std::string, std::string> params;
public:
    Nav_Params(std::string params_filename);
    ~Nav_Params();

    void reload();

    bool param_as_bool(std::string key);
    int param_as_int(std::string key);
    double param_as_double(std::string key);
    std::string param_as_string(std::string key);
};

Nav_Params::Nav_Params(std::string params_filename)
{
    // parse file
    params_filename_ = params_filename;

    reload();
}

Nav_Params::~Nav_Params()
{
}

void
Nav_Params::reload()
{
    std::ifstream params_file(params_filename_);
    std::string line;
    params.clear();

    while (getline(params_file, line))
    {
        if (*line.begin() == '#' || line.size() < 3)    continue;

        std::string key = line.substr(0, line.find('='));
        std::string value = line.substr(line.find('=') + 1);
        params.insert(std::make_pair(key, value));
    }
    params_file.close();
}

bool
Nav_Params::param_as_bool(std::string key)
{
    return (param_as_string(key) == "True");
}
int
Nav_Params::param_as_int(std::string key)
{
    try
    {
        return std::stoi(param_as_string(key));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}

double
Nav_Params::param_as_double(std::string key)
{
    try
    {
        return std::stod(param_as_string(key));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0.0;
}

std::string
Nav_Params::param_as_string(std::string key)
{
    if (params.count(key) > 0)
    {
        return params.at(key);
    }
    else
    {
        return "";
    }
}


}



#endif