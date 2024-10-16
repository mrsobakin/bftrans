#pragma once

#include <__expected/expected.h>
#include <cstdint>
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


    enum NumberType {
        SignedInteger,
        SignedReal,
        UnsignedInteger,
        UnsignedReal,
    };

    struct Number {
        NumberType type;
        std::variant<int64_t, uint64_t> integer_part;
        uint64_t float_part;
        std::variant<int64_t, uint64_t> fractional_part;
    };

    std::optional<Keyword> string_to_keyword(std::string_view str); 
    
    std::optional<NumberType> string_to_number_type(std::string_view str); 
    
    std::optional<Keysymbol> string_to_keysymbol(std::string_view str); 

    using TokenData = std::variant<Identifier, Keyword, Keysymbol, Number>;

    struct Token {
        Position pos;
        TokenData data;
    };

    enum ErrorType {
        UnexpectedToken,
        IncorrectNumber,
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

            std::expected<Token, ErrorType> read_token();
        
            std::expected<Number, ErrorType> read_number();
            
            std::string_view read_word();

            std::string_view read_spec_symbol();
        
            void next();

            bool is_number();
        public:
            Tokenizer(std::string_view code) : current_character(code.begin()), end(code.end()) {}
            ~Tokenizer();
        
            void tokenize();
            void print();
    };
}
