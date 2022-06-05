#ifndef _UTILITIES_HPP
#define _UTILITIES_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>

#define errquit(msg) { std::perror(msg); std::exit(-1); }

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
std::pair<std::string, std::string> handleArguments(const std::vector<std::string>& args);

auto quote(const std::string& str)->decltype(std::quoted(std::string{}));
auto splitString(const std::string& str)->std::vector<std::string>;
bool iequals(const std::string& lhs, const std::string& rhs);

ptrdiff_t getRegisterOffset(const std::string& registerName);

template<class... Args>
std::string format(const std::string& fmt, Args... args) {
    auto ssize{ std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1 };
    if (ssize <= 0) return std::string{};

    std::size_t size{ static_cast<std::size_t>(ssize) };

    auto buf{ std::make_unique<char[]>(size) };
    std::snprintf(buf.get(), size, fmt.c_str(), args...);

    return std::string{ buf.get() };
}

#endif // !_UTILITIES_HPP 
