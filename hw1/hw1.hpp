#ifndef _HW1_HPP
#define _HW1_HPP

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <regex>
#include <set>
#include <map>

#include "utils.hpp"
#include "wrapped.hpp"
#include "record.hpp"

template<class... Args>
std::string format(const std::string& fmt, Args... args);
void checkArguments(int argc, char** argv);

#endif // !_HW1_HPP
