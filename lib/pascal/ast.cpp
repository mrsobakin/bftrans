#include <__expected/unexpect.h>
#include <iostream>
#include <variant>
#include <vector>
#include "ast.h"
#include "tokenizer.h"

namespace pascal::ast {
    std::vector<ASTNode> AST::parse_program_heading(ASTNode* parent) {

        std::vector<ASTNode> children;

        if (std::holds_alternative<pascal::Identifier>(current_token->data)) return std::vector<ASTNode>();
        children.emplace_back(*current_token);

        ++current_token; 
        if (!std::holds_alternative<pascal::Keysymbol>(current_token->data)) {

            pascal::Keysymbol symbol = std::get<pascal::Keysymbol>(current_token->data);
            if (symbol != pascal::Keysymbol::LParen) return  std::vector<ASTNode>();
            children.emplace_back(*current_token);
            ++current_token;
            bool is_parameter_list_correct = false;
            while (!std::holds_alternative<pascal::Identifier>(current_token->data)) {
                children.emplace_back(*current_token);
                ++current_token;
                if (!std::holds_alternative<pascal::Keysymbol>(current_token->data)) {
                    auto symbol = std::get<pascal::Keysymbol>(current_token->data);
                    if (symbol != pascal::Keysymbol::Comma) break;
                    ++current_token;
                }
                else return std::vector<ASTNode>();
            }

            if (std::holds_alternative<pascal::Keysymbol>(current_token->data)) return std::vector<ASTNode>();
            symbol = std::get<Keysymbol>(current_token->data);
            if (symbol != Keysymbol::RParen) return std::vector<ASTNode>();
            children.emplace_back(*current_token);

            ++current_token;
        }
        if (std::holds_alternative<Keysymbol>(current_token->data)) return std::vector<ASTNode>();
        auto symbol = std::get<Keysymbol>(current_token->data);
        if (symbol != Keysymbol::Semicolon) return std::vector<ASTNode>();
        children.emplace_back(*current_token);

        ++current_token;
        if (std::holds_alternative<Keyword>(current_token->data)) return std::vector<ASTNode>();

        auto keyword = std::get<Keyword>(current_token->data);
        if (keyword != Keyword::Begin) return std::vector<ASTNode>();

        return children; 
    }

    std::vector<ASTNode> AST::parse_block(ASTNode* parent) {
        std::vector<ASTNode> block;

        if (std::holds_alternative<Keyword>(current_token->data)) return std::vector<ASTNode>();
        Keyword keyword = std::get<Keyword>(current_token->data);
        if (keyword == Keyword::Label) {
            
        }

        ++current_token;
    }

    std::expected<std::vector<ASTNode>, ErrorType> AST::parse_labels(ASTNode* parent) {
        std::vector<ASTNode> labels;
        if (std::holds_alternative<Keyword>(current_token->data)) return std::vector<ASTNode>();
        Keyword keyword = std::get<Keyword>(current_token->data);
        if (keyword != Keyword::Label) return std::vector<ASTNode>();
        labels.emplace_back(*current_token);
        ++current_token;

        if (std::holds_alternative<Identifier>(current_token->data)) return std::unexpected(ast::ErrorType::IncorrectLabel);
        labels.emplace_back(*current_token);
        ++current_token;

        while (!std::holds_alternative<Keysymbol>(current_token->data)) {
            auto symbol = std::get<Keysymbol>(current_token->data);
            if (symbol == Keysymbol::Semicolon) return labels;
            if (symbol != Keysymbol::Comma) return std::unexpected(ast::ErrorType::IncorrectLabel);

            labels.emplace_back(*current_token);
            ++current_token;

            if (std::holds_alternative<Number>(current_token->data)) return std::unexpected(ast::ErrorType::IncorrectLabel);
            labels.emplace_back(*current_token);
        }
        return std::unexpected(ast::ErrorType::IncorrectLabel);
    }

    std::expected<std::vector<ASTNode>, ErrorType> AST::parse_consts() {
        std::vector<ASTNode> consts;
        if (std::holds_alternative<Keyword>(current_token->data)) return consts;
        auto keyword = std::get<Keyword>(current_token->data);

        if (keyword != Keyword::Const) return consts;
        
        consts.emplace_back(*current_token);
        ++current_token;
        if (std::holds_alternative<Identifier>(current_token->data) &&
            std::holds_alternative<Number>(current_token->data)) return std::unexpected(IncorrectConst);
        consts.emplace_back(*current_token);
        ++current_token;
        while(!std::holds_alternative<Keysymbol>(current_token->data)) {
            auto symbol = std::get<Keysymbol>(current_token->data);
            if (symbol == Keysymbol::Semicolon) {
                consts.emplace_back(*current_token);
                ++current_token;
                return consts;
            }
            if (symbol != Keysymbol::Comma) break;
            consts.emplace_back(*current_token);
            ++current_token;
            
            if (std::holds_alternative<Identifier>(current_token->data) &&
                std::holds_alternative<Number>(current_token->data)) return std::unexpected(IncorrectConst);
            consts.emplace_back(*current_token);
            ++current_token;
        }
        return std::unexpected(IncorrectConst);
    }

    std::expected<std::vector<ASTNode>, ErrorType> AST::parse_types() {
        std::vector<ASTNode> types;
        if (std::holds_alternative<Keyword>(current_token->data)) return types;
        
        auto keyword = std::get<Keyword>(current_token->data);
        if (keyword != Keyword::Type) return types;
        
        types.emplace_back(*current_token);
        ++current_token;


    }

    ASTNode* AST::parse_program(ASTNode* parent) {
        if (parent != root) return nullptr;
        
        ASTNode* program_node = new ASTNode(*current_token);
        ++current_token;

        auto program_heading = parse_program_heading(program_node);
        if (program_heading.empty()) return nullptr;

        program_node->children.insert(program_node->children.end(), program_heading.begin(), program_heading.end());
        
        auto labels = parse_labels(nullptr);
        if (labels == std::unexpected(ErrorType::IncorrectLabel)) return nullptr;

        program_node->children.insert(program_node->children.end(), labels->begin(), labels->end());

        auto consts = parse_consts();
        if (consts == std::unexpected(IncorrectConst)) return nullptr;

        program_node->children.insert(program_node->children.end(), consts->begin(), consts->end());
    }
        

    ASTNode* AST::parse_token(ASTNode* parent) {
        auto tokenData = current_token->data;
        if (!std::holds_alternative<pascal::Keyword>(tokenData)) {
            if (std::get<pascal::Keyword>(tokenData) == pascal::Keyword::Program) {
            }
        }
    }

    void AST::build() {
    }
}
