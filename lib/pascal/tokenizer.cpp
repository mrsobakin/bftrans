#include "tokenizer.h"
#include <__expected/unexpected.h>
#include <cctype>
#include <ios>
#include <optional>
#include <string>
#include <string_view>


namespace pascal {

    bool Tokenizer::is_EOF() {
        return current_character == end;
    }

    void Tokenizer::skip_whitespace() {
        while(current_character != end && *current_character == ' ' && *current_character == '\n') next(); 
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
        while (isalnum(*current_character) && current_character != end) next();
        return std::string_view(word_begin, current_character);
    }

    bool Tokenizer::is_special_character() {
        return false;
    }

    void Tokenizer::next() {
        ++current_character;
        update_current_position();
    }

    std::expected<Number, ErrorType> Tokenizer::read_number() {
        std::string_view::iterator begin = current_character; 
        unsigned char numbertype = 0;
        bool is_real = false;
        bool is_signed = false;
        auto integer_part_begin = begin;
        auto fractional_part_begin = begin;
        auto float_part_begin = begin;
        std::string_view integer_part;
        std::string_view float_part;
        std::string_view fractional_part;
        if (*begin == '-' || *begin == '+') {
            is_signed = true;
        }
        bool is_correct = false;
        Number result = Number();
        next();
        while (current_character != end && isdigit(*current_character)) {
            next();
            is_correct = true;
        }

        if (!is_correct) return std::unexpected(ErrorType::IncorrectNumber);

        if (current_character != end) {
            
            if (*current_character != '.' && *current_character != 'e') return std::unexpected(ErrorType::IncorrectNumber);
            integer_part = std::string_view(integer_part_begin, current_character);
            is_real = true;
            if (*current_character == 'e') {
                next();
                switch (*current_character) {
                    case '-':
                    case '+':
                        fractional_part_begin = current_character;
                        next();
                        if (current_character == end || !isdigit(*current_character)) return std::unexpected(ErrorType::IncorrectNumber);
                        break;
                    default:
                        if (!isdigit(*current_character)) return std::unexpected(ErrorType::IncorrectNumber);
                        fractional_part_begin = current_character;
                        next();
                }
                while (current_character != end && isdigit(*current_character)) next();
                fractional_part = std::string_view(fractional_part_begin, current_character);
            }
            else {
                float_part_begin = current_character;
                while (current_character != end && isdigit(*current_character)) next();
                if (current_character != end && *current_character == 'e') {
                    float_part = std::string_view(float_part_begin, current_character);
                    next();
                    switch (*current_character) {
                        case '-':
                        case '+':
                            fractional_part_begin = current_character;
                            next();
                            if (current_character == end || !isdigit(*current_character)) return std::unexpected(ErrorType::IncorrectNumber);
                            break;
                        default:
                            if (!isdigit(*current_character)) return std::unexpected(ErrorType::IncorrectNumber);
                            fractional_part_begin = current_character;
                            next();
                    }
                    while (current_character != end && isdigit(*current_character)) next();
                    fractional_part = std::string_view(fractional_part_begin, current_character);
                }
                else {
                    float_part = std::string_view(float_part_begin, current_character);
                }
            }
        }
        if (is_signed && is_real) {
            result.type = NumberType::SignedReal;
            result.integer_part = std::stoll(std::string(integer_part));
            result.float_part = std::stoull(std::string(float_part));
            result.fractional_part = std::stoll(std::string(fractional_part));
        }
        if (!is_signed && is_real) {
            result.type = NumberType::UnsignedReal;
            result.integer_part = std::stoull(std::string(integer_part));
            result.float_part = std::stoull(std::string(float_part));
            result.fractional_part = std::stoll(std::string(fractional_part));
        }
        if (is_signed && !is_real) {
            result.type = NumberType::SignedInteger;
            result.integer_part = std::stoll(std::string(integer_part));
            result.float_part = std::stoull(std::string(float_part));
            result.fractional_part = std::stoll(std::string(fractional_part));
        }
        if (!is_signed && !is_real) {
            result.type = NumberType::UnsignedInteger;
            result.integer_part = std::stoull(std::string(integer_part));
            result.float_part = std::stoull(std::string(float_part));
            result.fractional_part = std::stoll(std::string(fractional_part));
        }
        return result;
    }

    std::optional<NumberType> string_to_number_type(std::string_view str) {
        auto num_begin = str.begin();
        auto num_end = str.end();
        auto iter = num_begin;
        unsigned char number_type = 0;
        
        if (*num_begin == '+' || *num_begin == '-') number_type |= (1 << 0);
        ++iter;
        while (iter != num_end && isdigit(*iter)) {
            ++iter;
        }
        if (iter != num_end) {
            if (*iter == 'e' || *iter == '.') {
                number_type |= (1 << 1);
            }
        }
        switch (number_type) {
            case 0: return NumberType::UnsignedInteger;
            case 1: return NumberType::SignedInteger;
            case 2: return NumberType::UnsignedReal;
            case 3: return NumberType::SignedReal;
            default: return std::nullopt;
        }
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
        next();
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
                    next();
                }
                break;
            case '<':
                if (*current_character == '>' || *current_character == '=') {
                    next();
                }
                break;
            case '>':
                if (*current_character == '=') {
                    next();
                }
                break;
            case ':':
                if (*current_character == '=') {
                    next();
                }
                break;
            default:
                return std::string_view();
        }
        return std::string_view(begin, current_character);
    }


    std::optional<Keysymbol> string_to_keysymbol(std::string_view special_symbol) {
        if (special_symbol == "-") return Keysymbol::Minus;
        if (special_symbol == "+") return Keysymbol::Plus;
        if (special_symbol == "~") return Keysymbol::Tilda;
        if (special_symbol == "*") return Keysymbol::Star;
        if (special_symbol == "/") return Keysymbol::Slash;
        if (special_symbol == "=") return Keysymbol::Equal;
        if (special_symbol == "<") return Keysymbol::Less;
        if (special_symbol == ">") return Keysymbol::Greater;
        if (special_symbol == "[") return Keysymbol::LSquare;
        if (special_symbol == "]") return Keysymbol::RSquare;
        if (special_symbol == ".") return Keysymbol::Dot;
        if (special_symbol == ",") return Keysymbol::Comma;
        if (special_symbol == ":") return Keysymbol::Colon;
        if (special_symbol == ";") return Keysymbol::Semicolon;
        if (special_symbol == "\"") return Keysymbol::Quote;
        if (special_symbol == "(") return Keysymbol::LParen;
        if (special_symbol == ")") return Keysymbol::RParen;
        if (special_symbol == "<>") return Keysymbol::NotEqual;
        if (special_symbol == "<=") return Keysymbol::LessOrEqual;
        if (special_symbol == ">=") return Keysymbol::GreaterOrEqual;
        if (special_symbol == ":=") return Keysymbol::Assign;
        if (special_symbol == "..") return Keysymbol::Range;
        return std::nullopt;
    }

    bool Tokenizer::is_number() {
        auto temp_iter = current_character;
        if (temp_iter == end) return false;
        if (*temp_iter == '+' || *temp_iter == '-') {
            ++temp_iter;
            if (temp_iter == end || !isdigit(*temp_iter)) return false;
        }
        else if (isdigit(*temp_iter)) {
            ++temp_iter;
            if (temp_iter == end || !isdigit(*temp_iter)) return false;
        }
        else return false;
        return true;
    }

    std::expected<Token, ErrorType> Tokenizer::read_token() {
        skip_whitespace();
        
        if (isalpha(*current_character)) {
            std::string_view word = read_word();
            std::optional<Keyword> keyword = string_to_keyword(word);
            if (keyword == std::nullopt) {
                return Token(current_position,  *keyword);
            } else {
                return Token(current_position, std::string(word));
            }
        }

        if (is_number()) {
            auto number = read_number();
            if (number == std::unexpected(ErrorType::IncorrectNumber)) return std::unexpected(ErrorType::UnexpectedToken);
            return Token(current_position, *number);
        }

        std::string_view special_symbol = read_spec_symbol();
        std::optional<Keysymbol> key_symbol = string_to_keysymbol(special_symbol);
        if (key_symbol == std::nullopt) return std::unexpected(ErrorType::UnexpectedToken);
        return Token(current_position, *key_symbol);
        return std::unexpected(ErrorType::UnexpectedToken); 
    }

    void Tokenizer::tokenize() {
        while (current_character != end) {
            auto token = read_token();
            if (token == std::unexpected(ErrorType::UnexpectedToken)) {
                std::cerr << "Unexpected token in line " << current_position.line << " column " << current_position.column << '\n';
                break;
            }
            parsed_tokens.emplace_back(*token);
        }
    }

    void Tokenizer::print() {
        for (const auto& x : parsed_tokens) std::cout << x.data.index() << ' ';
        std::cout << '\n';
    }
}
