#include "debugger.hpp"

std::string program_record;

int main(int argc, char** argv) {

    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; ++i) args[i].assign(argv[i]);

    auto [script, program] = handleArguments(args);
    program_record = program;

    std::unique_ptr<Debugger> debugger{};

    if (script.empty()) {
        if (program_record.empty()) {
            debugger = std::make_unique<Debugger>();
        }
        else {
            debugger = std::make_unique<Debugger>(program_record);
        }
        debugger->mainLoop();
    }
    else {
        std::ifstream fin{ script };

        if (program_record.empty()) {
            debugger = std::make_unique<Debugger>(fin);
        }
        else {
            debugger = std::make_unique<Debugger>(program_record, fin);
        }
        debugger->mainLoop();
    }

    return 0;
}
