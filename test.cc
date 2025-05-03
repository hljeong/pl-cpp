#include "lib/cpp_utils/cpy/cpy.h"
#include "lib/cpp_utils/fmt/fmt.h"
#include "nfa.h"

using namespace cpy;
using namespace fmt;
using namespace nfa;

int main() {
  Nfa::Alphabet S{'a', 'b'};
  Nfa::State q0, q1;
  Nfa n1{{q0}, q0, S, {{q0, {{'a', {q0}}}}}, {q0}};
  Nfa n2{{q1}, q1, S, {{q1, {{'b', {q1}}}}}, {q1}};
  Nfa n3 = n1 + n2;
  Nfa n4 = n1 | n2;
  Nfa n5 = *(n1 + n2);

  List<String> strings{"", "a", "aa", "ab", "bb", "aabb", "ba", "baa"};
  List<Pair<Nfa, List<Pair<String, bool>>>> tests;
  tests.push_back({n1,
                   {{"", true},
                    {"a", true},
                    {"aa", true},
                    {"ab", false},
                    {"bb", false},
                    {"aabb", false},
                    {"ba", false},
                    {"baa", false}}});
  tests.push_back({n2,
                   {{"", true},
                    {"a", false},
                    {"aa", false},
                    {"ab", false},
                    {"bb", true},
                    {"aabb", false},
                    {"ba", false},
                    {"baa", false}}});
  tests.push_back({n3,
                   {{"", true},
                    {"a", true},
                    {"aa", true},
                    {"ab", true},
                    {"bb", true},
                    {"aabb", true},
                    {"ba", false},
                    {"baa", false}}});
  tests.push_back({n4,
                   {{"", true},
                    {"a", true},
                    {"aa", true},
                    {"ab", false},
                    {"bb", true},
                    {"aabb", false},
                    {"ba", false},
                    {"baa", false}}});
  tests.push_back({n5,
                   {{"", true},
                    {"a", true},
                    {"aa", true},
                    {"ab", true},
                    {"bb", true},
                    {"aabb", true},
                    {"ba", true},
                    {"baa", true}}});
  for (const auto &[idx, nfa_tests] : enumerate(tests)) {
    const auto &[nfa, tests_] = nfa_tests;
    for (const auto &[s, expected] : tests_) {
      bool result = nfa.accepts(s);
      if (result != expected) {
        print("n{}.accepts({}) => {} (expected: {})", idx + 1, repr(s), result,
              expected);
        printo(nfa);
        return 1;
      }
    }
  }
}
