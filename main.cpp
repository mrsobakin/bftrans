#include <iostream>
#include <fstream>

#include <lib/asm/parser.h>
#include <lib/asm/compiler.h>

int main() {
    std::ifstream ifs("test.bfasm");
    std::string content(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
    );

    auto tokens = bfasm::parse::Tokenizer(content).tokenize();

    if (!tokens) {
        std::cout << tokens.error() << '\n';
        return 1;
    }

    auto res = bfasm::parse::Parser(*tokens).parse();

    if (!res.has_value()) {
        std::cout << res.error() << '\n';
        return 1;
    }

    std::cout << "AST: " << std::endl;
    for (const auto& [name, macro] : *res) {
        std::cout << macro;
    }
    std::cout << std::endl;

    std::cout << "Labels: " << std::endl;
    auto compiled = bfasm::compiler::Compiler(*res);

    compiled.compile();

    for (auto l : compiled.result) {
        std::cout << l;
    }

    std::cout << std::endl;

    std::cout << "Brainfuck: " << std::endl;
    auto bfl = bflabels::BFLCode(compiled.result, bflabels::MemoryLayout{});
    std::cout << bfl.compile();
    std::cout << std::endl;
}
