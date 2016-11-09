/*
 * placeholders_test.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <pushkin/l10n/message.hpp>
#include <pushkin/l10n/grammar/placeholders.hpp>

namespace psst {
namespace l10n {
namespace test {

class Placeholder : public ::testing::TestWithParam<::std::string>
{
protected:
    using string_iterator   = ::std::string::const_iterator;
    using grammar_type      = grammar::parse::placeholder_grammar<string_iterator>;

    grammar_type            parser;
};

TEST_P(Placeholder, ValidInput)
{
    namespace qi = ::boost::spirit::qi;
    ::std::string input = GetParam();
    string_iterator first = input.begin();
    string_iterator last = input.end();
    EXPECT_TRUE(qi::parse(first, last, parser));
    EXPECT_EQ(last, first);
}

INSTANTIATE_TEST_CASE_P(ParserTest, Placeholder,
        ::testing::Values(
            "{1}", "{2}", "{2,ftime='%I o''clock'}", "{2,time=full}",
            "{1,locale=he_IL@calendar=hebrew,date=l}",
            "{name}","{n:count,cur}"
        ));

class Placeholders : public ::testing::TestWithParam<::std::pair<::std::string, ::std::size_t>>
{
protected:
    using string_iterator   = ::std::string::const_iterator;
    using grammar_type      = grammar::parse::placeholders_grammar<string_iterator>;

    grammar_type            parser;
};

TEST_P(Placeholders, ValidInput)
{
    namespace qi = ::boost::spirit::qi;
    auto input = GetParam();
    string_iterator first = input.first.begin();
    string_iterator last = input.first.end();
    placeholders res;
    EXPECT_TRUE(qi::parse(first, last, parser, res));
    EXPECT_EQ(input.second, res.size());
}

INSTANTIATE_TEST_CASE_P(ParserTest, Placeholders,
        ::testing::Values(
           Placeholders::ParamType{"", 0},
           Placeholders::ParamType{"Some phrase", 0},
           Placeholders::ParamType{"Today {1,date} I would meet {2} at home", 2},
           Placeholders::ParamType{"Ms. {1} had arrived at {2,ftime='%I o''clock'} at home. The exact time is {2,time=full}", 3},
           Placeholders::ParamType{"Unexpected `{{' in line {1} in file {2}", 2},
           Placeholders::ParamType{"{FogGrenades.EffectTimeSec} second", 1},
           Placeholders::ParamType{"{n:FogGrenades.EffectTimeSec} second", 1}
        ));

TEST(ParserTest, ExtractNamedPlaceholders)
{
    {
        auto id_phs = extract_named_placeholders("{A} second {1} {B}");
        EXPECT_EQ("{1} second {3} {2}", id_phs.first);
        EXPECT_EQ(2, id_phs.second.size());
    }
    {
        auto id_phs = extract_named_placeholders("{A} second {1} {B} some {A}");
        EXPECT_EQ("{1} second {3} {2} some {1}", id_phs.first);
        EXPECT_EQ(2, id_phs.second.size()) << "Duplicates eliminated " << id_phs.second;
        EXPECT_EQ(2, id_phs.second[0].uses);
        EXPECT_EQ(1, id_phs.second[1].uses);
    }
    {
        auto id_phs = extract_named_placeholders("{A} second {1} {B} some {n:A}");
        EXPECT_EQ("{1} second {3} {2} some {1}", id_phs.first);
        EXPECT_EQ(2, id_phs.second.size()) << "Duplicates eliminated " << id_phs.second;
        EXPECT_TRUE(id_phs.second[0].is_pluralizer);
        EXPECT_FALSE(id_phs.second[1].is_pluralizer);
        EXPECT_EQ(2, id_phs.second[0].uses);
        EXPECT_EQ(1, id_phs.second[1].uses);
    }
    {
        auto id_phs = extract_named_placeholders("{A,hex} second {1,ftime='%I o''clock'} {B,time=full} some {n:A,oct}");
        EXPECT_EQ("{1,hex} second {3,ftime='%I o''clock'} {2,time=full} some {1,oct}", id_phs.first);
        EXPECT_EQ(2, id_phs.second.size()) << "Duplicates eliminated " << id_phs.second;
        EXPECT_TRUE(id_phs.second[0].is_pluralizer);
        EXPECT_FALSE(id_phs.second[1].is_pluralizer);
        EXPECT_EQ(2, id_phs.second[0].uses);
        EXPECT_EQ(1, id_phs.second[1].uses);
    }
}

TEST(Placeholders, FeedNamedValues)
{
    auto fn = [](message& msg, ::std::string const& name)
    {
        if (name == "A") {
            msg << 42;
        } else if (name == "B") {
            msg << "foo";
        }
    };
    auto get_n = [](::std::string const& name)
    {
        return 42;
    };
    {
        auto msg = message::create_message("{A} second {1} {B}", fn);
        EXPECT_EQ("{1} second {3} {2}", msg.id());
        msg << "blabla";
        EXPECT_EQ("42 second blabla foo", msg.str());
    }
    {
        auto msg = message::create_message("{n:A} bar {B} {A}",
                "{n:A} bars {B} {A}", fn, get_n);
        EXPECT_EQ("{1} bar {2} {1}", msg.id());
        EXPECT_EQ("42 bars foo 42", msg.str());
    }
}

}  /* namespace test */
}  /* namespace l10n */
}  /* namespace psst */
