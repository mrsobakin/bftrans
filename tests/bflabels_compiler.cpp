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
