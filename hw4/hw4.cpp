#include "hw4.hpp"

std::string script;

int main(int argc, char** argv) {

    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; ++i) args[i].assign(argv[i]);

    handleArguments(args);

    if (script.empty()) {
        debugger(std::cin);
    }
    else {
        std::ifstream fin{ script };
        debugger(fin);
    }

    return 0;
}
