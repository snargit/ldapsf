#include "ldap_sf_eval.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <boost/assert.hpp>
#include <boost/variant/get.hpp>
#include <unicode/locid.h>
#include <unicode/stsearch.h>

using namespace icu;

namespace {
    std::string const col_errmsg{"Failed to create an instance of a collator"};
    std::string const stsearch_errmsg{"Failed to create an instance of a string search object"};
}

namespace ldap {
    namespace sf {

        Eval::RecordListPartition Eval::eval(Node const & node,
                                             RecordList const & records,
                                             Collator::ECollationStrength strength,
                                             Locale const & loc) const
        {
            RecordListPartition res;
            Subtree const * subtree = boost::get<Subtree>(&node);

            if (subtree) {
                BOOST_ASSERT(!subtree->children_.empty());
                RecordListPartition curp;

                switch (subtree->comp_) {
                case FC_And:
                    curp = eval(subtree->children_[0], records, strength, loc);
                    res.second = std::move(curp.second);
                    for (auto k = std::next(std::cbegin(subtree->children_));
                         k != std::cend(subtree->children_); ++k) {
                        curp = eval(*k, *curp.first, strength, loc);
                        res.second->insert(res.second->end(), curp.second->begin(),
                                           curp.second->end());
                    }
                    res.first = std::move(curp.first);
                    break;
                case FC_Or:
                    curp = eval(subtree->children_[0], records, strength, loc);
                    res.first = std::move(curp.first);
                    for (auto k = std::next(std::cbegin(subtree->children_));
                         k != std::cend(subtree->children_); ++k) {
                        curp = eval(*k, *curp.second, strength, loc);
                        res.first->insert(res.first->end(), curp.first->begin(),
                                          curp.first->end());
                    }
                    res.second = std::move(curp.second);
                    break;
                case FC_Not:
                    {
                        using std::swap;
                        res = eval(subtree->children_[0], records, strength, loc);
                        swap(res.first, res.second);
                    }
                    break;
                default:
                    break;
                }
            } else {
                ItemPtr const * item = boost::get<ItemPtr>(&node);
                if (item)
                    res = evalItem(*item, records, strength, loc);
            }

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
            case IT_Simple:
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
                                        case SIO_Equal:
                                            /* approx is implemented as equal! */
                                        case SIO_Approx:
                                            return res == UCOL_EQUAL;
                                        case SIO_Greater:
                                            return res == UCOL_GREATER ||
                                                res == UCOL_EQUAL;
                                        case SIO_Less:
                                            return res == UCOL_LESS ||
                                                res == UCOL_EQUAL;
                                        default:
                                            return false;
                                        }
                                    });
                break;
            case IT_Present:
                std::partition_copy(std::begin(records), std::end(records),
                                    std::back_inserter(*res.first),
                                    std::back_inserter(*res.second),
                                    [&item](RecordPtr const & record)
                                    {
                                        return record->find(item->attr_) != record->end();
                                    });
                break;
            case IT_Substring:
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
                    throw std::runtime_error(stsearch_errmsg);
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
                    throw std::runtime_error(stsearch_errmsg);
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

