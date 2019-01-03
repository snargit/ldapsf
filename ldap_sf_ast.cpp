#include "ldap_sf_ast.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/format.hpp>
#include <boost/variant/get.hpp>


namespace  ldap
{
namespace  sf
{

void  Subtree::print( int  level ) const
{
  static const std::string  op_id[] = { "UNINITIALIZED", "&", "|", "!" };
  std::stringstream         value;

  value << op_id[ comp_ ];

  std::stringstream         format;

  format << "%|" << level * print_indent_ << "t|";
  std::cout << boost::format( format.str() ) << value.str() << std::endl;

  for ( const Node &  k : children_ )
  {
    const Subtree *  subtree( boost::get< Subtree >( &k ) );

    if ( subtree )
    {
      subtree->print( level + 1 );
    }
    else
    {
      const ItemPtr *  item( boost::get< ItemPtr >( &k ) );
      if ( item )
        ( *item )->print( level + 1 );
    }
  }
}


void  Item::print( int  level ) const
{
  std::stringstream         value;

  switch ( type_ )
  {
  case IT_Simple:
    {
      static const std::string  op_id[] =
        { "UNINITIALIZED", "=", "~=", ">=", "<=" };
      value << op_id[ simple_op_ ];
    }
    break;
  case IT_Present:
  case IT_Substring:
    value << "*";
    break;
  default:
    value << "UNKNOWN";
    break;
  }

  value << "  " << attr_;

  if ( ! attr_ops_.empty() )
  {
    value << " (";
    for ( const Attr &  k : attr_ops_ )
      value << " " << k;
    value << " )";
  }

  switch ( type_ )
  {
  case IT_Simple:
    value << "  |  " << value_ ;
    break;
  case IT_Substring:
    value << "  |  ";
    if ( values_.has_front_any_ )
      value << "* ";
    for ( ValueList::const_iterator  k( values_.data_.begin() );
          k != values_.data_.end(); ++k )
    {
      if ( k != values_.data_.begin() )
        value << "* ";
      value << *k << " ";
    }
    if ( values_.has_back_any_ )
      value << "* ";
    break;
  default:
    break;
  }

  std::stringstream         format;

  format << "%|" << level * Subtree::print_indent_ << "t|";
  std::cout << boost::format( format.str() ) << value.str() << std::endl;
}

}   // namespace sf
}   // namespace ldap

