/*
 * arg_value_test.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <pushkin/l10n/message.hpp>

namespace psst {
namespace l10n {
namespace test {

TEST(Args, Add)
{
    detail::message_args args;
    EXPECT_NO_THROW(args << 10 << "Foo" << "Bar") << "Add argument";
    EXPECT_FALSE(args.empty()) << "Arguments are not empty";
    EXPECT_EQ(3, args.size()) << "Correct arguments size";
}

TEST(Args, Copy)
{
    detail::message_args args;
    EXPECT_NO_THROW(args << 10 << "Foo" << "Bar") << "Add argument";
    detail::message_args copy = args;
    EXPECT_FALSE(args.empty()) << "Arguments are not empty";
    EXPECT_EQ(3, args.size()) << "Correct arguments size";
    EXPECT_FALSE(copy.empty()) << "Arguments are not empty";
    EXPECT_EQ(3, copy.size()) << "Correct arguments size";
}

TEST(Args, Move)
{
    detail::message_args args;
    EXPECT_NO_THROW(args << 10 << "Foo" << "Bar") << "Add argument";
    detail::message_args copy = ::std::move(args);
    EXPECT_TRUE(args.empty()) << "Source arguments are empty";
    EXPECT_EQ(0, args.size()) << "Correct source arguments size";
    EXPECT_FALSE(copy.empty()) << "Arguments are not empty";
    EXPECT_EQ(3, copy.size()) << "Correct arguments size";
}

TEST(Args, Format)
{
    message::localized_message fmt_str{"{3}{2}={1}"};
    ::std::string expected{"BarFoo=42"};
    {
        // One by one
        format fmt{fmt_str};
        detail::message_args args;
        EXPECT_NO_THROW(args << 42 << "Foo" << "Bar") << "Add argument";
        for (auto const& arg : args) {
            fmt % arg;
        }
        EXPECT_EQ(expected, fmt.str()) << "Correct formatted message";
    }

    {
        // Call container func
        format fmt{fmt_str};
        detail::message_args args;
        EXPECT_NO_THROW(args << 42 << "Foo" << "Bar") << "Add argument";
        fmt % args;
        EXPECT_EQ(expected, fmt.str()) << "Correct formatted message";
    }
}

}  /* namespace test */
}  /* namespace l10n */
}  /* namespace psst */
