#include "bflabels.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <charconv>
#include <set>
#include <list>
#include <map>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <vector>
#include "../utils.h"


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

bool Scopes::lifetimes_intersect(size_t f, size_t s) const {
    if (f > s) {
        std::swap(f, s);
    }

    while (s != 0) {
        if (f == s) {
            return true;
        }
        s = parents[s];
    }

    return false;
}

void Scopes::add_scope() {
    ++scope_curr;
    parents.push_back(stack.back());
    stack.push_back(scope_curr);
}

void Scopes::pop_scope() {
    stack.pop_back();
}

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

void BFLCode::relabel_tokens() {
    const size_t SENTINEL = std::numeric_limits<size_t>::max();

    Scopes scopes;

    std::unordered_map<size_t, size_t> relabel_map({});
    size_t next_label_idx = 0;
    std::stack<size_t> labels;

    labels.push(SENTINEL);

    for (auto& token : tokens) {
        std::visit(overloaded {
            [&](Label& label) {
                label.scope_idx = scopes.get_scope();

                if (relabel_map.contains(label.label_idx)) {
                    label.label_idx = relabel_map[label.label_idx];
                } else {
                    relabel_map[label.label_idx] = next_label_idx;
                    label.label_idx = next_label_idx;
                    ++next_label_idx;
                }
            },
            [&](Scope scope) {
                if (scope == Scope::Enter) {
                    scopes.add_scope();
                    labels.push(SENTINEL);
                } else if (scope == Scope::Exit) {
                    scopes.pop_scope();
                    while (labels.top() != SENTINEL) {
                        relabel_map.erase(labels.top());
                        labels.pop();
                    }
                    labels.pop();
                }
            },
            [&](char plain) {}
        }, token);
    }
}

std::map<Label, int64_t> BFLCode::find_offsets() const {
    std::vector<Label> labels;

    for (auto token : tokens) {
        if (const Label* label = std::get_if<Label>(&token)) {
            labels.push_back(*label);
        }
    }

    std::set<Label> unique_labels(labels.begin(), labels.end());

    std::vector<std::vector<int64_t>> weights(unique_labels.size(), std::vector<int64_t>(unique_labels.size()));

    for (size_t i = 1; i < labels.size(); ++i) {
        if (scopes.lifetimes_intersect(labels[i - 1].scope_idx, labels[i].scope_idx)) {
            weights[labels[i].label_idx][labels[i - 1].label_idx]++;
            weights[labels[i - 1].label_idx][labels[i].label_idx]++;
        }
    }

    std::list<Label> ordered;

    size_t len = 0;

    for (auto& to_add : unique_labels) {
        uint64_t left_w = 0, right_w = 0;
        for (auto& i : ordered) {
            left_w += weights[to_add.label_idx][i.label_idx];
        }

        uint64_t left_loss = 0, right_loss = 0;
        uint64_t agregate = 0;
        for (auto& i : ordered) {
            agregate += weights[to_add.label_idx][i.label_idx];
            left_loss += agregate;
        }

        size_t pos = 0, min_pos = 0;
        uint64_t min_loss = std::numeric_limits<uint64_t>::max();
        for (auto& i : reverse(ordered)) {
            if (min_loss > left_loss + right_loss) {
                min_loss = left_loss + right_loss;
                min_pos = pos;
            }
            ++pos;
            left_w -= weights[to_add.label_idx][i.label_idx];
            left_loss -= left_w;
            right_loss += right_w;
            right_w += weights[to_add.label_idx][i.label_idx];
        }
        if (min_loss > left_loss + right_loss) {
            min_loss = left_loss + right_loss;
            min_pos = pos;
        }

        ordered.insert(std::prev(ordered.end(), min_pos), to_add);
    }

    std::map<Label, int64_t> offsets;
    int64_t idx = 0;
    for (auto& i : ordered) {
        offsets[i] = idx;
    }

    return offsets;
}

std::string BFLCode::compile() {
    std::string code;
    int64_t last_pos = 0;

    relabel_tokens();

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
