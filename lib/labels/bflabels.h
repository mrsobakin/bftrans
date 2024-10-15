#pragma once

#include <cstdint>
#include <expected>
#include <iostream>
#include <string>
#include <map>
#include <variant>
#include <vector>


namespace bflabels {

using Operation = char;

struct Label {
    size_t label_idx;
    size_t element_idx;
    size_t scope_idx;

    bool operator==(const Label& other) const {
        return label_idx == other.label_idx && scope_idx == other.scope_idx;
    };

    bool operator<(const Label& other) const {
        if (label_idx == other.label_idx) {
            return scope_idx < other.scope_idx;
        }
        return label_idx < other.label_idx;
    };
};

struct LabelUnique {
    size_t label_idx;
    size_t scope_idx;

    bool operator==(const LabelUnique& other) const {
        return label_idx == other.label_idx && scope_idx == other.label_idx;
    }

    bool operator<(const LabelUnique& other) const {
        if (label_idx == other.label_idx) {
            return scope_idx < other.scope_idx; 
        }
        return label_idx < other.label_idx;
    }

    LabelUnique(Label label)
        : label_idx(label.label_idx)
        , scope_idx(label.scope_idx)
    {}
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

class Scopes {
private:
    size_t scope_curr = 0;
    std::vector<size_t> stack = {0};
    std::vector<size_t> parents = {0};

public:
    bool do_coexist(LabelUnique f, LabelUnique s) const;
    size_t get_scope() const { return scope_curr; };
    void add_scope();
    void pop_scope();
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

    std::map<std::string_view, size_t> ident_labels;
    size_t next_idx = 0;

    ParseResult<std::optional<Token>> parse_next();
    ParseResult<Label> parse_label();
    void skip_whitespaces();

    size_t get_label(std::string_view ident);

public:
    Parser(std::string_view code):
        ch(code.begin()),
        end(code.end()) {}

    ParseResult<std::vector<Token>> parse();
};

class BFLCode {
private:
    Scopes scopes;
    std::vector<Token> tokens;

    void relabel_tokens();
    std::map<LabelUnique, size_t> find_lengths() const;
    std::map<LabelUnique, int64_t> find_offsets() const;

public:
    BFLCode(const std::vector<Token>& tokens);

    std::string compile();
};

} // namespace bflabels
