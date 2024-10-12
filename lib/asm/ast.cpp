#include "ast.h"

#include <iostream>


std::ostream& operator<<(std::ostream& os, const bfasm::ast::Macro& macro) {
    os << "MACRO " << macro.name << " (";

    for (const auto& argument : macro.arguments) {
        os << ' ' << argument;
    }

    if (!macro.returns.empty()) {
        os << " ->";
        for (const auto& ret : macro.returns) {
            os << ' ' << ret;
        }
    }

    os << "): " << macro.block << '\n';

    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::ASTNode& node) {
    std::visit([&](const auto& node) {
        os << node;
    }, node);
    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::ASTBlock& block) {
    for (const auto& node : block) {
        os << node;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::Label& label) {
    os << "《var" << label.id << "》";
    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::Use& use) {
    os << "USE " << use.macro_name << " (";

    for (const auto& argument : use.arguments) {
        os << ' ' << argument;
    }

    if (!use.return_into.empty()) {
        os << " ->";
        for (const auto& ret : use.return_into) {
            os << ' ' << ret;
        }
    }

    os << ") ";

    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::If& if_) {
    os << "IF " << if_.condition << " { " << if_.then_block << " } ";

    if (!if_.else_block.empty()) {
        os << "ELSE { " << if_.else_block << " } ";
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::ast::While& while_) {
    os << "WHILE " << while_.condition << " { " << while_.do_block << " } ";

    return os;
}
