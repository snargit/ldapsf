#include "ldap_sf_grammar.h"

#include <boost/variant/get.hpp>

#include <algorithm>
#include <iterator>
#include <utility>

namespace  ldap { namespace  sf
{

void Compiler::operator()(NodeList & nodes, Node & node) const
{
  nodes.emplace_back(std::move(node));
}

void Compiler::operator()(Node & node, NodeList & nodes,
                          FilterComp comp) const
{
  node = Subtree{comp};
  auto & tree = boost::get<Subtree>(node);
  tree.children_ = std::move(nodes);
}

void Compiler::operator()(Node & node, Node & nodeNest) const
{
  node = Subtree{FC_Not};
  auto & tree = boost::get<Subtree>(node);
  tree.children_.emplace_back(std::move(nodeNest));
}

void Compiler::operator()(ItemPtr & item, AttrOptionsList & attrs,
                          SimpleItemOp op, Value & value) const
{
  item = std::make_shared<Item>(IT_Simple);
  item->attr_ = std::move(attrs.front());
  std::move(std::next(std::begin(attrs)), attrs.end(),
            std::back_inserter( item->attr_ops_));
  item->simple_op_ = op;
  item->value_ = std::move(value);
}

void Compiler::operator()(AttrOptionsList & attrs, Attr & value,
                          ::boost::optional<AttrOptionsList> & attrs_options) const
{
  attrs.emplace_back(std::move(value));
  if (attrs_options) {
    auto & attrs_more_list = *attrs_options;
    std::move(std::begin(attrs_more_list), std::end(attrs_more_list),
              std::back_inserter(attrs));
  }
}

void Compiler::operator()(ItemPtr & item, AttrOptionsList & attrs,
                          ValueListMore & values) const
{
  item = std::make_shared<Item>(values.data_.empty() ? IT_Present : IT_Substring);
  item->attr_ = std::move(attrs.front());
  std::move(std::next(std::begin(attrs)), std::end(attrs),
            std::back_inserter(item->attr_ops_));
  item->values_ = std::move(values);
}


void Compiler::operator()(ValueListMore & values,
                          boost::optional<Value> & value_first,
                          ValueList & values_middle,
                          boost::optional<Value> & value_last) const
{
  if (value_first) {
    values.data_.emplace_back(std::move(*value_first));
    values.has_front_any_ = false;
  }
  std::move(std::begin(values_middle), std::end(values_middle),
            std::back_inserter(values.data_));
  if (value_last) {
    values.data_.emplace_back(std::move(*value_last));
    values.has_back_any_ = false;
  }
}

}   // namespace sf
}   // namespace ldap

