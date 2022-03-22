#ifndef _WRAPPED_HPP
#define _WRAPPED_HPP

#include <optional>
#include <sys/stat.h>

using stat_t = struct stat;

std::optional<stat_t> Stat(const std::string& path);

#endif // !_WRAPPED_HPP
