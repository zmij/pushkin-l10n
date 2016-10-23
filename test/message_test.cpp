/*
 * message_test.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <pushkin/l10n/message.hpp>

namespace psst {
namespace l10n {
namespace test {

TEST(Message, AddArgs)
{
    message msg{"{3}{2}={1}"};
    EXPECT_NO_THROW(msg << 42 << "Foo" << "Bar") << "Feed format arguments";
    EXPECT_FALSE(msg.args_empty()) << "Has formatting arguments";
    EXPECT_TRUE(msg.has_format_args()) << "Has formatting arguments";
    EXPECT_EQ(3, msg.args_size()) << "Correct count of arguments";
}

TEST(Message, Copy)
{
    message msg{"{3}{2}={1}"};
    EXPECT_NO_THROW(msg << 42 << "Foo" << "Bar") << "Feed format arguments";
    message copy = msg;
    EXPECT_FALSE(copy.args_empty()) << "Has formatting arguments";
    EXPECT_TRUE(copy.has_format_args()) << "Has formatting arguments";
    EXPECT_EQ(3, copy.args_size()) << "Correct count of arguments";
    EXPECT_FALSE(msg.args_empty()) << "Has formatting arguments";
    EXPECT_TRUE(msg.has_format_args()) << "Has formatting arguments";
    EXPECT_EQ(3, msg.args_size()) << "Correct count of arguments";
}

TEST(Message, Move)
{
    message msg{"{3}{2}={1}"};
    EXPECT_NO_THROW(msg << 42 << "Foo" << "Bar") << "Feed format arguments";
    message copy = ::std::move(msg);
    EXPECT_FALSE(copy.args_empty()) << "Has formatting arguments";
    EXPECT_TRUE(copy.has_format_args()) << "Has formatting arguments";
    EXPECT_EQ(3, copy.args_size()) << "Correct count of arguments";
    EXPECT_TRUE(msg.args_empty()) << "Has formatting arguments";
    EXPECT_FALSE(msg.has_format_args()) << "Has formatting arguments";
    EXPECT_EQ(0, msg.args_size()) << "Correct count of arguments";
}

TEST(Message, Format)
{
    ::std::string expected{"BarFoo=42"};
    message msg{"{3}{2}={1}"};
    EXPECT_NO_THROW(msg << 42 << "Foo" << "Bar") << "Feed format arguments";

    auto fmt = msg.format();
    EXPECT_EQ(expected, fmt.str());
}

TEST(Message, NestedMessage)
{
    ::std::string expected{"Message: BarFoo=42"};
    message nested{"{3}{2}={1}"};
    EXPECT_NO_THROW(nested << 42 << "Foo" << "Bar") << "Feed format arguments";

    message outer{"{1}: {2}"};
    EXPECT_NO_THROW(outer << "Message" << nested) << "Add a nested message format argumet";
    EXPECT_FALSE(outer.args_empty()) << "Non empty outer format arguments";
    EXPECT_EQ(2, outer.args_size());

    auto fmt = outer.format();
    EXPECT_EQ(expected, fmt.str());
}

}  /* namespace test */
}  /* namespace l10n */
}  /* namespace psst */
