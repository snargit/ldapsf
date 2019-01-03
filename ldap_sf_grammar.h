#pragma once
#ifndef BOOST_SPIRIT_UNICODE
#define BOOST_SPIRIT_UNICODE
#endif

#include "ldap_sf_ast.h"

#include <boost/optional.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/qi.hpp>

#include <memory>
#include <string>
#include <vector>

namespace  ldap { namespace sf
{

using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::spirit::unicode;

struct Compiler
{
    template <typename A,
              typename B = ::boost::spirit::unused_type,
              typename C = ::boost::spirit::unused_type,
              typename D = ::boost::spirit::unused_type>
        struct result { using type = void; };

    void operator()(NodeList & nodes, Node &  node) const;

    void operator()(Node & node, NodeList & nodes, FilterComp comp) const;

    void operator()(Node & node, Node & nodeNest) const;

    void operator()(ItemPtr & item, AttrOptionsList & attrs,
                    SimpleItemOp  op, Value & value) const;

    void operator()(AttrOptionsList & attrs, Attr & value,
                    boost::optional<AttrOptionsList> & attrs_options) const;

    void operator()(ItemPtr &  item, AttrOptionsList & attrs,
                    ValueListMore & values) const;

    void operator()(ValueListMore & values,
                    boost::optional<Value> & value_first,
                    ValueList & values_middle,
                    boost::optional<Value> & value_last) const;
};

template<typename Iterator>
struct Grammar : grammar<Iterator,
                         Node(),
                         boost::spirit::unicode::space_type>
{
    using NodeRule = ::boost::spirit::qi::rule<Iterator,
                                               Node(),
                                               boost::spirit::unicode::space_type>;
    using NodeListRule = ::boost::spirit::qi::rule<Iterator,
                                                   NodeList(),
                                                   boost::spirit::unicode::space_type>;
    using ItemPtrRule = ::boost::spirit::qi::rule<Iterator,
                                                  ItemPtr(),
                                                  boost::spirit::unicode::space_type>;
    Grammar();

    NodeRule filter;
    NodeListRule filterlist;
    NodeRule filtercomp;
    NodeRule and_expr;
    NodeRule or_expr;
    NodeRule not_expr;
    ItemPtrRule item;
    ItemPtrRule simple;
    ItemPtrRule substring;
    ItemPtrRule extensible;

    ::boost::spirit::qi::rule<Iterator, SimpleItemOp(), ::boost::spirit::unicode::space_type>
        filtertype;

    ::boost::spirit::qi::rule<Iterator, AttrOptionsList(), ::boost::spirit::unicode::space_type>
        attr;

    ::boost::spirit::qi::rule<Iterator, Attr(), ::boost::spirit::unicode::space_type>
        attr_item;

    ::boost::spirit::qi::rule<Iterator, Value(), ::boost::spirit::unicode::space_type>
        value;

    ::boost::spirit::qi::rule<Iterator, ValueListMore(), ::boost::spirit::unicode::space_type>
        values;

    ::boost::spirit::qi::rule<Iterator, MatchingRule(), ::boost::spirit::unicode::space_type>
        matchingrule;

    ::boost::spirit::qi::uint_parser<unsigned char, 16, 2, 2> hex_2;

    ::boost::spirit::qi::uint_parser<unsigned int, 16, 8, 8>  hex_8;

    ::boost::phoenix::function<Compiler> op;
};

template<typename Iterator>
Grammar<Iterator>::Grammar() : Grammar::base_type(filter)
{
    using ::boost::spirit::unicode::space;
    using ::boost::spirit::unicode::alpha;
    using ::boost::spirit::unicode::alnum;
    using ::boost::spirit::unicode::char_;
    using ::boost::spirit::_1;
    using ::boost::spirit::_2;
    using ::boost::spirit::_3;

    filter = lit('(') >> filtercomp >> lit(')');

    filtercomp = and_expr | or_expr | not_expr | item;

    filterlist = +(filter[op(_val, _1)]);

    and_expr = lit('&') >> filterlist[op(_val, _1, FC_And)];

    or_expr = lit('|') >> filterlist[op(_val, _1, FC_Or)];

    not_expr = lit('!') >> filter[op(_val, _1)];

    item = substring | simple | extensible;

    simple = (attr >> filtertype >> value)[op(_val, _1, _2, _3)];

    attr = (attr_item >> -(lit(';') >> attr))[op(_val, _1, _2)];

    attr_item = alpha >> *(alnum | '-');

    value = +(lit('\\') >> (hex_8 | hex_2) | (char_ - '*' - ')'));

    filtertype = lit('=')[_val = SIO_Equal] |
        lit("~=")[_val = SIO_Approx] |
        lit(">=")[_val = SIO_Greater] |
        lit("<=")[_val = SIO_Less];

    substring = (attr >> lit('=') >> values)[op(_val, _1, _2)];

    values = (-(value) >> lit('*') >> *(value >> lit('*')) >>
        -(value))[op(_val, _1, _2, _3)];

    matchingrule = alnum >> *(alnum | '-' | '.');

    extensible = ((attr >> -(lit(":dn")) >>
        -(lit(':') >> matchingrule) >> lit(":=") >> value)
        |
        (-(lit(":dn")) >> lit(':') >> matchingrule >>
            lit(":=") >> value))
        [_pass = false /* extensible items are not implemented! */];
}

}   // namespace sf
}   // namespace ldap
