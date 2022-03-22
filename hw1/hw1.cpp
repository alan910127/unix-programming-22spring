#include "hw1.hpp"

bool flgCommandFilter{}, flgTypeFilter{}, flgFilenameFilter{};
std::regex commandRE{}, typeRE{}, filenameRE{};

std::string infoDir[]{ "cwd", "root", "exe" };

std::map<std::string, std::string> fdmap{ std::pair{ "cwd", "cwd" }, std::pair{ "root", "rtd" }, std::pair{ "exe", "txt" } };

int main(int argc, char** argv) {

    checkArguments(argc, argv);
    auto pids{ getPIDs() };

    std::sort(pids.begin(), pids.end());

    std::vector<Record> records;

    for (auto pid : pids) {
        std::vector<Record> pidRecords;

        auto dir{ format("/proc/%d/", pid) };

        auto command{ getCommand(dir + "comm") };
        if (command.empty()) continue;

        auto username{ getUser(dir) };
        if (username.empty()) continue;

        for (auto& d : infoDir) { // cwd, root, exe
            auto path{ dir + d };

            auto st{ Stat(path) };
            if (!st && errno != EACCES) continue;

            std::string fd{}, type{};
            int inode{};

            if (!st) {
                type = "unknown";
                inode = -1;
                fd = fdmap[d];
            }
            else {
                type = getType(*st);
                fd = fdmap[d];
                inode = st->st_ino;
                path = getName(path);
            }

            pidRecords.emplace_back(command, pid, username, fd, type, inode, path, !st);
        }

        { // maps
            std::map<std::pair<int, std::string>, std::size_t> maps;
            std::ifstream fin{ format("/proc/%d/maps", pid) };

            std::string line;
            std::size_t seqnum{};
            while (getline(fin, line)) {
                std::string address, perms, offset, dev, pathname;
                int inode;
                std::stringstream ss{ line };
                ss >> address >> perms >> offset >> dev >> inode >> pathname;

                std::string temp{};
                while (ss >> temp) pathname += " " + temp;

                if (maps.find({ inode, pathname }) == maps.end()) { // not occurred
                    maps[{ inode, pathname }] = ++seqnum;
                }
            }

            using Info = std::tuple<std::size_t, int, std::string>;
            std::set<Info> mapped;
            for (auto [key, val] : maps) mapped.emplace(val, key.first, key.second);

            for (auto [seqnum, inode, pathname] : mapped) {
                std::string fd{ "mem" };

                if (pathname.find("(deleted)") != std::string::npos) {
                    pathname.erase(pathname.length() - 10);
                    fd = "DEL";
                }

                auto st{ Stat(pathname) };
                if (!st) continue;
                auto type{ getType(*st) };

                if (pidRecords.back().inode == inode && pidRecords.back().filename == pathname) continue;
                pidRecords.emplace_back(command, pid, username, fd, type, st ? inode : -1, pathname);
            }
        }

        { // fd
            auto path{ format("/proc/%d/fd", pid) };
            DIR* dp{ opendir(path.c_str()) };
            if (dp == nullptr) {
                if (errno != EACCES) continue;
                pidRecords.emplace_back(command, pid, username, "NOFD", "", -1, path, true);
                continue;
            }

            dirent* dirp{};
            while ((dirp = readdir(dp)) != nullptr) {
                if (not std::regex_match(dirp->d_name, std::regex{ "[0-9]+" })) continue;

                auto fdpath{ format("%s/%s", path.c_str(), dirp->d_name) };

                auto st{ Stat(fdpath) };
                if (!st && errno != EACCES) continue;

                auto acc{ testAccess(fdpath) };
                auto fd{ format("%s%c", dirp->d_name, acc) };

                auto inode{ st ? st->st_ino : -1 };

                std::string type{ getType(*st) };

                auto filename{ getName(fdpath) };

                if (filename.find("(deleted)") != std::string::npos) {
                    filename.erase(filename.length() - 10);
                }

                pidRecords.emplace_back(command, pid, username, (acc > 0) ? fd : "", type, inode, filename);
            }

            records.insert(records.end(), pidRecords.begin(), pidRecords.end());
        }
    }

    std::cout << "COMMAND\t\tPID\t\tUSER\t\tFD\t\tTYPE\t\tNODE\t\tNAME" << std::endl;
    for (auto& r : records) {
        if (flgCommandFilter && not std::regex_search(r.command, commandRE)) continue;
        if (flgTypeFilter && not std::regex_search(r.type, typeRE)) continue;
        if (flgFilenameFilter && not std::regex_search(r.filename, filenameRE)) continue;
        std::cout << r << std::endl;

    }

    return 0;
}

template<class... Args>
std::string format(const std::string& fmt, Args... args) {
    int ssize{ std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1 };
    if (ssize <= 0) return std::string{};

    size_t size{ static_cast<size_t>(ssize) };

    auto buf{ std::make_unique<char[]>(size) };
    std::snprintf(buf.get(), size, fmt.c_str(), args...);

    return std::string{ buf.get() };
}

void checkArguments(int argc, char** argv) {
    static const std::regex __typeRE{ "(REG)|(CHR)|(DIR)|(FIFO)|(SOCK)|(unknown)" };

    char c;
    while ((c = getopt(argc, argv, "c:t:f:")) != -1) {
        switch (c) {
        case 'c':
            flgCommandFilter = true;
            commandRE = optarg;
            break;
        case 't':
            if (not std::regex_match(optarg, __typeRE)) {
                std::puts("Invalid TYPE option.");
                std::exit(EXIT_FAILURE);
            }

            flgTypeFilter = true;
            typeRE = optarg;
            break;
        case 'f':
            flgFilenameFilter = true;
            filenameRE = optarg;
            break;
        }
    }
}
