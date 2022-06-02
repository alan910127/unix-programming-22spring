#include "disassemble.hpp"

Disassembler::Disassembler(cs_arch arch, cs_mode mode) : handle{} {
    if (cs_open(arch, mode, &(this->handle)) != CS_ERR_OK) errquit("capstone");
}

Disassembler::~Disassembler() {
    cs_close(&(this->handle));
}

auto Disassembler::disasm(const code_t& code, addr_t address) -> std::pair<instr_t, addr_t> {
    cs_insn* insn;
    size_t count = cs_disasm(this->handle, code.data(), code.size(), address, 0, &insn);

    instr_t instruction;

    if (count > 0) {
        instruction = { insn[0].address, code_t{ insn[0].bytes, insn[0].bytes + insn[0].size }, insn[0].mnemonic, insn[0].op_str };
        cs_free(insn, count);
    }

    auto [addr, bytes, m, o] = instruction;
    addr_t nextAddr = addr + bytes.size();

    return { instruction, nextAddr };
}
