#pragma once

#include <variant>
#include <string>
#include <vector>
#include <expected>
#include <optional>


// Code Example:
//
// ```bfasm
// MACRO copy (x y -> z):
//     temp0[-]
//     x[-]
//     y[x+temp0+y-]
//     temp0[y+temp0-]
// ```


namespace bfasm::parse {
    using Plain = char;
    using Identifier = std::string;
    enum class Keyword {
        Macro,
        If,
        Else,
        While,
        Use,
    };
    enum class Control {
        LCurly,
        RCurly,
        LParen,
        RParen,
        Semicolon,
        Arrow,
    };

    struct Position {
        size_t line;
        size_t column;

        const static Position END;

        bool operator==(const Position& other) const = default;
    };

    using Token = std::variant<Plain, Identifier, Keyword, Control>;

    struct ParsedToken {
        Token data;
        Position pos;
    };

    struct TokenizeError {
        Position pos;
        const char* msg;

        TokenizeError(size_t line, size_t column, const char* msg) :
            pos({line, column}),
            msg(msg) {}
    };

    template <typename T>
    using TokenizeResult = std::expected<T, TokenizeError>;

    class Tokenizer {
      private:
        std::string_view::iterator ch;
        const std::string_view::iterator end;

        std::vector<ParsedToken> tokens;
        Position pos = {1, 0};
        std::string ident_buf;
        Position ident_pos;

      private:
        std::optional<TokenizeError> parse_char();
        void try_push_ident();
        bool next_is(char next);
        void skip_line();

      public:
        Tokenizer(std::string_view code) :
            ch(code.begin()),
            end(code.end()) {}

        TokenizeResult<std::vector<ParsedToken>> tokenize();
    };
}  // namespace bfasm::parse


std::ostream& operator<<(std::ostream& os, const bfasm::parse::Token& token);
std::ostream& operator<<(std::ostream& os, const bfasm::parse::TokenizeError& err);
