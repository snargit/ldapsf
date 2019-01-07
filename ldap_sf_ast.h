#pragma once
#include "ldapsf_export.h"

#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace ldap { namespace sf
{

enum FilterComp
{
    FC_Uninitialized,
    FC_And,
    FC_Or,
    FC_Not
};

enum ItemType
{
    IT_Unknown,
    IT_Simple,
    IT_Present,
    IT_Substring,
    IT_Extensible
};

enum SimpleItemOp
{
    SIO_Uninitialized,
    SIO_Equal,
    SIO_Approx,
    SIO_Greater,
    SIO_Less
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
        simple_op_{SIO_Uninitialized}
    {}

    Item() noexcept : Item(IT_Unknown)
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
    explicit Subtree(FilterComp comp) noexcept:
        comp_{comp}
    {}

    Subtree() noexcept : Subtree(FC_Uninitialized)
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
