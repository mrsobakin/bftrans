#pragma once

#include <expected>
#include <unordered_map>
#include "../labels/bflabels.h"
#include "ast.h"
#include "../utils.h"

namespace bfasm::compiler {
    struct CompileError {
        std::variant<const char*, std::string> msg;

        constexpr CompileError(const char* msg) :
            msg(msg) {}

        CompileError(std::string&& msg) :
            msg(std::move(msg)) {}
    };

    class StackLabels {
        using LabelsMap = std::unordered_map<ast::Label, bflabels::Label>;
        std::vector<std::pair<LabelsMap, size_t>> labels = {{{}, 0}};

      public:
        void push() {
            labels.push_back(labels.back());
        }

        void pop() {
            labels.pop_back();
        };

        void merge_injection(const std::vector<ast::Label>& new_labels, const std::vector<ast::Label>& old_labels) {
            auto& curr = labels.back().first;
            for (size_t i = 0; i < new_labels.size(); ++i) {
                curr[new_labels[i]] = get(old_labels[i]);
            }
        };

        bflabels::Label get(ast::Label label) {
            auto& curr = labels.back().first;
            auto& last_id = labels.back().second;

            if (curr.contains(label)) {
                return curr[label];
            }

            ++last_id;

            curr[label] = bflabels::Label { last_id };

            return bflabels::Label { last_id };
        };

        bflabels::Label temp() {
            auto& last_id = labels.back().second;

            ++last_id;

            return bflabels::Label { last_id };
        };
    };

    template <typename T>
    using CompileResult = std::expected<T, CompileError>;

    class Compiler {
      public:
        std::vector<bflabels::Token> result;
        const ast::Unit& unit;
        StackLabels labeler;

        Compiler(const ast::Unit& unit) : unit(unit) {}

        std::optional<CompileError> compile() {
            const auto& main = unit.at("main");

            if (!main.arguments.empty() || !main.returns.empty()) {
                return CompileError("main macro shouldn't take or return any labels.");
            }

            compile_block(main.block);

            return std::nullopt;
        }

        void push_plains(std::string_view plains) {
            for (char ch : plains) {
                result.emplace_back(ch);
            }
        }

        void compile_use(const ast::Use& use) {
            auto& macro = unit.at(use.macro_name);

            labeler.push();
            labeler.merge_injection(macro.arguments, use.arguments);
            labeler.merge_injection(macro.returns, use.return_into);
            
            compile_block(macro.block);

            labeler.pop();
        };

        void compile_block(const ast::ASTBlock& block) {
            for (const auto& node : block) {
                std::visit(overloaded {
                    [&](ast::Plain p) {
                        result.emplace_back(p);
                    },
                    [&](ast::Label label) {
                        result.emplace_back(labeler.get(label));
                    },
                    [&](const ast::Use& use) {
                        compile_use(use);
                    },
                    [&](const ast::If& if_) {
                        bflabels::Label temp0 = labeler.temp();
                        bflabels::Label temp1 = labeler.temp();
                        bflabels::Label x = labeler.get(if_.condition);

                        // temp0[-]+
                        result.emplace_back(temp0);
                        push_plains("[-]+");

                        // temp1[-]
                        result.emplace_back(temp1);
                        push_plains("[-]");

                        // x[
                        result.emplace_back(x);
                        push_plains("[");

                        // code1
                        compile_block(if_.then_block);

                        //     temp0-
                        result.emplace_back(temp0);
                        push_plains("-");

                        //     x[temp1+x-]
                        result.emplace_back(x);
                        push_plains("[");
                        result.emplace_back(temp1);
                        push_plains("+");
                        result.emplace_back(x);
                        push_plains("-]");

                        // ]
                        push_plains("]");

                        // temp1[x+temp1-]
                        result.emplace_back(temp1);
                        push_plains("[");
                        result.emplace_back(x);
                        push_plains("+");
                        result.emplace_back(temp1);
                        push_plains("-]");

                        // temp0[
                        result.emplace_back(temp0);
                        push_plains("[");

                        // code2
                        compile_block(if_.else_block);

                        // temp0-]
                        result.emplace_back(temp0);
                        push_plains("-]");
                    },
                    [&](const ast::While& while_) {
                        bflabels::Label x = labeler.get(while_.condition);

                        // x[
                        result.emplace_back(x);
                        push_plains("[");

                        //    code
                        compile_block(while_.do_block);

                        result.emplace_back(x);
                        push_plains("]");
                        // x]
                    },
                }, node);
            }
        }
    };
}  // na    mespace bfasm::compiler
