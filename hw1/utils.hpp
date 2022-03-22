#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>

#define err_quit(msg) { std::perror(msg); std::exit(EXIT_FAILURE); }

using stat_t = struct stat;

std::vector<int> getPIDs();
char testAccess(const std::string& path);

#endif // !_UTILS_HPP
