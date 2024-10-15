#pragma once

#include <iostream>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>
#include <map>

namespace pascal {


    struct Position {
        size_t line;
        size_t column;
    };

    using Identifier = std::string;


    enum Keyword {
        And,
        Array,
        Begin,
        Case,
        Const,
        Div,
        Do,
        Downto,
        Else,
        End,
        File,
        For,
        Function,
        Goto,
        If,
        In,
        Label,
        Mod,
        Nil,
        Not,
        Of,
        Or,
        Packed,
        Procedure,
        Program,
        Record,
        Repeat,
        Set,
        Then,
        To,
        Type,
        Until,
        Var,
        While,
        With,
    };

    enum Keysymbol {
        Plus,
        Tilda,
        Star,
        Slash,
        Equal,
        Less,
        Greater,
        LSquare,
        RSquare,
        Dot,
        Comma,
        Colon,
        Semicolon,
        Quote,
        LParen,
        RParen,
        NotEqual,
        LessOrEqual,
        GreaterOrEqual,
        Assign,
        Range,
        Minus,
    };

    std::optional<Keyword> string_to_keyword(std::string_view str); 

    using TokenData = std::variant<Identifier, Keyword, Keysymbol>;

    struct Token {
        Position pos;
        TokenData data;
    };


    class Tokenizer {
        private:
            Position current_position;
            std::vector<Token> parsed_tokens;
            std::string_view::iterator current_character;
            const std::string_view::iterator end;

            void update_current_position();

            void skip_whitespace();

            bool is_EOF();
        
            bool is_special_character();

            Token read_token();
        
            std::string_view read_number();
            
            std::string_view read_word();

            std::string_view read_spec_symbol();
        
            void try_move_caret();
        public:
            Tokenizer(std::string_view code) : current_character(code.begin()), end(code.end()) {}
            ~Tokenizer();
        
            void tokenize();
    };
}
