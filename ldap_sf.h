#pragma once
#include "ldap_sf_grammar.h"
#include "ldap_sf_eval.h"

#include <string>

namespace  ldap
{

class SearchFilter
{
  public:
    sf::Node       buildQuery( const std::string &  query ) const;

    RecordListPtr  operator()( const sf::Node &  ast,
                      const RecordList &  records,
                      sf::Eval::Collator::ECollationStrength  strength =
                                          sf::Eval::Collator::PRIMARY,
                      const sf::Eval::Locale &  loc =
                                          sf::Eval::getDefaultLocale() ) const;

    RecordListPtr  operator()( const std::string &  query,
                      const RecordList &  records,
                      sf::Eval::Collator::ECollationStrength  strength =
                                          sf::Eval::Collator::PRIMARY,
                      const sf::Eval::Locale &  loc =
                                          sf::Eval::getDefaultLocale() ) const;

  private:
    sf::Grammar< std::string::const_iterator >  grammar_;

    sf::Eval                                    eval_;
};

}   // namespace ldap
