#pragma once

#include <cstdint>
#include <expected>
#include <iostream>
#include <string>
//#include <unordered_map>
#include <map>
#include <variant>
#include <vector>


namespace bflabels {

using Operation = char;

struct Label {
    size_t label_idx;
    size_t element_idx;
    size_t scope_idx;

    bool operator==(const Label& other) const = default;
    bool operator<(const Label& other) const {
        return label_idx < other.label_idx;
    };
};

enum class Scope {
    Enter,
    Exit,
};

using Token = std::variant<Operation, Label, Scope>;

} // namespace bflabels

template <>
struct std::hash<bflabels::Label> {
    std::size_t operator()(const bflabels::Label& key) const {
        return std::hash<size_t>{}(key.label_idx);
    }
};

std::ostream& operator<<(std::ostream& os, bflabels::Token token);


namespace bflabels {

class Context {
private:
    size_t scope_cur;
    std::vector<size_t> scope_stack;
    std::vector<size_t> scope_parents;

public:
    Context()
        : scope_cur(0)
        , scope_stack({0})
        , scope_parents({0})
    {}

    Context(Context& other) = default;
    Context& operator=(Context &other) = default;
    Context(Context&& other) = default;
    Context& operator=(Context&& other) = default;
    
    size_t get_scope() const;
    bool adjacent(size_t f, size_t s) const;
    void add_scope();
    void pop_scope();
};

class Unit {
public:
    std::vector<Token> tokens;
    Context context;

    Unit(std::vector<Token> tokens, Context context)
        : tokens(tokens)
        , context(context)
    {}
};

enum class ParseError {
    InvalidLabel,
    BadCharacter,
    InvalidIndex,
    UnexpectedParens,
    ParensUnclosed,
};

template <typename T>
using ParseResult = std::expected<T, ParseError>;

class Parser {
private:
    std::string_view::iterator ch;
    const std::string_view::iterator end;

    Context context;

    std::map<std::tuple<std::string_view, size_t>, size_t> ident_labels;
    size_t next_idx = 0;

    ParseResult<std::optional<Token>> parse_next();
    ParseResult<Label> parse_label();
    void skip_whitespaces();

    size_t get_label(std::string_view ident);

public:
    Parser(std::string_view code)
        : ch(code.begin())
        , end(code.end())
    {}

    ParseResult<Unit> parse();
};


class BFLCode {
private:
    Unit unit;

    void import_tokens(std::vector<Token> tokens);
    std::map<Label, int64_t> find_offsets();

public:
    BFLCode(const std::vector<Token>& tokens);

    std::string compile();
};

} // namespace bflabels
