#ifndef _RECORD_HPP
#define _RECORD_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

using std::string;

struct Record {
    string command;
    int pid;
    string user;
    string fd;
    string type;
    int inode;
    string filename;
    bool flgPermissionDenied;

    Record();
    Record(const string& command, int pid, const string& user, const string& fd, const string& type, int inode, const string& filename, bool flgPermissionDenied = false);
    bool operator<(const Record& r) const;
};

using stat_t = struct stat;

std::ostream& operator<<(std::ostream& os, const Record& r);

string getCommand(const string& comm);
string getUser(const string& path);
string getType(const stat_t& st);
string getName(const string& path);

#endif // !_RECORD_HPP
