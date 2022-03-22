#include "record.hpp"

std::map<mode_t, string> modemap{
    std::pair{ S_IFDIR, "DIR" }, std::pair{ S_IFREG, "REG" },
    std::pair{ S_IFCHR, "CHR" }, std::pair{ S_IFIFO, "FIFO" },
    std::pair{ S_IFSOCK, "SOCK" }
};

Record::Record() : flgPermissionDenied{ false } {}
Record::Record(const string& command, int pid, const string& user, const string& fd, const string& type, int inode, const string& filename, bool flgPermissionDenied)
    : command{ command }, pid{ pid }, user{ user }, fd{ fd }, type{ type }, inode{ inode }, filename{ filename }, flgPermissionDenied{ flgPermissionDenied } {}

bool Record::operator<(const Record& r) const {
    return this->pid < r.pid;
}

std::ostream& operator<<(std::ostream& os, const Record& r) {
    os << r.command << "\t\t";
    os << r.pid << "\t\t";
    os << r.user << "\t\t";
    os << r.fd << "\t\t";
    os << r.type << "\t\t";
    if (r.inode >= 0) os << r.inode;
    os << "\t\t" << r.filename;
    if (r.flgPermissionDenied) os << " (Permission denied)";
    return os;
}

string getCommand(const string& comm) {
    std::ifstream fout{ comm };
    string result{};
    std::getline(fout, result);
    return result;
}

string getUser(const string& path) {
    stat_t st{};
    int ret{ stat(path.c_str(), &st) };
    if (ret < 0) return string{};
    return string{ getpwuid(st.st_uid)->pw_name };
}

string getType(const stat_t& st) {
    auto it{ modemap.find(st.st_mode & S_IFMT) };
    return (it == modemap.end()) ? "unknown" : it->second;
}

string getName(const string& path) {
    char buf[4096]{};
    readlink(path.c_str(), buf, 4096);
    return string{ buf };
}
