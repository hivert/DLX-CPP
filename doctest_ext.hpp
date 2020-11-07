//****************************************************************************//
//       Copyright (C) 2020 Florent Hivert <Florent.Hivert@lri.fr>,           //
//                                                                            //
//    Distributed under the terms of the GNU General Public License (GPL)     //
//                                                                            //
//    This code is distributed in the hope that it will be useful,            //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
//    General Public License for more details.                                //
//                                                                            //
//    The full text of the GPL is available at:                               //
//                                                                            //
//                  http://www.gnu.org/licenses/                              //
//****************************************************************************//
#ifndef DOCTEST_EXT_HPP_
#define DOCTEST_EXT_HPP_
#include <doctest/doctest.h>
#include <cstdlib>  // exit
#include <ostream>
#include <sstream>  // ostringstream

namespace doctest {

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
  out << '[';
  auto it = v.begin(), end = v.end();
  if (it != end) {
    out << *it;
    for (++it; it != end; ++it) out << ", " << *it;
  }
  out << "]";
  return out;
}

template <typename T>
struct StringMaker<std::vector<T>> {
  static String convert(const std::vector<T> &v) {
    std::ostringstream os;
    os << v;
    return String(os.str().c_str());
  }
};

//////////////////////////////////////////////////////////////////////////
TEST_SUITE_BEGIN("[doctest_ext]doctest::operator<<(out, std::vector<T>)");
//////////////////////////////////////////////////////////////////////////
TEST_CASE("std::vector<bool>") {
  std::ostringstream os;
  os << std::vector<bool>({});
  CHECK(os.str() == "[]");
  os.str("");
  os << std::vector<bool>({1, 0, 1, 1});
  CHECK(os.str() == "[1, 0, 1, 1]");
}
TEST_CASE("std::vector<int>") {
  std::ostringstream os;
  os << std::vector<int>({});
  CHECK(os.str() == "[]");
  os.str("");
  os << std::vector<int>({3, 1, -3, 2});
  CHECK(os.str() == "[3, 1, -3, 2]");
}
TEST_CASE("std::vector<std::vector<int>>") {
  std::ostringstream os;
  os << std::vector<std::vector<int>>({});
  CHECK(os.str() == "[]");
  os.str("");
  os << std::vector<std::vector<int>>({{3, 1, -3, 2}, {}, {1, 2}});
  CHECK(os.str() == "[[3, 1, -3, 2], [], [1, 2]]");
}
//////////////////////////////////////////////////////////////////////////
TEST_SUITE_END();  // [dlx_matrix]doctest::operator<<(out, std::vector<T>)
//////////////////////////////////////////////////////////////////////////

inline void run_test(int argc, char **argv) {
  doctest::Context context;

  context.setOption("order-by", "name");  // sort the test cases by their name
  context.applyCommandLine(argc, argv);
  int res = context.run();
  if (context.shouldExit()) exit(res);
}

}  // namespace doctest

#endif  // DOCTEST_EXT_HPP_
