#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <variant>
#include <vector>


namespace bflabels {

using Operation = char;

struct Label {
    size_t idx;

    bool operator==(const Label& other) const = default;
    bool operator<(const Label& other) const {
        return idx < other.idx;
    };
};

using Token = std::variant<Label, Operation>;

} // namespace bflabels

template <>
struct std::hash<bflabels::Label> {
    std::size_t operator()(const bflabels::Label& key) const {
        return std::hash<size_t>{}(key.idx);
    }
};

std::ostream& operator<<(std::ostream& os, bflabels::Token token);


namespace bflabels {

class BFLParser {
private:
    std::vector<Token> tokens;

    std::unordered_map<std::string, Label> ident_labels;
    size_t next_idx = 0;

    std::string ident;

    void parse_char(char c);
    void finish();
    void push_ident();

public:
    static std::vector<Token> parse(std::string_view code);
};


struct Range {
    int64_t start;
    int64_t end;

    bool contains(int64_t idx);
};


struct MemoryLayout {
    std::unordered_map<Label, int64_t> label_offsets;
    // std::vector<Range> reserved_ranges;
};


class BFLCode {
private:
    const MemoryLayout& layout;
    const std::vector<Token>& tokens;

    std::map<Label, int64_t> find_offsets();

public:
    BFLCode(const std::vector<Token>& tokens, const MemoryLayout& layout)
        : layout(layout),
          tokens(tokens) {};

    std::string compile();
};

} // namespace bflabels
