#pragma once
#include "ldapsf_export.h"

#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace ldap {
    namespace sf {

        enum class FilterComp
        {
            Uninitialized,
            And,
            Or,
            Not
        };

        enum class ItemType
        {
            Unknown,
            Simple,
            Present,
            Substring,
            Extensible
        };

        enum class SimpleItemOp
        {
            Uninitialized,
            Equal,
            Approx,
            Greater,
            Less
        };

        struct Subtree;
        struct Item;

        using Tree = ::boost::recursive_wrapper<Subtree>;
        using ItemPtr = std::shared_ptr<Item>;
        using Node = ::boost::variant<Tree, ItemPtr>;
        using NodeList = std::vector<Node>;
        using Value = std::string;
        using Attr = std::string;
        using MatchingRule = std::string;
        using AttrOptionsList = std::list<std::string>;
        using ValueList = std::vector<std::string>;

        struct ValueListMore
        {
            ValueListMore() noexcept :
                has_front_any_{true},
                has_back_any_{true}
            {}

            ValueListMore(ValueListMore const &) = default;
            ValueListMore &operator=(ValueListMore const &) = default;
            ValueListMore(ValueListMore &&) = default;
            ValueListMore &operator=(ValueListMore &&) = default;

            bool      has_front_any_;
            bool      has_back_any_;
            ValueList data_;
        };


        struct ldapsf_EXPORT Item
        {
            explicit Item(ItemType type) noexcept :
                type_{type},
                simple_op_{SimpleItemOp::Uninitialized}
            {}

            Item() noexcept : Item(ItemType::Unknown)
            {}

            Item(Item const &) = default;
            Item &operator=(Item const &) = default;
            Item(Item &&) = default;
            Item &operator=(Item &&) = default;

            void print(int level = 0) const;

            ItemType             type_;
            SimpleItemOp         simple_op_;
            Attr                 attr_;
            AttrOptionsList      attr_ops_;
            Value                value_;
            ValueListMore        values_;
        };


        struct ldapsf_EXPORT Subtree
        {
            explicit Subtree(FilterComp comp) noexcept :
                comp_{comp}
            {}

            Subtree() noexcept : Subtree(FilterComp::Uninitialized)
            {}

            Subtree(Subtree const &) = default;
            Subtree &operator=(Subtree const &) = default;
            Subtree(Subtree &&) = default;
            Subtree &operator=(Subtree &&) = default;

            void print(int level = 0) const;

            NodeList             children_;
            FilterComp           comp_;
            static const int     print_indent_ = 4;
        };

    }   // namespace sf
}   // namespace ldap
