#include "ldap_sf.h"

#include <iterator>
#include <stdexcept>

namespace ldap {
    using ::boost::spirit::qi::phrase_parse;
    using ::boost::spirit::unicode::space;

    sf::Node SearchFilter::buildQuery(std::string const & query) const
    {
        sf::Node ast;

        auto begin = std::cbegin(query);
        auto end = std::cend(query);
        if (!phrase_parse(begin, end, grammar_, space, ast) ||
            begin != end) {
            throw std::runtime_error{"Failed to build a query '" + query + "'"};
        }

        return ast;
    }

    RecordListPtr SearchFilter::operator()(sf::Node const & ast,
                                           RecordList const & records,
                                           sf::Eval::Collator::ECollationStrength strength,
                                           sf::Eval::Locale const & loc) const
    {
        return eval_(ast, records, strength, loc);
    }

    RecordListPtr SearchFilter::operator()(std::string const & query,
                                           RecordList const & records,
                                           sf::Eval::Collator::ECollationStrength strength,
                                           sf::Eval::Locale const & loc) const
    {
        auto ast = buildQuery(query);
        return eval_(ast, records, strength, loc);
    }

}   // namespace ldap

