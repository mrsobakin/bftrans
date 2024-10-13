#include "parser.h"

#include <variant>
#include <iostream>
#include <tuple>
#include "ast.h"
#include "utils.h"
#include "../utils.h"

namespace bfasm::parse {
    void LabelDispatcher::clear_locals() {
        ids.clear();
    }

    ast::Label LabelDispatcher::get(std::string_view ident) {
        if (ids.contains(ident)) {
            return ast::Label { ids[ident] };
        }

        ++last_id;

        ids[ident] = last_id;

        return ast::Label { last_id };
    }

    ParseResult<ast::Macro> Parser::parse_macro() {
        this->labels_dispatcher.clear_locals();

        auto res = this->parse_struct(
            Keyword::Macro,
            &Parser::parse_signature,
            Control::Semicolon,
            &Parser::parse_block
        );

        if (!res.has_value()) {
            return std::unexpected(res.error());
        }

        auto&& [signature, block] = *res;

        return ast::Macro {
            .name = signature.name,
            .block = block,
            .arguments = signature.arguments,
            .returns = signature.returns,
        };
    }

    ParseResult<ast::ASTBlock> Parser::parse_block() {
        ast::ASTBlock block;

        bool next_block = false;
        for (; token != end; ++token) {
            auto parse_result = std::visit(overloaded{
                [&](Plain plain) -> std::optional<ParseError> {
                    block.emplace_back(plain);
                    return std::nullopt;
                },
                [&](const Identifier& ident) -> std::optional<ParseError> {
                    block.emplace_back(labels_dispatcher.get(ident));
                    return std::nullopt;
                },
                [&](Keyword keyword) -> std::optional<ParseError> {
                    switch (keyword) {
                        case Keyword::Macro:
                            // next macro block, break
                            next_block = true;
                            return std::nullopt;

                        case Keyword::Else:
                            return ParseError(token->pos, "Unexpexted ELSE keyword.");

                        case Keyword::If: {
                            auto if_or_err = parse_if();
                            --token;
                            if (if_or_err.has_value()) {
                                block.emplace_back(if_or_err.value());
                                return std::nullopt;
                            }

                            return if_or_err.error();
                        }

                        case Keyword::While: {
                            auto while_or_err = parse_while();
                            --token;
                            if (while_or_err.has_value()) {
                                block.emplace_back(while_or_err.value());
                                return std::nullopt;
                            }

                            return while_or_err.error();
                        }

                        case Keyword::Use: {
                            auto use_or_err = parse_use();
                            --token;
                            if (use_or_err.has_value()) {
                                block.emplace_back(use_or_err.value());
                                return std::nullopt;
                            }

                            return use_or_err.error();
                        }
                    }
                    return std::nullopt;
                },
                [&](Control control) -> std::optional<ParseError> {
                    if (control == Control::RCurly) {
                        next_block = true;
                        return std::nullopt;
                    }
                    std::stringstream ss;
                    ss << "Unexpected control symbol: \"" << control << "\".";
                    return ParseError(token->pos, ss.str());
                },
            }, token->data);

            if (parse_result.has_value()) {
                return std::unexpected(*parse_result);
            }

            if (next_block) break;
        }

        return block;
    }

    ParseResult<ast::If> Parser::parse_if() {
        auto if_part = this->parse_struct(
            Keyword::If,
            &Parser::parse_label,
            Control::LCurly,
            &Parser::parse_block,
            Control::RCurly
        );

        if (!if_part.has_value()) {
            return std::unexpected(if_part.error());
        }

        auto&& [condition, then_block] = *if_part;
        ast::ASTBlock else_block;

        if (this->is_standing_on(Keyword::Else)) {
            auto else_part = this->parse_struct(
                Keyword::Else,
                Control::LCurly,
                &Parser::parse_block,
                Control::RCurly
            );

            if (!else_part.has_value()) {
                return std::unexpected(else_part.error());
            }

            else_block = std::move(std::get<0>(*else_part));
        }

        return ast::If {
            .condition = condition,
            .then_block = std::move(then_block),
            .else_block = std::move(else_block),
        };
    }

    ParseResult<ast::While> Parser::parse_while() {
        auto while_ = this->parse_struct(
            Keyword::While,
            &Parser::parse_label,
            Control::LCurly,
            &Parser::parse_block,
            Control::RCurly
        );

        if (!while_.has_value()) {
            return std::unexpected(while_.error());
        }

        auto& [condition, block] = *while_;

        return ast::While {
            .condition = condition,
            .do_block = block,
        };
    }

    ParseResult<ast::Use> Parser::parse_use() {
        auto use = this->parse_struct(
            Keyword::Use,
            &Parser::parse_signature
        );

        if (!use.has_value()) {
            return std::unexpected(use.error());
        }

        auto& [signature] = *use;

        if (!unit.contains(signature.name)) {
            return std::unexpected(ParseError(Position::END, "No such macro."));  // TODO: pos
        }

        return ast::Use {
            .macro_name = signature.name,
            .arguments = signature.arguments,
            .return_into = signature.returns,
        };
    }

    ParseResult<Identifier> Parser::parse_ident() {
        if (this->token == this->end) {
            return std::unexpected(ParseError(Position::END, "Unexpected end. Expected identifier."));
        }

        Identifier* ident = std::get_if<Identifier>(&this->token->data);
        if (!ident) {
            return std::unexpected(ParseError(token->pos, "Invalid token. Expected identifier."));
        }

        ++this->token;

        return *ident;
    }

    ParseResult<ast::Label> Parser::parse_label() {
        auto ident = this->parse_ident();

        if (!ident.has_value()) {
            return std::unexpected(ident.error());
        }

        return this->labels_dispatcher.get(*ident);
    }

    ParseResult<Signature> Parser::parse_signature() {
        std::vector<ast::Label> arguments;
        std::vector<ast::Label> returns;

        Identifier* name = std::get_if<Identifier>(&token->data);

        if (!name) {
            return std::unexpected(ParseError(token->pos, "Invalid token. Expected identifier."));
        }

        if (++token == end) {
            return std::unexpected(ParseError(Position::END, "Unexpected end. Expected opening parenthesis."));
        }

        {
            Control* control = std::get_if<Control>(&token->data);
            if (!control || (*control != Control::LParen)) {
                return std::unexpected(ParseError(token->pos, "Invalid token. Expected opening parenthesis."));
            }
        }

        bool parse_returns = false;
        while (1) {
            if (++token == end) {
                return std::unexpected(ParseError(Position::END, "Unexpected end."));
            }

            if (Identifier* argument = std::get_if<Identifier>(&token->data)) {
                arguments.emplace_back(labels_dispatcher.get(*argument));
                continue;
            }

            Control* control = std::get_if<Control>(&token->data);
            if (!control || (*control != Control::Arrow && *control != Control::RParen)) {
                return std::unexpected(ParseError(token->pos, "Invalid token. Expected arrow or closing parenthesis."));
            }

            parse_returns = *control == Control::Arrow;

            break;
        }

        if (parse_returns) {
            while (1) {
                if (++token == end) {
                    return std::unexpected(ParseError(Position::END, "Unexpected end."));
                }

                if (Identifier* return_ = std::get_if<Identifier>(&token->data)) {
                    returns.emplace_back(labels_dispatcher.get(*return_));
                    continue;
                }

                Control* control = std::get_if<Control>(&token->data);
                if (!control || (*control != Control::RParen)) {
                    return std::unexpected(ParseError(token->pos, "Invalid token. Expected closing parenthesis."));
                }

                break;
            }
        }

        ++token;

        return Signature{ *name, arguments, returns };
    }

    ParseResult<ast::Unit> Parser::parse() {
        while (token != end) {
            auto macro = parse_macro();

            if (!macro.has_value()) {
                return std::unexpected(macro.error());
            }

            unit[macro->name] = std::move(*macro);
        }

        return unit;
    }
}  // namespace bfasm::parse

std::ostream& operator<<(std::ostream& os, const bfasm::parse::ParseError& error) {
    if (error.pos != bfasm::parse::Position::END) {
        os << "Error at line " << error.pos.line << " column " << error.pos.column << ": ";
    } else {
        os << "Error : ";
    }

    std::visit([&] (const auto& error) {
        os << error;
    }, error.msg);

    return os;
}
