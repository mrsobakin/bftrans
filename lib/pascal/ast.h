#include <iostream>
#include <vector>
#include "tokenizer.h"


namespace pascal::ast {
    
    enum NodeType { };

    struct ASTNode {
        Token data;
        std::vector<ASTNode> children;
    };

    class AST {
        private:

            ASTNode* root;

            std::vector<Token>::iterator current_token;

            std::vector<Token>& tokens;

            ASTNode* parse_token(ASTNode* cur);

        public:

            AST(std::vector<Token>& tokens) : tokens(tokens), current_token(tokens.begin()) { };
            ~AST();

            void build();
    };
}
