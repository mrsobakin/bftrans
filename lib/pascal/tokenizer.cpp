#include "tokenizer.h"
#include <cctype>
#include <ios>
#include <optional>
#include <string_view>


namespace pascal {

    bool Tokenizer::is_EOF() {
        return current_character == end;
    }

    void Tokenizer::skip_whitespace() {
        while(current_character != end && *current_character == ' ' && *current_character == '\n') try_move_caret(); 
    }
            
    void Tokenizer::update_current_position() {
        if (*current_character == '\n') {
            ++current_position.line;
            current_position.column = 0;
        }
        else {
            ++current_position.column;
        }
    }

    std::string_view Tokenizer::read_word() {
        std::string_view::iterator word_begin = current_character;
        while (isalnum(*current_character) && current_character != end) try_move_caret();
        return std::string_view(word_begin, current_character);
    }

    bool Tokenizer::is_special_character() {
        return false;
    }

    void Tokenizer::try_move_caret() {
        ++current_character;
        update_current_position();
    }

    std::string_view Tokenizer::read_number() {
        std::string_view::iterator temp_ch = current_character;

    }

    std::optional<Keyword> string_to_keyword(std::string_view str) {
        if (str == "and") return Keyword::And;
        if (str == "array") return Keyword::Array;
        if (str == "begin") return Keyword::Begin;
        if (str == "case") return Keyword::Case;
        if (str == "const") return Keyword::Const;
        if (str == "div") return Keyword::Div;
        if (str == "do") return Keyword::Do;
        if (str == "downto") return Keyword::Downto;
        if (str == "else") return Keyword::Else;
        if (str == "end") return Keyword::End;
        if (str == "file") return Keyword::File;
        if (str == "for") return Keyword::For;
        if (str == "function") return Keyword::Function;
        if (str == "goto") return Keyword::Goto;
        if (str == "if") return Keyword::If;
        if (str == "in") return Keyword::In;
        if (str == "label") return Keyword::Label;
        if (str == "mod") return Keyword::Mod;
        if (str == "nil") return Keyword::Nil;
        if (str == "not") return Keyword::Not;
        if (str == "of") return Keyword::Of;
        if (str == "or") return Keyword::Or;
        if (str == "packed") return Keyword::Packed;
        if (str == "procedure") return Keyword::Procedure;
        if (str == "program") return Keyword::Program;
        if (str == "record") return Keyword::Record;
        if (str == "repeat") return Keyword::Repeat;
        if (str == "set") return Keyword::Set;
        if (str == "then") return Keyword::Then;
        if (str == "to") return Keyword::To;
        if (str == "type") return Keyword::Type;
        if (str == "until") return Keyword::Until;
        if (str == "var") return Keyword::Var;
        if (str == "while") return Keyword::While;
        if (str == "with") return Keyword::With;
        return std::nullopt;
    }

    std::string_view Tokenizer::read_spec_symbol() {
        if (is_EOF()) return std::string_view();
        std::string_view::iterator begin = current_character;
        try_move_caret();
        switch(*begin) {
            case '+':
            case '-':
            case ';':
            case '[':
            case ']':
            case '(':
            case ')':
            case ',':
            case '^':
            case '*':
            case '/':
            case '=':
                break;
            case '.':
                if (*current_character == '.') {
                    try_move_caret();
                }
                break;
            case '<':
                if (*current_character == '>' || *current_character == '=') {
                    try_move_caret();
                }
                break;
            case '>':
                if (*current_character == '=') {
                    try_move_caret();
                }
                break;
            case ':':
                if (*current_character == '=') {
                    try_move_caret();
                }
                break;
            default:
                return std::string_view();
        }
        return std::string_view(begin, current_character);
    }

    Token Tokenizer::read_token() {
        skip_whitespace();
        
        if (isalpha(*current_character)) {
            std::string_view word = read_word();
            std::optional<Keyword> keyword = string_to_keyword(word);
            if (keyword == std::nullopt) {
                return Token(current_position, *keyword);
            } else {
                return Token(current_position, std::string(word));
            }
        }

        if (true) {
            std::string_view number = read_word();
            parsed_tokens.emplace_back(current_position, std::string(number));
        }

        std::string_view special_symbol = read_spec_symbol();
        if (special_symbol == "-") return Token(current_position, Keysymbol::Minus);
        if (special_symbol == "+") return Token(current_position, Keysymbol::Plus);
        if (special_symbol == "~") return Token(current_position, Keysymbol::Tilda);
        if (special_symbol == "*") return Token(current_position, Keysymbol::Star);
        if (special_symbol == "/") return Token(current_position, Keysymbol::Slash);
        if (special_symbol == "=") return Token(current_position, Keysymbol::Equal);
        if (special_symbol == "<") return Token(current_position, Keysymbol::Less);
        if (special_symbol == ">") return Token(current_position, Keysymbol::Greater);
        if (special_symbol == "[") return Token(current_position, Keysymbol::LSquare);
        if (special_symbol == "]") return Token(current_position, Keysymbol::RSquare);
        if (special_symbol == ".") return Token(current_position, Keysymbol::Dot);
        if (special_symbol == ",") return Token(current_position, Keysymbol::Comma);
        if (special_symbol == ":") return Token(current_position, Keysymbol::Colon);
        if (special_symbol == ";") return Token(current_position, Keysymbol::Semicolon);
        if (special_symbol == "\"") return Token(current_position, Keysymbol::Quote);
        if (special_symbol == "(") return Token(current_position, Keysymbol::LParen);
        if (special_symbol == ")") return Token(current_position, Keysymbol::RParen);
        if (special_symbol == "<>") return Token(current_position, Keysymbol::NotEqual);
        if (special_symbol == "<=") return Token(current_position, Keysymbol::LessOrEqual);
        if (special_symbol == ">=") return Token(current_position, Keysymbol::GreaterOrEqual);
        if (special_symbol == ":=") return Token(current_position, Keysymbol::Assign);
        if (special_symbol == "..") return Token(current_position, Keysymbol::Range);
        return Token();
    }

    void Tokenizer::tokenize() {
        while (!is_EOF()) {

        }
    }
}
