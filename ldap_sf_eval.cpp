#include "ldap_sf_eval.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <boost/assert.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

#include <unicode/locid.h>
#include <unicode/stsearch.h>

using namespace icu;

namespace {
    std::string const col_errmsg{"Failed to create an instance of a collator"};
    std::string const stsearch_errmsg{"Failed to create an instance of a string search object"};
}

namespace ldap {
    namespace sf {

        struct NodeVisitor : ::boost::static_visitor<ldap::sf::Eval::RecordListPartition>
        {
            RecordList const& records;
            Collator::ECollationStrength strength;
            Locale const& loc;
            Eval const & _eval;

            NodeVisitor(RecordList const &rl, Collator::ECollationStrength s, Locale const &l, Eval const &e)
                : records{rl}
                , strength{s}
                , loc{l}
                , _eval{e}
            {}

            Eval::RecordListPartition operator()(ldap::sf::Subtree const& s) const
            {
                BOOST_ASSERT(!s.children_.empty());

                Eval::RecordListPartition res;
                Eval::RecordListPartition curp;

                switch (s.comp_) {
                case FilterComp::And:
                    curp = _eval.eval(s.children_[0], records, strength, loc);
                    res.second = std::move(curp.second);
                    for (auto k = std::next(std::cbegin(s.children_));
                         k != std::cend(s.children_); ++k) {
                        curp = _eval.eval(*k, *curp.first, strength, loc);
                        res.second->insert(res.second->end(), curp.second->begin(),
                            curp.second->end());
                    }
                    res.first = std::move(curp.first);
                    break;
                case FilterComp::Or:
                    curp = _eval.eval(s.children_[0], records, strength, loc);
                    res.first = std::move(curp.first);
                    for (auto k = std::next(std::cbegin(s.children_));
                        k != std::cend(s.children_); ++k) {
                        curp = _eval.eval(*k, *curp.second, strength, loc);
                        res.first->insert(res.first->end(), curp.first->begin(),
                            curp.first->end());
                    }
                    res.second = std::move(curp.second);
                    break;
                case FilterComp::Not:
                    {
                        using std::swap;
                        res = _eval.eval(s.children_[0], records, strength, loc);
                        swap(res.first, res.second);
                    }
                    break;
                default:
                    break;
                }
                return res;
            }

            ldap::sf::Eval::RecordListPartition operator()(ldap::sf::ItemPtr const& p) const
            {
                return _eval.evalItem(p, records, strength, loc);
            }
        };

        Eval::RecordListPartition Eval::eval(Node const & node,
                                             RecordList const & records,
                                             Collator::ECollationStrength strength,
                                             Locale const & loc) const
        {
            auto res = ::boost::apply_visitor(NodeVisitor{records, strength, loc, *this}, node);

            BOOST_ASSERT(res.first && res.second);

            return res;
        }

        Eval::RecordListPartition Eval::evalItem(ItemPtr const item,
                                                 RecordList const & records,
                                                 Collator::ECollationStrength strength,
                                                 Locale const & loc) const
        {
            RecordListPartition res{std::make_shared<RecordList>(),
                                    std::make_shared<RecordList>()};

            switch (item->type_) {
            case ItemType::Simple:
                std::partition_copy(std::begin(records), std::end(records),
                                    std::back_inserter(*res.first),
                                    std::back_inserter(*res.second),
                                    [&item, strength, &loc](RecordPtr const & record)
                                    {
                                        Record::const_iterator it = record->find(item->attr_);
                                        if (it == record->end()) {
                                            return false;
                                        }
                                        UErrorCode status{U_ZERO_ERROR};
                                        std::unique_ptr<Collator> col{Collator::createInstance(loc, status)};
                                        if (U_FAILURE(status)) {
                                            throw std::runtime_error(col_errmsg);
                                        }
                                        col->setStrength(strength);
                                        status = U_ZERO_ERROR;
                                        auto const res = col->compareUTF8(StringPiece(it->second),
                                                                          StringPiece(item->value_),
                                                                          status);
                                        if (U_FAILURE(status)) {
                                            return false;
                                        }
                                        switch (item->simple_op_) {
                                        case SimpleItemOp::Equal:
                                            /* approx is implemented as equal! */
                                        case SimpleItemOp::Approx:
                                            return res == UCOL_EQUAL;
                                        case SimpleItemOp::Greater:
                                            return res == UCOL_GREATER ||
                                                res == UCOL_EQUAL;
                                        case SimpleItemOp::Less:
                                            return res == UCOL_LESS ||
                                                res == UCOL_EQUAL;
                                        default:
                                            return false;
                                        }
                                    });
                break;
            case ItemType::Present:
                std::partition_copy(std::begin(records), std::end(records),
                                    std::back_inserter(*res.first),
                                    std::back_inserter(*res.second),
                                    [&item](RecordPtr const & record)
                                    {
                                        return record->find(item->attr_) != record->end();
                                    });
                break;
            case ItemType::Substring:
                std::partition_copy(std::begin(records), std::end(records),
                                    std::back_inserter(*res.first),
                                    std::back_inserter(*res.second),
                                    [&item, strength, &loc, this](const RecordPtr &  record)
                                    {
                                        Record::const_iterator it = record->find(item->attr_);
                                        return it == record->end() ? false :
                                            testSubstring(item->values_,
                                                          it->second,
                                                          strength, loc);
                                    });
                break;
            default:
                break;
            }

            return res;
        }

        bool Eval::testSubstring(ValueListMore const & values,
                                 std::string const & s,
                                 Collator::ECollationStrength strength,
                                 Locale const & loc) const
        {
            BOOST_ASSERT(!values.data_.empty());

            auto begin = std::cbegin(values.data_);
            auto end = std::cend(values.data_);
            auto su = UnicodeString::fromUTF8(StringPiece(s));
            auto pos = static_cast<int32_t>(0);
            auto len = su.length();
            UErrorCode status{U_ZERO_ERROR};

            if (!values.has_front_any_) {
                auto vu = UnicodeString::fromUTF8(StringPiece(*begin));
                StringSearch it(vu, su, loc, nullptr, status);

                if (U_FAILURE(status)) {
                    throw std::runtime_error{stsearch_errmsg};
                }

                it.getCollator()->setStrength(strength);

                auto const su_curpos = it.first(status);
                if (su_curpos != 0) {
                    return false;
                }

                pos = it.getMatchedLength();
                len -= pos;

                begin = std::next(begin);
            }

            if (!values.has_back_any_) {
                end = std::prev(end);

                auto vu = UnicodeString::fromUTF8(StringPiece(*end));
                StringSearch it(vu, UnicodeString(su, pos), loc, nullptr, status);

                if (U_FAILURE(status)) {
                    throw std::runtime_error{stsearch_errmsg};
                }

                it.getCollator()->setStrength(strength);

                auto const su_curpos = it.last(status);
                if (su_curpos == USEARCH_DONE) {
                    return false;
                }

                auto const su_curlen = it.getMatchedLength();
                if (su_curpos != (len - su_curlen)) {
                    return false;
                }

                len -= su_curlen;
            }

            for (auto k = begin; k != end; ++k) {
                auto vu = UnicodeString::fromUTF8(StringPiece(*k));
                StringSearch it(vu, UnicodeString{su, pos, len}, loc, nullptr, status);

                if (U_FAILURE(status)) {
                    throw std::runtime_error{stsearch_errmsg};
                }

                it.getCollator()->setStrength(strength);

                auto const su_curpos = it.first(status);
                if (su_curpos == USEARCH_DONE) {
                    return false;
                }

                auto const shift = su_curpos + it.getMatchedLength();
                pos += shift;
                len -= shift;
            }

            return true;
        }

        Eval::Locale const & Eval::getDefaultLocale()
        {
            return Locale::getDefault();
        }

    }   // namespace sf
}   // namespace ldap

