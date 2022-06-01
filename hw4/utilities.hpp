#ifndef _UTILITIES_HPP
#define _UTILITIES_HPP

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#define errquit(msg) { std::perror(msg); std::exit(-1); }

#define CLASS32 1
#define CLASS64 2

struct ELFHeader {
    uint8_t prefix[4];
    uint8_t executable_class;
    uint8_t endianness;
    uint8_t version;
    uint8_t ABI;
    uint8_t ABI_version;
    uint8_t unused[7];
    uint16_t type;
    uint16_t machine;
    uint32_t Version;
};

struct ELF64 {
    ELFHeader header;
    uint64_t entry_point;
    uint64_t program_header_offset;
    uint64_t section_header_offset;
    uint32_t flags;
    uint16_t size;
    uint16_t program_header_size;
    uint16_t program_header_count;
    uint16_t section_header_size;
    uint16_t section_header_count;
    uint16_t section_header_index;
};

struct ELF32 {
    ELFHeader header;
    uint32_t entry_point;
    uint32_t program_header_offset;
    uint32_t section_header_offset;
    uint16_t flags;
    uint16_t size;
    uint16_t program_header_size;
    uint16_t program_header_count;
    uint16_t section_header_size;
    uint16_t section_header_count;
    uint16_t section_header_index;
};

class ios_flag_saver {
public:
    explicit ios_flag_saver(std::ostream& _stream);
    ~ios_flag_saver();

    ios_flag_saver(const ios_flag_saver&) = delete;
    ios_flag_saver& operator=(const ios_flag_saver&) = delete;
private:
    std::ostream& stream;
    std::ios state;
};

void usage(const std::string& executable, const std::string& message);
void handleArguments(const std::vector<std::string>& args);

auto quote(const std::string& str)->decltype(std::quoted(std::string{}));
auto splitString(const std::string& str)->std::vector<std::string>;
bool iequals(const std::string& lhs, const std::string& rhs);
ptrdiff_t getRegisterOffset(const std::string& registerName);

bool wait_child(pid_t process);

#endif // !_UTILITIES_HPP 
