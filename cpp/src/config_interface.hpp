#ifndef __CONFIG_INTERFACE_HPP__
#define __CONFIG_INTERFACE_HPP__

#include <vector>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

typedef std::unordered_map<std::string, std::string> parameter_map;
typedef std::vector<parameter_map> config_list;

#endif
