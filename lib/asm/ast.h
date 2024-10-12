#pragma once

#include <iostream>
#include <map>
#include <variant>
#include <vector>


namespace bfasm::ast {

    struct Macro;
    
    using Plain = char;
    struct Label;
    struct Use;
    struct If;
    struct While;

    using ASTNode = std::variant<
        Plain,
        Label,
        Use,
        If,
        While
    >;

    using ASTBlock = std::vector<ASTNode>;

    struct Label {
        size_t id;

        bool operator==(const Label& other) const = default;
    };

    struct Use {
        std::string macro_name;
        std::vector<ast::Label> arguments;
        std::vector<ast::Label> return_into;
    };

    struct If {
        Label condition;
        ASTBlock then_block;
        ASTBlock else_block;
    };

    struct While {
        Label condition;
        ASTBlock do_block;
    };

    struct Macro {
        std::string name;
        ast::ASTBlock block;
        std::vector<ast::Label> arguments;
        std::vector<ast::Label> returns;
    };

    using Unit = std::map<std::string, ast::Macro>;

}  // namespace bfasm::ast

template <>
struct std::hash<bfasm::ast::Label> {
    std::size_t operator()(const bfasm::ast::Label& key) const {
        return std::hash<size_t>{}(key.id);
    }
};

std::ostream& operator<<(std::ostream& os, const bfasm::ast::Macro& macro);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::ASTNode& node);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::ASTBlock& block);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::Label& label);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::Use& use);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::If& if_);
std::ostream& operator<<(std::ostream& os, const bfasm::ast::While& while_);
