#include "stringutil.h"

namespace util::string {
bool IgnoreCase::operator()(const std::string &s1, const std::string &s2) const {
  std::string s1_lower(s1);
  std::string s2_lower(s2);
  std::transform(s1.begin(), s1.end(), s1_lower.begin(), ::tolower);
  std::transform(s2.begin(), s2.end(), s2_lower.begin(), ::tolower);
  return std::lexicographical_compare(s1_lower.begin(), s1_lower.end(), s2_lower.begin(), s2_lower.end());
}

bool IgnoreCase::operator()(const std::wstring &s1, const std::wstring &s2) const {
  std::wstring s1_lower(s1);
  std::wstring s2_lower(s2);
  std::transform(s1.begin(), s1.end(), s1_lower.begin(), ::tolower);
  std::transform(s2.begin(), s2.end(), s2_lower.begin(), ::tolower);
  return std::lexicographical_compare(s1_lower.begin(), s1_lower.end(), s2_lower.begin(), s2_lower.end());
}

}  // namespace util::string