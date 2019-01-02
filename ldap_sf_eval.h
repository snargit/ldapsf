#pragma once

#include "ldap_sf_ast.h"
#include "ldap_records.h"

#include <unicode/coll.h>

#include <memory>
#include <string>
#include <utility>

namespace ldap { namespace sf
{

class Eval
{
  public:
    using Collator = icu::Collator;
    using Locale = icu::Locale;

  private:
    using RecordListPartition = std::pair<RecordListPtr, RecordListPtr>;

  public:
    RecordListPtr  operator()( const Node &  node, const RecordList &  records,
                               Collator::ECollationStrength strength =
                                                              Collator::PRIMARY,
                               const Locale & loc = getDefaultLocale() ) const
    {
      return eval( node, records, strength, loc ).first;
    }

  private:
    RecordListPartition  eval( const Node &  node,
                               const RecordList &  records,
                               Collator::ECollationStrength  strength,
                               const Locale &  loc ) const;

    RecordListPartition  evalItem( const ItemPtr  item,
                                   const RecordList &  records,
                                   Collator::ECollationStrength  strength,
                                   const Locale &  loc ) const;

    bool  testSubstring( const ValueListMore &  values,
                         const std::string &  s,
                         Collator::ECollationStrength  strength,
                         const Locale &  loc ) const;

  public:
    static const Locale & getDefaultLocale();
};

}   // namespace sf
}   // namespace ldap
