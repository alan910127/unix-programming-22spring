#include "debugger.hpp"

extern std::string program_record;

Debugger::Debugger() :
    isAlive{ true }, isChildAlive{ false }, isSTDIN{ true }, stream{ std::cin },
    state{ State::NOT_LOADED }, process{}, program{}, breakpoints{} {
}

Debugger::Debugger(std::istream& stream) :
    isAlive{ true }, isChildAlive{ false }, isSTDIN{ false }, stream{ stream },
    state{ State::NOT_LOADED }, process{}, program{}, breakpoints{} {
}

Debugger::Debugger(const string& program) :
    isAlive{ true }, isChildAlive{ false }, isSTDIN{ true }, stream{ std::cin },
    state{ State::NOT_LOADED }, process{}, program{ program }, breakpoints{} {
    loadProgram(this->program);
}

Debugger::Debugger(const string& program, std::istream& stream) :
    isAlive{ true }, isChildAlive{ false }, isSTDIN{ false }, stream{ stream },
    state{ State::NOT_LOADED }, process{}, program{ program }, breakpoints{} {
    loadProgram(this->program);
}

Debugger::~Debugger() {
    if (this->isChildAlive) kill(this->process, SIGKILL);
}

void Debugger::mainLoop() {

    while (this->stream) {
        if (this->isSTDIN) std::cout << "sdb> ";
        string line;

        while (this->isAlive and std::getline(this->stream, line)) {

            auto [function, args] = parseCommand(line);

            bool isArgumentValid = checkArgument(function, args);
            if (not isArgumentValid) {
                if (this->isSTDIN) std::cout << "sdb> ";
                continue;
            }

            switch (function) {
            case Function::BREAK: {
                    addr_t address = std::stoull(args[1], nullptr, 0);
                    addBreakpoint(address);
                    break;
                }
            case Function::CONT: {
                    continueInstructions();
                    break;
                }
            case Function::DELETE: {
                    std::size_t id = std::stoull(args[1], nullptr, 0);
                    deleteBreakpoint(id);
                    break;
                }
            case Function::DISASM: {
                    addr_t address = std::stoull(args[1], nullptr, 0);
                    disassemble(address);
                    break;
                }
            case Function::DUMP: {
                    addr_t address = std::stoull(args[1], nullptr, 0);
                    dumpMemory(address);
                    break;
                }
            case Function::EXIT: {
                    terminate();
                    break;
                }
            case Function::GET: {
                    string name = args[1];
                    getRegister(name);
                    break;
                }
            case Function::GETREGS: {
                    getRegister();
                    break;
                }
            case Function::HELP: {
                    help();
                    break;
                }
            case Function::LIST: {
                    listBreakpoint();
                    break;
                }
            case Function::LOAD: {
                    string program = args[1];
                    loadProgram(program);
                    break;
                }
            case Function::RUN: {
                    run();
                    break;
                }
            case Function::VMMAP: {
                    showMemoryMap();
                    break;
                }
            case Function::SET: {
                    string name = args[1];
                    auto value = std::stoull(args[2], nullptr, 0);
                    setRegister(name, value);
                    break;
                }
            case Function::SI: {
                    stepInstruction();
                    break;
                }
            case Function::START: {
                    start();
                    break;
                }
            default:
                std::cout << "** unknown command: " << quote(args.front()) << std::endl;
            }

            if (this->isAlive and this->isSTDIN) std::cout << "sdb> ";
        }

        // reset this
        if (this->isChildAlive) kill(this->process, SIGKILL);
        this->isAlive = true;
        this->isChildAlive = false;
        state = State::LOADED;
        process = 0;
    }
}

void Debugger::help() {
    static constexpr auto messages = {
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

    for (const auto& message : messages) {
        std::cout << message << std::endl;
    }
}

void Debugger::terminate() {
    if (this->isChildAlive) kill(this->process, SIGKILL);
    std::exit(0);
}

void Debugger::loadProgram(const string& program) {

    if (this->state != State::NOT_LOADED) {
        std::cout << "** program " << quote(program) << " has bee loaded" << std::endl;
        return;
    }

    std::ifstream file(program, std::ios::binary);

    Elf64_Ehdr header;
    file.read(reinterpret_cast<char*>(&header), sizeof(Elf64_Ehdr));

    std::cout << "** program " << quote(program) << " loaded." << format(" entry point %#x", header.e_entry) << std::endl;

    std::vector<Elf64_Shdr> sectionHeaders(header.e_shnum);

    Elf64_Shdr& shstrtab = sectionHeaders[header.e_shstrndx];

    file.seekg(header.e_shoff, std::ios::beg);

    for (auto& shdr : sectionHeaders) {
        file.read(reinterpret_cast<char*>(&shdr), sizeof(Elf64_Shdr));
    }

    string stringTable(shstrtab.sh_size, 0);
    file.seekg(shstrtab.sh_offset, std::ios::beg);
    file.read(stringTable.data(), shstrtab.sh_size);

    for (auto shdr : sectionHeaders) {
        using namespace std::string_literals;
        if (&stringTable[shdr.sh_name] == ".text"s) {
            this->textStart = shdr.sh_addr;
            this->textEnd = shdr.sh_addr + shdr.sh_size;
            break;
        }
    }
    std::cout << format("** .text segment: %#x-%#x", this->textStart, this->textEnd) << std::endl;

    program_record = this->program = program;
    this->state = State::LOADED;
}

void Debugger::start() {

    if (this->state == State::NOT_LOADED) {
        std::cout << "** no program loaded" << std::endl;
        return;
    }
    if (this->state == State::RUNNING) {
        std::cout << "** program " << quote(program) << " is already running" << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid < 0) errquit("fork");

    if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) errquit("PTRACE_TRACEME@start");
        execlp(this->program.c_str(), this->program.c_str(), 0);
        errquit("exec");
    }
    else {
        this->process = pid;
        std::cout << "** pid " << this->process << std::endl;

        if (not WIFSTOPPED(waitChild())) this->isChildAlive = this->isAlive = false;
    }

    for (auto [addr, code] : breakpoints) {
        putInt3(addr);
    }

    this->isChildAlive = true;
    this->state = State::RUNNING;
}

void Debugger::run() {

    if (this->state == State::NOT_LOADED) {
        std::cout << "** no program loaded" << std::endl;
        return;
    }

    start();
    this->state = State::RUNNING;

    if (ptrace(PTRACE_CONT, this->process, 0, 0) < 0) errquit("PTRACE_CONT@run");
    if (not WIFSTOPPED(waitChild())) {
        this->isChildAlive = this->isAlive = false;
        return;
    }

    addr_t rip = ptrace(PTRACE_PEEKUSER, process, getRegisterOffset("rip"), 0);
    if (ptrace(PTRACE_POKEUSER, process, getRegisterOffset("rip"), --rip) < 0) errquit("PTRACE_POKEUSER@run");
    printBreakpoint(rip);
}

void Debugger::continueInstructions() {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    if (auto rip = recoverInstruction(); rip != 0) {
        if (ptrace(PTRACE_POKEUSER, this->process, getRegisterOffset("rip"), rip) < 0) errquit("PTRACE_POKEUSER@cont");

        if (ptrace(PTRACE_SINGLESTEP, this->process, 0, 0) < 0) errquit("PTRACE_SINGLESTEP@cont");
        if (not WIFSTOPPED(waitChild())) this->isChildAlive = this->isAlive = false;

        putInt3(rip);
    }

    ptrace(PTRACE_CONT, this->process, 0, 0);
    if (not WIFSTOPPED(waitChild())) {
        this->isChildAlive = this->isAlive = false;
        return;
    }

    auto rip = ptrace(PTRACE_PEEKUSER, this->process, getRegisterOffset("rip"), 0);
    if (ptrace(PTRACE_POKEUSER, process, getRegisterOffset("rip"), --rip) < 0) errquit("PTRACE_POKEUSER@run");
    printBreakpoint(rip);
}

void Debugger::stepInstruction() {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    if (auto rip = recoverInstruction(); rip != 0) {
        if (ptrace(PTRACE_POKEUSER, this->process, getRegisterOffset("rip"), rip) < 0) errquit("PTRACE_POKEUSER@si");
        if (ptrace(PTRACE_SINGLESTEP, this->process, 0, 0) < 0) errquit("PTRACE_SINGLESTEP@si");
        if (not WIFSTOPPED(waitChild())) this->isChildAlive = this->isAlive = false;
        putInt3(rip);
    }
    else {
        ptrace(PTRACE_SINGLESTEP, this->process, 0, 0);
        if (not WIFSTOPPED(waitChild())) this->isChildAlive = this->isAlive = false;
    }

    auto rip = ptrace(PTRACE_PEEKUSER, this->process, getRegisterOffset("rip"), 0);

    if (breakpoints.find(rip) != breakpoints.end()) {
        printBreakpoint(rip);
    }
}

void Debugger::addBreakpoint(addr_t address) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    putInt3(address);
}

void Debugger::deleteBreakpoint(std::size_t breakpointID) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    auto it = this->breakpoints.begin();

    for (size_t index{}; index < breakpointID and it != this->breakpoints.end(); ++index) {
        ++it;
    }

    if (it == this->breakpoints.end()) {
        std::cout << "** invalid breakpoint id" << std::endl;
        return;
    }

    auto [address, code] = *it;
    this->breakpoints.erase(it);

    auto codeInText = ptrace(PTRACE_PEEKTEXT, this->process, address, 0);
    auto codeToRecover = (codeInText & 0xffff'ffff'ffff'ff00) | (code & 0xff);

    if (ptrace(PTRACE_POKETEXT, this->process, address, codeToRecover) < 0) errquit("PTRACE_POKETEXT@delete");
    std::cout << format("** breakpoint %d deleted", breakpointID) << std::endl;
}

void Debugger::listBreakpoint() {
    size_t id{};
    for (auto [address, _] : breakpoints) {
        std::cout << format("%3d: %8lx", id++, address) << std::endl;
    }
}

void Debugger::getRegister() {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, process, 0, &regs) == 0) {
        std::cout << format("RAX %-16lx RBX %-16lx RCX %-16lx RDX %-16lx", regs.rax, regs.rbx, regs.rcx, regs.rdx) << std::endl;
        std::cout << format("R8  %-16lx R9  %-16lx R10 %-16lx R11 %-16lx", regs.r8, regs.r9, regs.r10, regs.r11) << std::endl;
        std::cout << format("R12 %-16lx R13 %-16lx R14 %-16lx R15 %-16lx", regs.r12, regs.r13, regs.r14, regs.r15) << std::endl;
        std::cout << format("RDI %-16lx RSI %-16lx RBP %-16lx RSP %-16lx", regs.rdi, regs.rsi, regs.rbp, regs.rsp) << std::endl;
        std::cout << format("RIP %-16lx FLAGS %016lx", regs.rip, regs.eflags) << std::endl;
    }
}

void Debugger::getRegister(const string& name) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    auto offset = getRegisterOffset(name);

    uint64_t reg = ptrace(PTRACE_PEEKUSER, process, offset, 0);

    std::cout << format("%s = %d (%#lx)", name.c_str(), reg, reg) << std::endl;
}

void Debugger::setRegister(const string& name, uint64_t value) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    auto offset = getRegisterOffset(name);
    if (ptrace(PTRACE_POKEUSER, process, offset, value) < 0) errquit("PTRACE_POKEUSER@set");
}

void Debugger::disassemble(addr_t address) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    Disassembler disassembler{ CS_ARCH_X86, CS_MODE_64 };

    for (size_t i = 0; i < 10; ++i) {
        if (address >= this->textEnd) {
            std::cout << "** the address is out of the range of the text segment" << std::endl;
            break;
        }

        auto it = breakpoints.find(address);

        auto value = (it == breakpoints.end()) ? ptrace(PTRACE_PEEKTEXT, this->process, address, 0) : it->second;

        std::vector<uint8_t> code;
        for (int i = 0; i < 8; ++i) {
            code.push_back(value & 0xff);
            value >>= 8;
        }

        auto [instr, nextInstruction] = disassembler.disasm(code, address);
        auto [addr, bytes, mnemonic, op_str] = instr;

        std::stringstream ss;
        for (auto byte : bytes) {
            ss << format(" %02x", static_cast<uint16_t>(byte) & 0xff);
        }

        std::cout << format("%12lx:%-36s%-10s%s", addr, ss.str().c_str(), mnemonic.c_str(), op_str.c_str()) << std::endl;

        address = nextInstruction;
    }
}

void Debugger::dumpMemory(addr_t address) {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    // 64-bit, little endian
    for (int i = 0; i < 5; ++i) {
        code_t buf[2];

        buf[0] = ptrace(PTRACE_PEEKTEXT, process, address, 0);
        buf[1] = ptrace(PTRACE_PEEKTEXT, process, address + 8, 0);

        auto ptr = reinterpret_cast<uint8_t*>(buf);

        std::cout << format("%12lx:", address);
        for (int j = 0; j < 16; ++j) {
            std::cout << format(" %02x", static_cast<short>(ptr[j]) & 0xff);
        }

        std::cout << "  |";
        for (int j = 0; j < 16; ++j) {
            std::cout << (std::isprint(ptr[j]) ? static_cast<char>(ptr[j]) : '.');
        }
        std::cout << "|" << std::endl;

        address += 16;
    }

}

void Debugger::showMemoryMap() {

    if (this->state != State::RUNNING) {
        std::cout << "** the program is not being run" << std::endl;
        return;
    }

    std::string mmap = "/proc/" + std::to_string(this->process) + "/maps";
    std::ifstream file{ mmap };

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss{ line };
        char delim;
        addr_t start, end;
        std::string permission, dev, pathname;
        uint64_t inode, offset;

        ss >> std::hex >> start >> delim >> end >> permission >> offset >> dev >> std::dec >> inode >> pathname;
        permission.pop_back();

        std::cout << format("%016lx-%016lx %s %-8x %s", start, end, permission.c_str(), offset, pathname.c_str()) << std::endl;
    }
}

auto Debugger::parseCommand(const string& command) -> std::pair<Function, args_t> {
    auto args = splitString(command);

    if (args.empty()) return { Function::UNKNOWN, args };

    auto& cmd = args.front();

    if (cmd == "break" or cmd == "b") return { Function::BREAK, args };
    if (cmd == "cont" or cmd == "c") return { Function::CONT, args };
    if (cmd == "delete") return { Function::DELETE, args };
    if (cmd == "disasm" or cmd == "d") return { Function::DISASM, args };
    if (cmd == "dump" or cmd == "x") return { Function::DUMP, args };
    if (cmd == "exit" or cmd == "q") return { Function::EXIT, args };
    if (cmd == "get" or cmd == "g") return { Function::GET, args };
    if (cmd == "getregs") return { Function::GETREGS, args };
    if (cmd == "help" or cmd == "h") return { Function::HELP, args };
    if (cmd == "list" or cmd == "l") return { Function::LIST, args };
    if (cmd == "load") return { Function::LOAD, args };
    if (cmd == "run" or cmd == "r") return { Function::RUN, args };
    if (cmd == "vmmap" or cmd == "m") return { Function::VMMAP, args };
    if (cmd == "set" or cmd == "s") return { Function::SET, args };
    if (cmd == "si") return { Function::SI, args };
    if (cmd == "start") return { Function::START, args };

    return { Function::UNKNOWN, args };
}

bool Debugger::checkArgument(Function function, const args_t& args) {

    if (args.empty()) return false;

    auto num = args.size() - 1;

    switch (function) {
    case Function::BREAK:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing breakpoint address" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::CONT:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::DELETE:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing breakpoint id" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::DISASM:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing diassemble address" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::DUMP:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing dump address" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::EXIT:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::GET:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing register name" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::GETREGS:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::HELP:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::LIST:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::LOAD:
        if (num == 1) return true;

        if (num < 1) std::cout << "** missing program name" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::RUN:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::VMMAP:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::SET:
        if (num == 2) return true;

        if (num < 2) std::cout << "** missing register name or value" << std::endl;
        else std::cout << "** too many arguments" << std::endl;
        break;

    case Function::SI:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    case Function::START:
        if (num == 0) return true;

        std::cout << "** too many arguments" << std::endl;
        break;

    default:
        std::cout << "** unknown command: " << quote(args.front()) << std::endl;
    }
    return false;
}

auto Debugger::recoverInstruction() -> addr_t {

    auto offset = getRegisterOffset("rip");
    auto rip = ptrace(PTRACE_PEEKUSER, this->process, offset, 0);

    auto it = breakpoints.find(rip);

    if (it == breakpoints.end()) return 0;

    auto code = it->second;
    auto codeInText = ptrace(PTRACE_PEEKTEXT, process, rip, 0);
    auto codeToRecover = (codeInText & 0xffff'ffff'ffff'ff00) | (code & 0xff); // only recover the byte that is changed to INT3

    if (ptrace(PTRACE_POKETEXT, process, rip, codeToRecover) < 0) errquit("PTRACE_POKETEXT@recover");

    return rip;

}

void Debugger::putInt3(addr_t address) {

    auto code = ptrace(PTRACE_PEEKTEXT, process, address, 0);
    breakpoints[address] = code;

    auto inserted = (code & 0xffff'ffff'ffff'ff00) | 0xcc;
    if (ptrace(PTRACE_POKETEXT, process, address, inserted) < 0) errquit("PTRACE_POKETEXT@put");

}

int Debugger::waitChild() {
    int status;
    if (waitpid(this->process, &status, 0) < 0) errquit("wait");

    if (WIFEXITED(status)) {
        std::cout << "** child process " << this->process << " terminiated normally (code " << WEXITSTATUS(status) << ")" << std::endl;
    }
    if (WIFSIGNALED(status)) {
        std::cout << "** child process " << this->process << " terminiated abnormally (code " << WEXITSTATUS(status) << ")" << std::endl;
    }

    return status;
}

void Debugger::printBreakpoint(addr_t address) {

    code_t code = breakpoints[address];

    Disassembler disassembler{ CS_ARCH_X86, CS_MODE_64 };

    Disassembler::code_t code_disasm;
    for (int i = 0; i < 8; ++i) {
        code_disasm.push_back(code & 0xff);
        code >>= 8;
    }

    auto [instr, _] = disassembler.disasm(code_disasm, address);

    auto [addr, bytes, mnemonic, op_str] = instr;

    std::stringstream ss;
    for (auto byte : bytes) {
        ss << format(" %02x", static_cast<uint16_t>(byte) & 0xff);
    }

    std::cout << format("** breakpoint @%12lx:%-32s%s\t%s", addr, ss.str().c_str(), mnemonic.c_str(), op_str.c_str()) << std::endl;
}
