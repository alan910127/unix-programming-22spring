#include "utilities.hpp"
#include "debugger.hpp"

extern State currentState;
extern std::string script;
extern std::string program;

ios_flag_saver::ios_flag_saver(std::ostream& _stream) : stream{ _stream }, state{ nullptr } {
    state.copyfmt(stream);
}
ios_flag_saver::~ios_flag_saver() { stream.copyfmt(state); }

void usage(const std::string& executable, const std::string& message) {
    std::cerr << "usage: " << executable << " [-s script] [program]" << std::endl;
    std::cerr << message << std::endl;
    std::exit(-1);
}

void handleArguments(const std::vector<std::string>& args) {
    if (args.size() == 1) return;

    for (auto it = std::next(args.begin()); it != args.end(); ++it) {
        if (*it == "-s") {
            ++it;
            if (it == args.end()) usage(args.front(), "'-s' option is given but no script provided.");

            script = *it;
            if (access(script.c_str(), R_OK) < 0) errquit("script");
        }
        else if (it->find('-') == 0) {
            std::stringstream ss;
            std::string message;
            ss << "Unknown option: ";
            ss << std::quoted(*it, '\'');
            ss >> message;
            usage(args.front(), message.c_str());
        }
        else {
            program = *it;
            if (access(program.c_str(), R_OK | W_OK | X_OK) < 0) errquit("program");

            dbgLoadProgram(program);
        }
    }
}

auto quote(const std::string& str) -> decltype(std::quoted(std::string{})) {
    return std::quoted(str, '\'');
}

auto splitString(const std::string& str) -> std::vector<std::string> {
    std::stringstream ss{ str };

    std::vector<std::string> result;
    std::string token;

    while (ss >> token) {
        result.emplace_back(token);
    }

    return result;
}

bool iequals(const std::string& lhs, const std::string& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](char a, char b) {
            return std::tolower(a) == std::tolower(b);
        }
    );
}

ptrdiff_t getRegisterOffset(const std::string& registerName) {
    ptrdiff_t offset;

    if (iequals(registerName, "rax")) offset = offsetof(user_regs_struct, rax);
    else if (iequals(registerName, "rbx")) offset = offsetof(user_regs_struct, rbx);
    else if (iequals(registerName, "rcx")) offset = offsetof(user_regs_struct, rcx);
    else if (iequals(registerName, "rdx")) offset = offsetof(user_regs_struct, rdx);
    else if (iequals(registerName, "r8")) offset = offsetof(user_regs_struct, r8);
    else if (iequals(registerName, "r9")) offset = offsetof(user_regs_struct, r9);
    else if (iequals(registerName, "r10")) offset = offsetof(user_regs_struct, r10);
    else if (iequals(registerName, "r11")) offset = offsetof(user_regs_struct, r11);
    else if (iequals(registerName, "r12")) offset = offsetof(user_regs_struct, r12);
    else if (iequals(registerName, "r13")) offset = offsetof(user_regs_struct, r13);
    else if (iequals(registerName, "r14")) offset = offsetof(user_regs_struct, r14);
    else if (iequals(registerName, "r15")) offset = offsetof(user_regs_struct, r15);
    else if (iequals(registerName, "rdi")) offset = offsetof(user_regs_struct, rdi);
    else if (iequals(registerName, "rsi")) offset = offsetof(user_regs_struct, rsi);
    else if (iequals(registerName, "rbp")) offset = offsetof(user_regs_struct, rbp);
    else if (iequals(registerName, "rsp")) offset = offsetof(user_regs_struct, rsp);
    else if (iequals(registerName, "rip")) offset = offsetof(user_regs_struct, rip);
    else std::cerr << "** unknown register" << std::endl;

    return offset;
}

bool wait_child(pid_t process) {
    int status;
    if (waitpid(process, &status, 0) < 0) errquit("wait");

    if (WIFEXITED(status)) {
        std::cerr << "** child process " << process << " terminiated normally (code " << WEXITSTATUS(status) << ")" << std::endl;
        return false;
    }
    if (WIFSIGNALED(status)) {
        std::cerr << "** child process " << process << " terminiated abnormally (code " << WEXITSTATUS(status) << ")" << std::endl;
        return false;
    }

    return WIFSTOPPED(status);
}
