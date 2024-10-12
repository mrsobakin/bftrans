#include <gtest/gtest.h>

#include <lib/labels/bflabels.h>


using Tokens = std::vector<bflabels::Token>;


TEST(BFLabelsParser, Simple) {
    using namespace bflabels;

    std::string code = "+-[]<>.,";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        '+',
        '-',
        '[',
        ']',
        '<',
        '>',
        '.',
        ',',
    }));
}

TEST(BFLabelsParser, Labels) {
    using namespace bflabels;

    std::string code = "+temp0-temp1,temp2";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        '+',
        Label{1, 0},
        '-',
        Label{2, 0},
        ',',
        Label{3, 0},
    }));
}

TEST(BFLabelsParser, LongLabels) {
    using namespace bflabels;

    std::string code = "+temp0(12)-temp1(23),temp2(34)";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        '+',
        Label{1, 12},
        '-',
        Label{2, 23},
        ',',
        Label{3, 34},
    }));
}

TEST(BFLabelsParser, MixedLabels) {
    using namespace bflabels;

    std::string code = "temp0+-temp1(4)";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        Label{1, 0},
        '+',
        '-',
        Label{2, 4},
    }));
}

TEST(BFLabelsParser, Whitespaces) {
    using namespace bflabels;

    std::string code = "  temp0 \t  - temp1    ( \n  32 \t )";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        Label{1, 0},
        '-',
        Label{2, 32},
    }));
}

TEST(BFLabelsParser, Scopes) {
    using namespace bflabels;

    std::string code = "{temp0{temp1{temp2}}++}";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        Scope::Enter,
        Label{1, 0},
        Scope::Enter,
        Label{2, 0},
        Scope::Enter,
        Label{3, 0},
        Scope::Exit,
        Scope::Exit,
        '+',
        '+',
        Scope::Exit,
    }));
}

TEST(BFLabelsParser, BadCharacters) {
    using namespace bflabels;

    std::string code = "привет";

    auto result = Parser(code).parse();

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error(), ParseError::BadCharacter);
}

TEST(BFLabelsParser, Complex) {
    using namespace bflabels;

    std::string code =
        "{                      \n"
        "    temp0++++          \n"
        "                       \n"
        "    temp1(01)++++      \n"
        "    temp1(12)++++      \n"
        "    <<--<<--<<--       \n"
        "                       \n"
        "    temp1(0)           \n"
        "                       \n"
        "    {                  \n"
        "        temp2          \n"
        "    }                  \n"
        "}                      \n";

    auto result = Parser(code).parse();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, Tokens({
        Scope::Enter,
        Label{1, 0},
        '+', '+', '+', '+',
        Label{2, 1},
        '+', '+', '+', '+',
        Label{2, 12},
        '+', '+', '+', '+',
        '<', '<', '-', '-',
        '<', '<', '-', '-',
        '<', '<', '-', '-',
        Label{2, 0},
        Scope::Enter,
        Label{3, 0},
        Scope::Exit,
        Scope::Exit,
    }));
}
