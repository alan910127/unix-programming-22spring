#ifndef _DEBUGGER_HPP
#define _DEBUGGER_HPP

#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <map>

#include <elf.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <capstone/capstone.h>

#include "utilities.hpp"
#include "disassemble.hpp"

class Debugger {
public:
    using addr_t = uint64_t;
    using code_t = uint64_t;
    using args_t = std::vector<std::string>;
    using string = std::string;

    Debugger();
    Debugger(std::istream& stream);
    Debugger(const string& program);
    Debugger(const string& program, std::istream& stream);
    ~Debugger();

    void mainLoop();

    void help();
    void terminate();
    void loadProgram(const string& program);

    void start();
    void run();

    void continueInstructions();
    void stepInstruction();

    void addBreakpoint(addr_t address);
    void deleteBreakpoint(std::size_t breakpointID);
    void listBreakpoint();

    void getRegister(); // all registers
    void getRegister(const string& name);
    void setRegister(const string& name, uint64_t value);

    void disassemble(addr_t address);
    void dumpMemory(addr_t address);
    void showMemoryMap();

private:
    enum class Function {
        BREAK, CONT, DELETE, DISASM,
        DUMP, EXIT, GET, GETREGS,
        HELP, LIST, LOAD, RUN,
        VMMAP, SET, SI, START,
        UNKNOWN = -1
    };

    enum class State {
        NOT_LOADED,
        LOADED,
        RUNNING
    };

    std::pair<Function, args_t> parseCommand(const string& command);
    bool checkArgument(Function function, const args_t& args);
    addr_t recoverInstruction();
    void putInt3(addr_t address);
    int waitChild();
    void printBreakpoint(addr_t address);

    bool isAlive;
    bool isChildAlive;
    bool isSTDIN;
    std::istream& stream;
    State state;
    pid_t process;
    string program;
    std::map<addr_t, code_t> breakpoints;
    addr_t textStart, textEnd;
};

#endif // !_DEBUGGER_HPP