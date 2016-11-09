/*
 * placeholders.hpp
 *
 *  Created on: Nov 9, 2016
 *      Author: zmij
 */

#ifndef PUSHKIN_L10N_GRAMMAR_PLACEHOLDERS_HPP_
#define PUSHKIN_L10N_GRAMMAR_PLACEHOLDERS_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include <pushkin/l10n/message_util.hpp>

namespace psst {
namespace l10n {
namespace grammar {
namespace parse {

template < typename InputIterator >
struct placeholder_grammar :
        ::boost::spirit::qi::grammar<InputIterator, placeholder()> {
    using main_rule = ::boost::spirit::qi::rule<InputIterator, placeholder()>;
    template < typename ... T >
    using rule = ::boost::spirit::qi::rule<InputIterator, T...>;

    placeholder_grammar() : placeholder_grammar::base_type(main)
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::lit;
        using qi::_val;
        using qi::_1;
        using qi::char_;

        number  = qi::uint_parser<::std::size_t>();
        name    %= char_("a-zA-Z_") >> *char_("a-zA-Z0-9_.");
        options %= ',' >> +(char_ - '}');
        main    = '{' >> (number[ phx::bind(&placeholder::id, _val) = _1 ]
                      | (-lit("n:")[ phx::bind(&placeholder::is_pluralizer, _val) = true ]
                         >> name[ phx::bind(&placeholder::id, _val) = _1 ] ))
                >> -options[ phx::bind(&placeholder::options, _val) = _1 ] >> '}';
    }

    main_rule                       main;
    rule<::std::size_t()>           number;
    rule<::std::string()>           name, options;
};

template < typename InputIterator >
struct placeholders_grammar :
        ::boost::spirit::qi::grammar<InputIterator, placeholders()> {

    using main_rule = ::boost::spirit::qi::rule<InputIterator, placeholders()>;
    using placeholder_rule = placeholder_grammar<InputIterator>;

    template < typename ... T >
    using rule = ::boost::spirit::qi::rule<InputIterator, T...>;

    placeholders_grammar() : placeholders_grammar::base_type{main}
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::lit;
        using qi::_val;
        using qi::_1;
        using qi::char_;

        main = *(*(char_ - '{') >> (lit("{{") | ph[ phx::push_back(_val, _1) ]));
    }

    main_rule                       main;
    placeholder_rule                ph;
};

}  /* namespace parse */
}  /* namespace grammar */
}  /* namespace l10n */
}  /* namespace psst */



#endif /* PUSHKIN_L10N_GRAMMAR_PLACEHOLDERS_HPP_ */
