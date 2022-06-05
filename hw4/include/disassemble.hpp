#ifndef _DISASSEMBLE_HPP
#define _DISASSEMBLE_HPP

#include <vector>
#include <string>
#include <utility>

#include <capstone/capstone.h>

#include "utilities.hpp"

class Disassembler {
public:
    using code_t = std::vector<uint8_t>;
    using addr_t = uint64_t;
    using instr_t = std::tuple<addr_t, code_t, std::string, std::string>;

    Disassembler(cs_arch arch, cs_mode mode);
    ~Disassembler();
    std::pair<instr_t, addr_t> disasm(const code_t& code, addr_t address);

private:
    csh handle;
};

#endif // !_DISASSEMBLE_HPP
