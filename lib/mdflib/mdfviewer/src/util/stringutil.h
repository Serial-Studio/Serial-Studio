#pragma once
#include <string>
#include <algorithm>

namespace util::string {

/// This class is used when get a sorted list of string to ignoring case
/// characters.\n Example of declaring a map: \code std::map<std::string,
/// FooClass, util::string::IgnoreCase> foo_list; \endcode
class IgnoreCase final {
 public:
  bool operator()(const std::string &s1, const std::string &s2)
      const;  ///< Compare the strings by ignoring case.
  bool operator()(const std::wstring &s1, const std::wstring &s2)
      const;  ///< Compare the strings by ignoring case.
};
}  // namespace util::string