#pragma once

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
  ValueListMore() : has_front_any_( true ), has_back_any_( true )
  {}

  bool                 has_front_any_;

  bool                 has_back_any_;

  ValueList            data_;
};


struct  Item
{
  Item( ItemType  type = IT_Unknown ) : type_( type ),
    simple_op_( SIO_Uninitialized )
  {}

  void  print( int  level = 0 ) const;

  ItemType             type_;

  SimpleItemOp         simple_op_;

  Attr                 attr_;

  AttrOptionsList      attr_ops_;

  Value                value_;

  ValueListMore        values_;
};


struct  Subtree
{
  Subtree( FilterComp  comp = FC_Uninitialized ) : comp_( comp )
  {}

  void  print( int  level = 0 ) const;

  NodeList             children_;

  FilterComp           comp_;

  static const int     print_indent_ = 4;
};

}   // namespace sf
}   // namespace ldap
