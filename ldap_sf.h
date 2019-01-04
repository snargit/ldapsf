#pragma once

#include "ldap_sf_grammar.h"
#include "ldap_sf_eval.h"

#include <string>

namespace  ldap {

class SearchFilter
{
public:
    sf::Node buildQuery(std::string const & query) const;

    RecordListPtr operator()(sf::Node const & ast,
                             RecordList const & records,
                             sf::Eval::Collator::ECollationStrength strength =
                                 sf::Eval::Collator::PRIMARY,
                             sf::Eval::Locale const & loc =
                                 sf::Eval::getDefaultLocale()) const;

    RecordListPtr operator()(std::string const & query,
                             RecordList const & records,
                             sf::Eval::Collator::ECollationStrength strength =
                                 sf::Eval::Collator::PRIMARY,
                             sf::Eval::Locale const & loc =
                                 sf::Eval::getDefaultLocale()) const;

private:
    sf::Grammar<std::string::const_iterator> grammar_;
    sf::Eval eval_;
};

}   // namespace ldap
