#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace  ldap
{
    using Record = std::map<std::string, std::string>;
    using RecordPtr = std::shared_ptr<Record>;
    using RecordList = std::vector<RecordPtr>;
    using RecordListPtr = std::shared_ptr<RecordList>;
}   // namespace ldap


inline std::ostream & operator<<(std::ostream & out,
                                 const ldap::RecordPtr & record)
{
  out << "\n";

  if (!record) {
      out << "<null>";
  }

  for (auto const & k : *record) {
      out << k.first << ": " << k.second << std::endl;
  }

  return out;
}
