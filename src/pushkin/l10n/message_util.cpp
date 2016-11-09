/*
 * message_util.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: zmij
 */

#include <pushkin/l10n/message_util.hpp>
#include <pushkin/l10n/grammar/placeholders.hpp>

#include <iostream>
#include <sstream>
#include <map>

namespace psst {
namespace l10n {

::std::ostream&
operator << (::std::ostream& os, placeholder const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        os << "{" << (val.type() == placeholder::name ? (val.is_pluralizer ? "n:" : "") : "")
                << val.id << (val.options.empty() ? "" : ",") << val.options << "}";
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, placeholders const& val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        for (auto const& ph: val) {
            os << ph;
        }
    }
    return os;
}

placeholders
extract_placeholders(::std::string const& input)
{
    namespace qi = ::boost::spirit::qi;
    using string_iterator   = ::std::string::const_iterator;
    using grammar_type      = grammar::parse::placeholders_grammar<string_iterator>;
    static grammar_type parser;

    string_iterator first = input.begin();
    string_iterator last = input.end();

    placeholders ph;
    qi::parse(first, last, parser, ph);
    return ph;
}

struct ph_usage {
    ::std::size_t   number;
    bool            is_pluralizer;
    ::std::size_t   uses;
};

::std::pair<::std::string, placeholders>
extract_named_placeholders(::std::string const& input)
{
    auto phs = extract_placeholders(input);
    if (phs.empty())
        return {input, placeholders{}};
    placeholders named_phs;
    ::std::copy_if(phs.begin(), phs.end(), ::std::back_inserter(named_phs),
            [](placeholder const& ph)
            {
                return ph.type() == placeholder::name;
            });
    if (named_phs.empty())
        return {input, placeholders{}};

    // 1. Assign numbers to names
    ::std::map< ::std::string, ph_usage > name_numbers;
    for (auto const& ph : named_phs) {
        auto nm = ::boost::get< ::std::string >(ph.id);
        auto f = name_numbers.find(nm);
        if (f == name_numbers.end()) {
            name_numbers.emplace(nm, ph_usage{name_numbers.size() + 1, ph.is_pluralizer, 1});
        } else{
            if (ph.is_pluralizer)
                f->second.is_pluralizer = ph.is_pluralizer;
            ++f->second.uses;
        }
    }

    // 2. Renumber placeholders
    auto delta = name_numbers.size();
    for (auto& ph : phs) {
        if (ph.type() == placeholder::number) {
            ::boost::get<::std::size_t>(ph.id) += delta;
        } else {
            auto const& nm = ::boost::get< ::std::string >(ph.id);
            ph.id = name_numbers[nm].number;
        }
    }
    // 3. Construct message id with replaced placeholders
    ::std::ostringstream os;
    auto ph = phs.begin();
    for (auto c = input.begin(); c != input.end(); ++c) {
        if (*c == '{') {
            ::std::ostringstream backtrack;
            for (; c != input.end() && *c != '}'; ++c) {
                backtrack.put(*c);
            }
            if (c == input.end()) {
                os << backtrack.str();
            } else {
                os << *ph++;
            }
        } else {
            os.put(*c);
        }
    }
    // 4. Remove duplicate names
    if (named_phs.size() != name_numbers.size()) {
        placeholders tmp;
        tmp.reserve(name_numbers.size());
        ::std::set<::std::string> seen;
        for (auto& ph: named_phs) {
            auto const& nm = ::boost::get< ::std::string >(ph.id);
            if (!seen.count(nm)) {
                auto const& use = name_numbers[nm];
                ph.is_pluralizer = use.is_pluralizer;
                ph.uses = use.uses;
                tmp.push_back(ph);
                seen.emplace(nm);
            }
        }
        named_phs.swap(tmp);
    }

    return { os.str(), named_phs };
}

}  /* namespace l10n */
}  /* namespace psst */

