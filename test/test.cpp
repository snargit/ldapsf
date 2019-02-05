#include <string>
#include <ldapsf/ldap_records.h>
#include <ldapsf/ldap_sf.h>

#include <iostream>

using namespace  ldap;
using namespace  ldap::sf;


void  printQuery( const Node &  tree )
{
  const Subtree *    subtree( boost::get< Subtree >( &tree ) );

  std::cout << std::endl << ">>> Query" << std::endl;

  if ( subtree )
  {
    subtree->print();
  }
  else
  {
    const ItemPtr *  item( boost::get<ItemPtr>( &tree ) );
    if ( item )
      ( *item )->print();
  }
}


int  main( void )
{
  RecordList         records;

    Record  r{{"name", "Семён"},
                {"sname", "Иванов"},
                {"grade", "5"},
                {"extra", "M"}};
    Record  r2{{"name", "Виталий"},{"sname", "Полищук"}};
    Record  r3{{"name", "Пламен"},{"sname" , "Стоянов"},{"grade" , "5"}};
    Record  r4{{"name", "Семён"},{"sname" , "Степанов"}};
    Record  r5{{"name", "Семён"},{"sname" , "Пирогов"},{"grade" , "3"}};
    Record  r6{{"name", "Света"},{"sname"},{"Соколова"},{"grade" , "3"},{"extra" , "P"}};
    Record  r7{{"name", "Дементий"},{"sname" , "Порохов"},{"grade" , "2"}};
    Record  r8{{"name", "Лукьян"},{"sname" , "Чудинов"},{"grade" , "4"}};

    records.insert(r);
    records.insert(r2);
    records.insert(r3);
    records.insert(r4);
    records.insert(r5);
    records.insert(r6);
    records.insert(r7);
    records.insert(r8);

  std::cout << ">>> Data";

  for ( auto  k : records )
      std::cout << k;

  SearchFilter       filter;

  std::string        query( "(&(name=*мен)(!(sname=*ван*))(grade=*))" );
  Node               tree( filter.buildQuery( query ) );
  Node               tree0( tree );

  std::cout << std::endl;
  printQuery( tree );

  RecordListPtr      found( filter( tree, records ) );

  std::cout << std::endl << ">>> Found (primary collation)";

  for ( const RecordPtr &  k : *found )
    std::cout << k;

  query = "(grade>=4)";
  tree  = filter.buildQuery( query );

  std::cout << std::endl;
  printQuery( tree );

  found = filter( tree, *found );

  std::cout << std::endl << ">>> Found (among prev result)";

  for ( const RecordPtr &  k : *found )
    std::cout << k;

  std::cout << std::endl;
  printQuery( tree0 );

  found = filter( tree0, records, Collator::IDENTICAL );

  std::cout << std::endl << ">>> Found (identical collation)";

  for ( const RecordPtr &  k : *found )
    std::cout << k;

  query = "(|(grade>=4)(extra=*))";
  tree  = filter.buildQuery( query );

  std::cout << std::endl;
  printQuery( tree );

  found = filter( tree, records );

  std::cout << std::endl << ">>> Found (primary collation)";

  for ( const RecordPtr &  k : *found )
    std::cout << k;

  return 0;
}

