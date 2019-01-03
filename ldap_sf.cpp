#include "ldap_sf.h"

#include <iterator>
#include <stdexcept>

namespace ldap
{

sf::Node SearchFilter::buildQuery(const std::string & query) const
{
    auto begin = std::cbegin(query);
    auto end = std::cend(query);
    sf::Node ast;

    if (!sf::phrase_parse(begin, end, grammar_, sf::space, ast) ||
        begin != end) {
        throw std::runtime_error("Failed to build a query '" + query + "'");
    }

    return ast;
}


RecordListPtr  SearchFilter::operator()(const sf::Node & ast,
                                        const RecordList & records,
                                        sf::Eval::Collator::ECollationStrength strength,
                                        const sf::Eval::Locale & loc) const
{
  return eval_( ast, records, strength, loc );
}

RecordListPtr  SearchFilter::operator()(const std::string & query,
                                        const RecordList & records,
                                        sf::Eval::Collator::ECollationStrength strength,
                                        const sf::Eval::Locale & loc) const
{
  auto ast = buildQuery(query);
  return eval_(ast, records, strength, loc);
}

}   // namespace ldap

