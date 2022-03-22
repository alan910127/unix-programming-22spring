#include "wrapped.hpp"

std::optional<stat_t> Stat(const std::string& path) {
    stat_t st;
    int ret{ stat(path.c_str(), &st) };
    if (ret < 0) return std::nullopt;
    return st;
}
