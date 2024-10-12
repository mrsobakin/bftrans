#include "bflabels.h"

#include <cstdint>
#include <string>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <variant>
#include <vector>


std::ostream& operator<<(std::ostream& os, bflabels::Token token) {
    std::visit([&](auto token) {
        if constexpr (std::is_same_v<decltype(token), char>) {
            os << token;
        } else if constexpr (std::is_same_v<decltype(token), bflabels::Label>) {
            os << "var" << token.idx;
        }
    }, token);
    return os;
}


namespace bflabels {


bool Range::contains(int64_t idx) {
    return idx >= start && idx <= end;
}


void BFLParser::parse_char(char c) {
    switch (c) {
        case '+':
        case ',':
        case '-':
        case '.':
        case '<':
        case '>':
        case '[':
        case ']':
            if (!ident.empty()) {
                push_ident();
            }
            tokens.push_back(c);
            break;

        case ' ':
        case '\n':
            break;

        default:
            ident.push_back(c);
            break;
    }
}

void BFLParser::finish() {
    if (!ident.empty()) {
        push_ident();
    }
}

void BFLParser::push_ident() {
    if (ident_labels.contains(ident)) {
        tokens.push_back(ident_labels[ident]);
    } else {
        ident_labels[ident] = Label { next_idx };
        tokens.push_back(Label { next_idx });
        ++next_idx;
    }

    ident.clear();
}

std::vector<Token> BFLParser::parse(std::string_view code) {
    BFLParser p;

    for (char c : code) {
        p.parse_char(c);
    }
    p.finish();

    return p.tokens;
}


std::map<Label, int64_t> BFLCode::find_offsets() {
    std::vector<Label> labels;

    for (auto token : tokens) {
        if (const Label* label = std::get_if<Label>(&token)) {
            labels.push_back(*label);
        }
    }

    std::set<Label> unique_labels(labels.begin(), labels.end());

    std::set<Label> placed_labels;
    std::set<Label> unplaced_labels;

    for (auto label : unique_labels) {
        if (!layout.label_offsets.contains(label)) {
            std::cout << "Label " << label << '\n';
            unplaced_labels.insert(label);
        } else {
            placed_labels.insert(label);
        }
    }

    // TODO TODO TODO TODO

    std::map<Label, int64_t> offsets;

    size_t i = 0;
    for (auto label : unplaced_labels) {
        offsets[label] = i++;
    }

    return offsets;
}


std::string BFLCode::compile() {
    std::string code;
    int64_t last_pos = 0;

    auto offsets = find_offsets();

    for (auto token : tokens) {
        std::visit([&](auto token) {
            if constexpr (std::is_same_v<decltype(token), Label>) {
                auto offset = offsets[token];
                char op = last_pos < offset ? '>' : '<';

                for (size_t i = std::abs(last_pos - offset); i != 0; --i) {
                    code += op;
                }

                last_pos = offset;
            } else if constexpr (std::is_same_v<decltype(token), Operation>) {
                code += token;
            }
        }, token);
    }

    return code;
}


} // namespace bflabels
