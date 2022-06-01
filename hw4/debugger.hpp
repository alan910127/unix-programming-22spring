#ifndef _DEBUGGER_HPP
#define _DEBUGGER_HPP

#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <map>

#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "utilities.hpp"

enum class State {
    NOT_LOADED,
    LOADED,
    RUNNING
};

void debugger(std::istream& input);
void endDebugger();

bool dbgParseCommand(const std::string& command);

void dbgAddBreakpoint(uint64_t address);
void dbgContinue();
void dbgDeleteBreakpoint(uint64_t breakpointID);
void dbgDisasm(uint64_t address);
void dbgDump(uint64_t address);
void dbgTerminate();
void dbgGetRegister(const std::string& registerName);
void dbgGetAllRegister();
void dbgHelp();
void dbgListBreakpoint();
void dbgLoadProgram(const std::string& path);
void dbgRun();
void dbgMemoryMap();
void dbgSetRegisterValue(const std::string& registerName, uint64_t value);
void dbgStepInstruction();
void dbgStart();

#endif // !_DEBUGGER_HPP