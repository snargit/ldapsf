#pragma once

#include <memory>

namespace ldap
{
    class SearchFilter;
    using SearchFilterPtr = std::shared_ptr<SearchFilter>;
}   // namespace ldap
