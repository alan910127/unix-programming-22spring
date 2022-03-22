#include "utils.hpp"

std::vector<int> getPIDs() {
    static const std::regex pidRE{ "[0-9]+" };

    DIR* dp{ opendir("/proc") };
    if (dp == nullptr) err_quit("opendir");

    dirent* dir{};
    std::vector<int> pids;
    while ((dir = readdir(dp)) != nullptr) {
        if (std::regex_match(dir->d_name, pidRE)) {
            pids.emplace_back(std::stoi(dir->d_name));
        }
    }

    return pids;
}

char testAccess(const std::string& path) {
    static constexpr char ch[] = { ' ', 'r', 'w', 'u' };
    int index{};

    if (faccessat(0, path.c_str(), R_OK, AT_SYMLINK_NOFOLLOW) == 0) index |= 1;
    if (faccessat(0, path.c_str(), W_OK, AT_SYMLINK_NOFOLLOW) == 0) index |= 2;

    return ch[index];
}
