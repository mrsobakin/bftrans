#include <iostream>
#include <vector>
#include "tokenizer.h"


namespace pascal::ast {
    
    enum NodeType { };

    struct ASTNode {
        ASTNode(Token data) : data(data), children(std::vector<ASTNode>()) { }
        Token data;
        std::vector<ASTNode> children;
    };

    enum ErrorType {
        IncorrectLabel,
        IncorrectConst,
        IncorrectVar,
        IncorrectType,
    };

    class AST {
        private:

            ASTNode* root;

            std::vector<Token>::iterator current_token;

            std::vector<Token>& tokens;

            ASTNode* parse_token(ASTNode* parent);

            ASTNode* parse_uses(ASTNode* parent);

            std::vector<ASTNode> parse_program_heading(ASTNode* parent);

            std::vector<ASTNode> parse_block(ASTNode* parent);

            std::expected<std::vector<ASTNode>, ErrorType> parse_labels(ASTNode* parent);

            std::expected<std::vector<ASTNode>, ErrorType> parse_consts();

            std::expected<std::vector<ASTNode>, ErrorType> parse_types();

            std::expected<std::vector<ASTNode>, ErrorType> parse_vars();

            ASTNode* parse_program_parameter_list(ASTNode* parent);

            ASTNode* parse_procedure(ASTNode* parent);

            ASTNode* parse_function(ASTNode* parent);

            ASTNode* parse_if(ASTNode* parent);

            ASTNode* parse_while(ASTNode* parent);

            ASTNode* parse_program(ASTNode* parent);
        public:

            AST(std::vector<Token>& tokens) : tokens(tokens), current_token(tokens.begin()) { };
            ~AST();

            void build();
    };
}
