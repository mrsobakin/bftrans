#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lib/labels/bflabels.h>


using Tokens = std::vector<bflabels::Token>;


TEST(BFLabelsCompiler, Simple) {
    using namespace bflabels;
    using namespace ::testing;

    Tokens code = {
        Label(1, 0),
        '+',
        Label(2, 0),
        '+',
    };

    auto result = BFLCode(code).compile();

    ASSERT_THAT(result, AnyOf("+>+", ">+<+"));
}

TEST(BFLabelsCompiler, LabelElision) {
    using namespace bflabels;
    using namespace ::testing;

    Tokens code = {
        '+',
        Label(1, 0),
        Label(2, 0),
        Label(2, 0),
        Label(3, 0),
        Label(1, 0),
        Label(2, 0),
        Label(1, 0),
        Label(3, 0),
        Label(1, 0),
        '+',
    };

    auto result = BFLCode(code).compile();

    ASSERT_THAT(result, AnyOf("++", "+>+", "+>>+"));
}

TEST(BFLabelsCompiler, InterscopeLabelReuse) {
    using namespace bflabels;
    using namespace ::testing;

    Tokens code = {
        '+',
        Scope::Enter,
        Label(1, 0),
        '+',
        Scope::Exit,
        Scope::Enter,
        Label(2, 0),
        '+',
        Scope::Exit,
        Scope::Enter,
        Label(3, 0),
        '+',
        Scope::Exit,
        '+',
    };

    auto result = BFLCode(code).compile();

    ASSERT_THAT(result, AnyOf("+++++"));
}

TEST(BFLabelsCompiler, IntrascopeLabelSharing) {
    using namespace bflabels;
    using namespace ::testing;

    Tokens code = {
        '+',
        Scope::Enter,
        Label(1, 0),
        '+',
        Label(2, 0),
        '+',
        Scope::Enter,
        Label(1, 0),
        '+',
        Scope::Exit,
        Scope::Exit,
        '+',
    };

    auto result = BFLCode(code).compile();

    ASSERT_THAT(result, AnyOf("+>+<+>++", "++>+<+"));
}
