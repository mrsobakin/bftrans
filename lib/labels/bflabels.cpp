#include "bflabels.h"

#include <cstdint>
#include <string>
#include <charconv>
#include <set>
#include <map>
#include <variant>
#include <iostream>
#include <vector>


std::ostream& operator<<(std::ostream& os, bflabels::Token token) {
    std::visit([&](auto token) {
        if constexpr (std::is_same_v<decltype(token), char>) {
            os << token;
        } else if constexpr (std::is_same_v<decltype(token), bflabels::Label>) {
            os << "var" << token.label_idx;
            if (token.element_idx) {
                os << '(' << token.element_idx << ')';
            }
        } else if constexpr (std::is_same_v<decltype(token), bflabels::Scope>) {
            os << (token == bflabels::Scope::Enter ? '{' : '}');
        }
    }, token);
    return os;
}


namespace bflabels {

ParseResult<std::optional<Token>> Parser::parse_next() {
    if (ch == end) {
        return std::nullopt;
    }

    switch (*ch) {
        case '+':
        case ',':
        case '-':
        case '.':
        case '<':
        case '>':
        case '[':
        case ']':
            return *ch++;

        case '(':
        case ')':
            return std::unexpected(ParseError::UnexpectedParens);

        case '{':
            ++ch;
            return Scope::Enter;

        case '}':
            ++ch;
            return Scope::Exit;

        default: {
            if (isspace(*ch)) {
                ++ch;
                return parse_next();
            }

            if (!isalnum((*ch))) {
                return std::unexpected(ParseError::BadCharacter);
            }

            return parse_label();
        }
    }
}

ParseResult<Label> Parser::parse_label() {
    std::string_view::iterator label_start = ch;

    while (ch != end && isalnum(*ch)) {
        ++ch;
    }

    std::string_view::iterator label_end = ch;

    if (label_start == label_end) {
        return std::unexpected(ParseError::InvalidLabel);
    }

    size_t label_idx = get_label({label_start, label_end});

    skip_whitespaces();

    if (ch == end || *ch != '(') {
        return Label {
            .label_idx = label_idx,
            .element_idx = 0,
        };
    }
    ++ch;

    skip_whitespaces();

    std::string_view::iterator index_start = ch;

    while (ch != end && isdigit(*ch)) {
        ++ch;
    }

    std::string_view::iterator index_end = ch;

    skip_whitespaces();

    if (ch == end || *ch != ')') {
        return std::unexpected(ParseError::ParensUnclosed);
    }
    ++ch;

    size_t element_idx = 0;

    if (index_start != index_end) {
        if (std::from_chars(index_start, index_end, element_idx).ec != std::errc{}) {
            return std::unexpected(ParseError::InvalidIndex);
        }
    }

    return Label {
        .label_idx = label_idx,
        .element_idx = element_idx,
    };
}

void Parser::skip_whitespaces() {
    while (ch != end && isspace(*ch)) {
        ++ch;
    }
}

size_t Parser::get_label(std::string_view ident) {
    if (ident_labels.contains(ident)) {
        return ident_labels.at(ident);
    } else {
        ++next_idx;
        ident_labels[ident] = next_idx;
        return next_idx;
    }
}

ParseResult<std::vector<Token>> Parser::parse() {
    std::vector<Token> tokens;

    while (ch < end) {
        auto token = parse_next();

        if (!token.has_value()) {
            std::cout << (int)(token.error()) << std::endl;
            return std::unexpected(token.error());
        }

        if (!*token) {
            break;
        }

        tokens.push_back(**token);
    }

    return tokens;
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

    // for (auto label : unique_labels) {
    //     if (!layout.label_offsets.contains(label)) {
    //         std::cout << "Label " << label << '\n';
    //         unplaced_labels.insert(label);
    //     } else {
    //         placed_labels.insert(label);
    //     }
    // }

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
