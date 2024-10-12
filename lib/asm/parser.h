#pragma once

#include <vector>
#include <sstream>
#include <expected>
#include <unordered_map>

#include "ast.h"
#include "utils.h"
#include "tokenizer.h"


namespace bfasm::parse {
    struct ParseError {
        Position pos;
        std::variant<const char*, std::string> msg;

        constexpr ParseError(Position pos, const char* msg) :
            pos(pos),
            msg(msg) {}

        ParseError(Position pos, std::string&& msg) :
            pos(pos),
            msg(std::move(msg)) {}
    };

    template <typename T>
    using ParseResult = std::expected<T, ParseError>;

    struct Signature {
        Identifier name;
        std::vector<ast::Label> arguments;
        std::vector<ast::Label> returns;
    };

    class LabelDispatcher {
        std::unordered_map<std::string_view, size_t> ids;
        size_t last_id = 0;

      public:
        void clear_locals();
        ast::Label get(std::string_view ident);
    };

    class Parser {
      private:
        ast::Unit unit;

        std::vector<ParsedToken>::iterator token;
        const std::vector<ParsedToken>::iterator end;

        LabelDispatcher labels_dispatcher;

      private:
        ParseResult<Signature> parse_signature();
        ParseResult<ast::Label> parse_label();
        ParseResult<Identifier> parse_ident();

        ParseResult<ast::ASTBlock> parse_block();
        ParseResult<ast::If> parse_if();
        ParseResult<ast::While> parse_while();
        ParseResult<ast::Use> parse_use();

        ParseResult<ast::Macro> parse_macro();

      private:
        template <typename T>
        bool is_standing_on(const T& token) {
            return this->token != end && variant_is(this->token->data, token);
        }

        constexpr auto parse_struct() -> ParseResult<std::tuple<>> {
            return std::tuple<>();
        }

        template <typename First, typename ...Args>
        constexpr auto parse_struct(First&& arg, Args&&... args) -> ParseResult<remove_expected_t<ParseError, extract_callables_t<First, Args...>>> {
            if constexpr (!is_invocable_or_member_v<First>) {
                if (token == end) {
                    std::stringstream ss;
                    ss << "Unexpected end. Expected \"" << arg << "\".";
                    return std::unexpected(ParseError(token->pos, std::move(ss.str())));
                }

                if (!variant_is(token->data, arg)) {
                    std::stringstream ss;
                    ss << "Unexpected token. Expected \"" << arg << "\".";
                    return std::unexpected(ParseError(token->pos, std::move(ss.str())));
                }

                ++token;

                return parse_struct(std::forward<Args>(args)...);
            } else {
                if (token == end) {
                    return std::unexpected(ParseError(token->pos, "Unexpected end."));
                }

                auto result = [&]() {
                    if constexpr (std::is_member_pointer_v<First>) {
                        return (this->*arg)();
                    } else {
                        return arg();
                    }
                }();

                if constexpr (is_expected_v<decltype(result)>) {
                    if (!result.has_value()) {
                        return std::unexpected(result.error());
                    }
                }

                auto others = parse_struct(std::forward<Args>(args)...);

                if (!others.has_value()) {
                    return std::unexpected(others.error());
                }

                return std::tuple_cat(
                    std::make_tuple(std::move(result.value())),
                    std::move(others.value())
                );
            }
        }

      public:
        Parser(std::vector<ParsedToken>& tokens) :
            token(tokens.begin()),
            end(tokens.end()) {}

        
        ParseResult<ast::Unit> parse();
    };

}  // namespace bfasm::parse

std::ostream& operator<<(std::ostream& os, const bfasm::parse::ParseError& error);
