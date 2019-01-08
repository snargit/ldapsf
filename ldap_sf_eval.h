#pragma once

#include "ldap_sf_ast.h"
#include "ldapsf_export.h"
#include "ldap_records.h"

#include <unicode/coll.h>

#include <memory>
#include <string>
#include <utility>

namespace ldap {
    namespace sf {

        class ldapsf_EXPORT Eval
        {
        public:
            using Collator = icu::Collator;
            using Locale = icu::Locale;

        private:
            using RecordListPartition = std::pair<RecordListPtr, RecordListPtr>;

        public:
            RecordListPtr operator()(Node const & node, RecordList const & records,
                                     Collator::ECollationStrength strength = Collator::PRIMARY,
                                     Locale const & loc = getDefaultLocale()) const
            {
                return eval(node, records, strength, loc).first;
            }

        private:
            RecordListPartition eval(Node const & node,
                                     RecordList const & records,
                                     Collator::ECollationStrength strength,
                                     Locale const & loc) const;

            RecordListPartition evalItem(ItemPtr const item,
                                         RecordList const & records,
                                         Collator::ECollationStrength strength,
                                         Locale const & loc) const;

            bool testSubstring(ValueListMore const & values,
                               std::string const & s,
                               Collator::ECollationStrength strength,
                               Locale const & loc) const;

        public:
            static Locale const & getDefaultLocale();
        };

    }   // namespace sf
}   // namespace ldap
