#include "tokenizer.h"

#include <iostream>
#include <variant>
#include <expected>
#include "utils.h"


namespace bfasm::parse {
    const Position Position::END = Position {(size_t)(-1), (size_t)(-1)};

    bool Tokenizer::next_is(char next) {
        return (ch + 1) != end && (*(ch + 1) == next);
    }

    void Tokenizer::try_push_ident() {
        if (!ident_buf.empty()) {
            if (ident_buf == "MACRO") {
                tokens.emplace_back(Keyword::Macro, ident_pos);
            } else if (ident_buf == "IF") {
                tokens.emplace_back(Keyword::If, ident_pos);
            } else if (ident_buf == "ELSE") {
                tokens.emplace_back(Keyword::Else, ident_pos);
            } else if (ident_buf == "USE") {
                tokens.emplace_back(Keyword::Use, ident_pos);
            } else if (ident_buf == "WHILE") {
                tokens.emplace_back(Keyword::While, ident_pos);
            } else {
                tokens.emplace_back(std::move(ident_buf), ident_pos);
                return;
            }
            ident_buf.clear();
        }
    }

    void Tokenizer::skip_line() {
        while (*ch++ != '\n');
    }

    std::optional<TokenizeError> Tokenizer::parse_char() {
        ++pos.column;

        if (std::isalnum(*ch)) {
            if (ident_buf.empty()) {
                ident_pos = pos;
            }
            ident_buf += *ch;
            return std::nullopt;
        }

        try_push_ident();

        switch (*ch) {
            case ' ':
            case '\t':
                return std::nullopt;

            case '\n':
                pos.column = 0;
                ++pos.line;
                return std::nullopt;

            case ':':
                tokens.emplace_back(Control::Semicolon, pos);
                return std::nullopt;

            case '(':
                tokens.emplace_back(Control::LParen, pos);
                return std::nullopt;

            case ')':
                tokens.emplace_back(Control::RParen, pos);
                return std::nullopt;

            case '{':
                tokens.emplace_back(Control::LCurly, pos);
                return std::nullopt;

            case '}':
                tokens.emplace_back(Control::RCurly, pos);
                return std::nullopt;

            case '-':
                if (next_is('>')) {  // ->
                    ++pos.column;
                    ++ch;
                    tokens.emplace_back(Control::Arrow, pos);
                } else {
                    tokens.emplace_back(*ch, pos);
                }
                return std::nullopt;

            case '+':
            case '[':
            case ']':
            case '.':
            case ',':
                tokens.emplace_back(*ch, pos);
                return std::nullopt;

            case '<':
            case '>':
                return TokenizeError(pos.line, pos.column, "Raw data pointer modifying (`><`) is not allowed");

            case '#':
                skip_line();
                return std::nullopt;
        }

        return TokenizeError(pos.line, pos.column, "Invalid character");
    }

    TokenizeResult<std::vector<ParsedToken>> Tokenizer::tokenize() {
        while (ch != end) {
            if (auto err = parse_char()) {
                return std::unexpected(err.value());
            }
            ++ch;
        }
        try_push_ident();
        return std::move(tokens);
    }

}  // namespace bfasm::parse


std::ostream& operator<<(std::ostream& os, const bfasm::parse::Token& token) {
    using namespace bfasm::parse;

    std::visit(overloaded {
        [&](Plain p) {
            os << p;
        },
        [&](const Identifier& ident) {
            os << ident << ' ';
        },
        [&](Control control) {
            switch (control) {
                case Control::LCurly:
                    os << '{';
                    break;
                case Control::RCurly:
                    os << '}';
                    break;
                case Control::LParen:
                    os << '(';
                    break;
                case Control::RParen:
                    os << ')';
                    break;
                case Control::Arrow:
                    os << "->";
                    break;
                case Control::Semicolon:
                    os << ':';
                    break;
            }
        },
        [&](Keyword keyword) {
            switch (keyword) {
                case Keyword::Macro:
                    os << "MACRO";
                    break;
                case Keyword::If:
                    os << "IF";
                    break;
                case Keyword::Else:
                    os << "ELSE";
                    break;
                case Keyword::While:
                    os << "WHILE";
                    break;
                case Keyword::Use:
                    os << "USE";
                    break;
            }
        },
    }, token);

    return os;
}

std::ostream& operator<<(std::ostream& os, const bfasm::parse::TokenizeError& error) {
    if (error.pos != bfasm::parse::Position::END) {
        os << "Error at line " << error.pos.line << " column " << error.pos.column << ": ";
    } else {
        os << "Error : ";
    }

    os << error.msg;
    // std::visit([&] (const auto& error) {
    //     os << error;
    // }, error.msg);

    return os;
}
