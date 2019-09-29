#include "ldap_sf_ast.h"

#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/format.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

namespace {
    struct NodeVisitor : ::boost::static_visitor<void>
    {
        int _level;

        NodeVisitor(int level) : _level{ level } {}

        void operator()(ldap::sf::Subtree const& s) const
        {
            s.print(_level + 1);
        }

        void operator()(ldap::sf::ItemPtr const& p) const
        {
            p->print(_level + 1);
        }
    };
}


namespace  ldap {
    namespace  sf {

        void Subtree::print(int level) const
        {
            static std::array<std::string, 4> const op_id = {"UNINITIALIZED", "&", "|", "!"};

            std::stringstream value;

            value << op_id[static_cast<size_t>(comp_)];

            std::stringstream format;

            format << "%|" << level * print_indent_ << "t|";
            std::cout << boost::format(format.str()) << value.str() << std::endl;

            NodeVisitor v{ level };

            for (auto const & k : children_) {
                ::boost::apply_visitor(v, k);
            }
        }

        void Item::print(int level) const
        {
            std::stringstream value;

            switch (type_) {
            case ItemType::Simple:
                {
                    static std::array<std::string, 5> const op_id = {"UNINITIALIZED", "=", "~=", ">=", "<="};
                    value << op_id[static_cast<size_t>(simple_op_)];
                }
                break;
            case ItemType::Present:
            case ItemType::Substring:
                value << "*";
                break;
            default:
                value << "UNKNOWN";
                break;
            }

            value << "  " << attr_;

            if (!attr_ops_.empty()) {
                value << " (";
                for (auto const & k : attr_ops_) {
                    value << " " << k;
                }
                value << " )";
            }

            switch (type_) {
            case ItemType::Simple:
                value << "  |  " << value_;
                break;
            case ItemType::Substring:
                value << "  |  ";
                if (values_.has_front_any_)
                    value << "* ";
                for (auto k = std::cbegin(values_.data_);
                     k != std::cend(values_.data_); ++k) {
                    if (k != std::cbegin(values_.data_)) {
                        value << "* ";
                    }
                    value << *k << " ";
                }
                if (values_.has_back_any_)
                    value << "* ";
                break;
            default:
                break;
            }

            std::stringstream format;

            format << "%|" << level * Subtree::print_indent_ << "t|";
            std::cout << boost::format(format.str()) << value.str() << std::endl;
        }

    }   // namespace sf
}   // namespace ldap

