#include "debugger.hpp"

State currentState = State::NOT_LOADED;
std::string program;
std::variant<ELF32, ELF64> header;
pid_t process;
std::map<uint64_t, uint64_t> breakpoints; // address -> code

void debugger(std::istream& input) {
    std::string command;

    while (input) {
        std::cerr << "sdb> ";
        while (std::getline(input, command)) {
            dbgParseCommand(command);
            std::cerr << "sdb> ";
        }
        endDebugger();
    }
}

void endDebugger() {
    kill(process, SIGKILL);
    currentState = State::LOADED;
}

bool dbgParseCommand(const std::string& command) {
    using namespace std::string_literals;

    using dbgFuncType = std::variant<void(*)(), void(*)(uint64_t), void(*)(const std::string&), void(*)(const std::string&, uint64_t)>;

    static const std::map<std::string, dbgFuncType> functionTable{
        std::pair{ "break"s, dbgAddBreakpoint },
        std::pair{ "b"s, dbgAddBreakpoint },
        std::pair{ "cont"s, dbgContinue },
        std::pair{ "c"s, dbgContinue },
        std::pair{ "delete"s, dbgDeleteBreakpoint },
        std::pair{ "disasm"s, dbgDisasm },
        std::pair{ "d"s, dbgDisasm },
        std::pair{ "dump"s, dbgDump },
        std::pair{ "x"s, dbgDump },
        std::pair{ "exit"s, dbgTerminate },
        std::pair{ "q"s, dbgTerminate },
        std::pair{ "get"s, dbgGetRegister },
        std::pair{ "g"s, dbgGetRegister },
        std::pair{ "getregs"s, dbgGetAllRegister },
        std::pair{ "help"s, dbgHelp },
        std::pair{ "h"s, dbgHelp },
        std::pair{ "list"s, dbgListBreakpoint },
        std::pair{ "l"s, dbgListBreakpoint },
        std::pair{ "load"s, dbgLoadProgram },
        std::pair{ "run"s, dbgRun },
        std::pair{ "r"s, dbgRun },
        std::pair{ "vmmap"s, dbgMemoryMap },
        std::pair{ "m"s, dbgMemoryMap },
        std::pair{ "set"s, dbgSetRegisterValue },
        std::pair{ "s"s, dbgSetRegisterValue },
        std::pair{ "si"s, dbgStepInstruction },
        std::pair{ "start"s, dbgStart }
    };

    auto args = splitString(command);

    if (args.empty()) return false;
    const auto& cmd = args.front();

    if (auto it = functionTable.find(cmd); it == functionTable.end()) {
        std::cerr << "** Invalid command: " << quote(cmd) << std::endl;
        return false;
    }
    else {
        auto func = it->second;
        auto numArgs = args.size() - 1;

        switch (func.index()) {
        case 0: // void ()
            if (numArgs != 0) {
                std::cerr << "** 0 arguments expected, got " << numArgs << std::endl;
                return false;
            }
            std::get<0>(func)();
            break;
        case 1: // void (uint64_t)
            if (numArgs != 1) {
                std::cerr << "** 1 arguments expected, got " << numArgs << std::endl;
                return false;
            }
            std::get<1>(func)(stoull(args[1], nullptr, 0));
            break;
        case 2: // void (const std::string&)
            if (numArgs != 1) {
                std::cerr << "** 1 arguments expected, got " << numArgs << std::endl;
                return false;
            }
            std::get<2>(func)(args[1]);
            break;
        case 3: // void (const std::string&, uint64_t)
            if (numArgs != 2) {
                std::cerr << "** 2 arguments expected, got " << numArgs << std::endl;
                return false;
            }
            std::get<3>(func)(args[1], stoull(args[2], nullptr, 0));
            break;
        default:
            std::cerr << "Debugger function error" << std::endl;
            std::exit(-1);
        }
    }
    return true;
}

uint64_t recoverInstruction() {

    auto offset = getRegisterOffset("rip");
    auto rip = ptrace(PTRACE_PEEKUSER, process, offset, 0);

    auto it = breakpoints.find(rip);

    if (it == breakpoints.end()) {
        // next instruction is not breakpoint
        // test whether we are stucked by breakpoint
        --rip;
        it = breakpoints.find(rip);
        if (it == breakpoints.end()) return 0; // neither, return directly
    }

    auto code = it->second;
    auto codeInText = ptrace(PTRACE_PEEKTEXT, process, rip, 0);
    auto codeToRecover = (codeInText & 0xffff'ffff'ffff'ff00) | (code & 0xff); // only recover the byte that is changed to INT3

    if (ptrace(PTRACE_POKETEXT, process, rip, codeToRecover) < 0) errquit("ptrace.POKETEXT@recover");

    return rip;
}

void putInt3(uint64_t address) {
    auto code = ptrace(PTRACE_PEEKTEXT, process, address, 0);
    breakpoints[address] = code;

    auto inserted = (code & 0xffff'ffff'ffff'ff00) | 0xcc;
    if (ptrace(PTRACE_POKETEXT, process, address, inserted) < 0) errquit("ptrace.POKETEXT@put");
}

void dbgAddBreakpoint(uint64_t address) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    putInt3(address);
}

void dbgContinue() {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    if (auto rip = recoverInstruction(); rip != 0) {
        if (ptrace(PTRACE_POKEUSER, process, getRegisterOffset("rip"), rip) < 0) errquit("ptrace.POKEUSER");

        if (ptrace(PTRACE_SINGLESTEP, process, 0, 0) < 0) errquit("ptrace.SINGLESTEP");
        if (!wait_child(process)) NEXT_ROUND();

        putInt3(rip);
    }

    ptrace(PTRACE_CONT, process, 0, 0);
    if (!wait_child(process)) NEXT_ROUND();

    ios_flag_saver ifs{ std::cerr };
    auto rip = ptrace(PTRACE_PEEKUSER, process, getRegisterOffset("rip"), 0);
    std::cerr << " ** breakpoint @ = " << std::setw(12) << std::hex << rip - 1 << ": " << breakpoints[rip - 1] << std::endl;
}

void dbgDeleteBreakpoint(uint64_t breakpointID) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }
}

void dbgDisasm(uint64_t address) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

}

void dbgDump(uint64_t address) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }


    // assumption: 64-bit, little endian
    for (int i = 0; i < 5; ++i) {
        ios_flag_saver ifs{ std::cerr };
        uint64_t buf[2];

        buf[0] = ptrace(PTRACE_PEEKTEXT, process, address, 0);
        buf[1] = ptrace(PTRACE_PEEKTEXT, process, address + 8, 0);

        auto ptr = reinterpret_cast<uint8_t*>(buf);

        std::cerr << std::setw(12) << std::right << std::hex << address << ":";
        for (int j = 0; j < 16; ++j) {
            std::cerr << ' ' << std::setw(2) << std::setfill('0') << std::hex << (static_cast<short>(ptr[j]) & 0xff);
        }

        std::cerr << "  |";
        for (int j = 0; j < 16; ++j) {
            std::cerr << (std::isprint(ptr[j]) ? static_cast<char>(ptr[j]) : '.');
        }
        std::cerr << "|" << std::endl;

        address += 16;
    }
}

void dbgTerminate() {
    kill(process, SIGKILL);
    std::exit(0);
}

void dbgGetRegister(const std::string& registerName) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    auto offset = getRegisterOffset(registerName);

    uint64_t reg = ptrace(PTRACE_PEEKUSER, process, offset, 0);

    ios_flag_saver ifs{ std::cerr };
    std::cerr << registerName << " = " << reg << " (0x" << std::hex << reg << ")" << std::endl;
}

void dbgGetAllRegister() {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    struct user_regs_struct regs;
    ios_flag_saver ifs{ std::cerr };

    if (ptrace(PTRACE_GETREGS, process, 0, &regs) == 0) {
        std::cerr << std::left << std::hex;
        std::cerr << "RAX " << std::setw(16) << regs.rax;
        std::cerr << " RBX " << std::setw(16) << regs.rbx;
        std::cerr << " RCX " << std::setw(16) << regs.rcx;
        std::cerr << " RDX " << std::setw(16) << regs.rdx << std::endl;
        std::cerr << "R8  " << std::setw(16) << regs.r8;
        std::cerr << " R9  " << std::setw(16) << regs.r9;
        std::cerr << " R10 " << std::setw(16) << regs.r10;
        std::cerr << " R11 " << std::setw(16) << regs.r11 << std::endl;
        std::cerr << "R12 " << std::setw(16) << regs.r12;
        std::cerr << " R13 " << std::setw(16) << regs.r13;
        std::cerr << " R14 " << std::setw(16) << regs.r14;
        std::cerr << " R15 " << std::setw(16) << regs.r15 << std::endl;
        std::cerr << "RDI " << std::setw(16) << regs.rdi;
        std::cerr << " RSI " << std::setw(16) << regs.rsi;
        std::cerr << " RBP " << std::setw(16) << regs.rbp;
        std::cerr << " RSP " << std::setw(16) << regs.rsp << std::endl;
        std::cerr << "RIP " << std::setw(16) << regs.rip;
        std::cerr << " FLAGS " << std::setw(16) << std::setfill('0') << std::right << regs.eflags << std::endl;
    }
}

void dbgHelp() {
    static constexpr auto helpMessages = {
        "- break {instruction-address}: add a break point",
        "- cont: continue execution",
        "- delete {break-point-id}: remove a break point",
        "- disasm addr: disassemble instructions in a file or a memory region",
        "- dump addr: dump memory content",
        "- exit: terminate the debugger",
        "- get reg: get a single value from a register",
        "- getregs: show registers",
        "- help: show this message",
        "- list: list break points",
        "- load {path/to/a/program}: load a program",
        "- run: run the program",
        "- vmmap: show memory layout",
        "- set reg val: get a single value to a register",
        "- si: step into instruction",
        "- start: start the program and stop at the first instruction"
    };

    for (const auto& message : helpMessages) {
        std::cerr << message << std::endl;
    }
}

void dbgListBreakpoint() {
    size_t id{};
    for (auto [addr, code] : breakpoints) {
        ios_flag_saver ifs{ std::cerr };
        std::cerr << std::setw(3) << std::right << id << ':';
        std::cerr << std::setw(8) << std::right << std::hex << addr << std::endl;
        ++id;
    }
}

void dbgLoadProgram(const std::string& path) {

    if (currentState != State::NOT_LOADED) {
        std::cerr << "** program " << quote(program) << " has bee loaded" << std::endl;
        return;
    }

    std::ifstream file(path, std::ios::binary);

    ELFHeader prefix;
    file.read(reinterpret_cast<char*>(&prefix), sizeof(ELFHeader));

    if (prefix.executable_class == CLASS64) header = ELF64{};
    else header = ELF32{};

    std::visit([&file, &path](auto& elf) {
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(&elf), sizeof(elf));
        ios_flag_saver ifs{ std::cerr };
        std::cerr << "** program " << quote(path) << " loaded. entry point 0x" << std::hex << elf.entry_point << std::dec << std::endl;
        }, header);

    program = path;
    currentState = State::LOADED;
}

void dbgRun() {

    if (currentState == State::NOT_LOADED) {
        std::cerr << "** no program is loaded" << std::endl;
        return;
    }

    dbgStart();
    currentState = State::RUNNING;
    dbgContinue();
}

void dbgMemoryMap() {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    std::string mmap = "/proc/" + std::to_string(process) + "/maps";
    std::ifstream file{ mmap };

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss{ line };
        std::string address, permission, offset_s, dev, pathname;
        uint64_t inode;

        ss >> address >> permission >> offset_s >> dev >> inode >> pathname;

        size_t idx;
        auto start = std::stoull(address, &idx, 16);
        auto end = std::stoull(address.substr(idx + 1), nullptr, 16);
        auto offset = std::stoull(offset_s, nullptr, 16);
        permission.pop_back();

        ios_flag_saver ifs{ std::cerr };
        std::cerr << std::setw(16) << std::setfill('0') << std::hex << start;
        std::cerr << '-' << std::setw(16) << std::setfill('0') << std::hex << end;
        std::cerr << ' ' << permission;
        std::cerr << ' ' << std::setw(8) << std::setfill(' ') << std::left << std::hex << offset;
        std::cerr << ' ' << pathname << std::endl;
    }
}

void dbgSetRegisterValue(const std::string& registerName, uint64_t value) {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    auto offset = getRegisterOffset(registerName);
    if (ptrace(PTRACE_POKEUSER, process, offset, value) < 0) errquit("ptrace@set");
}

void dbgStepInstruction() {

    if (currentState != State::RUNNING) {
        std::cerr << "** no programm is currently running" << std::endl;
        return;
    }

    if (auto rip = recoverInstruction(); rip != 0) {
        if (ptrace(PTRACE_POKEUSER, process, getRegisterOffset("rip"), rip) < 0) errquit("ptrace.POKEUSER@si");
        if (ptrace(PTRACE_SINGLESTEP, process, 0, 0) < 0) errquit("ptrace.SINGLESTEP@si");
        if (!wait_child(process)) NEXT_ROUND();
        putInt3(rip);
    }
    else {
        ptrace(PTRACE_SINGLESTEP, process, 0, 0);
        if (!wait_child(process)) NEXT_ROUND();
    }

    ios_flag_saver ifs{ std::cerr };

    auto rip = ptrace(PTRACE_PEEKUSER, process, getRegisterOffset("rip"), 0);
    if (auto it = breakpoints.find(rip); it != breakpoints.end()) {
        std::cerr << " ** breakpoint @ = " << std::setw(12) << std::hex << it->first << ": " << it->second << std::endl;
    }
}

void dbgStart() {

    if (currentState == State::NOT_LOADED) {
        std::cerr << "** no program loaded" << std::endl;
        return;
    }
    if (currentState == State::RUNNING) {
        std::cerr << "** program " << quote(program) << " is already running" << std::endl;
        return;
    }

    process = fork();
    if (process < 0) errquit("fork");

    if (process == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) errquit("ptrace.TRACEME");
        execlp(program.c_str(), program.c_str(), 0);
        errquit("exec");
    }
    else {
        std::cerr << "** pid " << process << std::endl;
        if (!wait_child(process)) NEXT_ROUND();
    }

    currentState = State::RUNNING;
}
